#pragma once

#include "./types.hpp"
#include "./allocator.hpp"
#include "./expected.hpp"

namespace ntf {

namespace impl {

template<typename K, typename HashT, typename EqualsT>
struct fixed_hashmap_ops : 
  private HashT, private EqualsT
{
  template<typename HashU, typename EqualsU>
  fixed_hashmap_ops(HashU&& hash, EqualsU&& equals) :
    HashT{std::forward<HashU>(hash)},
    EqualsT{std::forward<EqualsU>(equals)} {}

  bool _equals(const K& a, const K& b) const {
    return EqualsT::operator()(a, b);
  }
  EqualsT& _key_eq() const { return static_cast<EqualsT&>(*this); }

  size_t _hash(const K& x) const {
    return HashT::operator()(x);
  }
  HashT& _hash_function() const { return static_cast<HashT&>(*this); }
};

template<typename K, typename T, typename Del>
struct fixed_hashmap_del : private Del
{
  fixed_hashmap_del(const Del& del) :
    Del{del} {}

  void _dealloc_values(std::pair<const K, T>* vals, size_t count) {
    constexpr size_t pair_sz = sizeof(std::pair<const K, T>);
    Del::operator()(reinterpret_cast<uint8*>(vals), count*pair_sz);
  }

  void _dealloc_flags(uint32* vals, size_t count) {
    Del::operator()(reinterpret_cast<uint8*>(vals), count*sizeof(uint32));
  }

  Del& _get_deleter() { return static_cast<Del&>(*this); }
};

} // namespace impl

template<
  typename K, typename T,
  typename HashT = std::hash<K>,
  typename EqualsT = std::equal_to<K>,
  typename DelT = default_alloc_del<uint8>
>
class fixed_hashmap :
  private impl::fixed_hashmap_ops<K, HashT, EqualsT>,
  private impl::fixed_hashmap_del<K, T, DelT>
{
private:
  static constexpr size_t FLAGS_PER_ENTRY = 16; // 2 bits each, uses 32 bits
  static constexpr uint32 FLAG_EMPTY = 0b00;
  static constexpr uint32 FLAG_USED = 0b01;
  static constexpr uint32 FLAG_TOMB = 0b10;
  static constexpr uint32 FLAG_MASK = 0b11;

  template<typename MapT, typename ValT>
  class forward_it {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = ValT;
    using pointer = value_type*;
    using reference = value_type&;
    using iterator_category = std::forward_iterator_tag;

  public:
    bool operator==(const forward_it& other) const {
      return other._idx == _idx && other._map == _map;
    }

    bool operator!=(const forward_it& other) const {
      return !(other == *this);
    }

    forward_it& operator++() {
      ++_idx;
      _next_valid();
      return *this;
    }

    reference operator*() const { return _map->_values[_idx]; }
    pointer operator->() const { return _map->_values+_idx; }

  private:
    explicit forward_it(MapT* map, size_t idx) :
      _map{map}, _idx{idx} { _next_valid(); }

    void _next_valid() {
      while (_idx < _map->capacity() && _map->_flag_at(_idx) != FLAG_USED) {
        ++_idx;
      }
    }

  private:
    MapT* _map;
    size_t _idx;

  private:
    friend fixed_hashmap;
  };

  using ops_base = impl::fixed_hashmap_ops<K, HashT, EqualsT>;
  using del_base = impl::fixed_hashmap_del<K, T, DelT>;

public:
  using key_type = K;
  using mapped_type = T;

  using value_type = std::pair<const key_type, mapped_type>;

  using hasher = HashT;
  using key_equal = EqualsT;
  using deleter_type = DelT;

  using size_type = std::size_t;
  using reference = value_type&;
  using const_reference = const value_type&;

  using iterator = forward_it<fixed_hashmap, value_type>;
  using const_iterator = forward_it<const fixed_hashmap, const value_type>;

private:
  fixed_hashmap(value_type* values, uint32* flags, size_t cap,
                const HashT& hash, const EqualsT& eqs, const DelT& del) :
    ops_base{hash, eqs}, del_base{del},
    _values{values}, _flags{flags}, _used{0}, _cap{cap} {}

public:
  template<typename HashU=HashT, typename EqualsU=EqualsT, typename Alloc=std::allocator<uint8>>
  requires(allocator_type<std::remove_cvref_t<Alloc>, uint8>)
  static auto from_size(
    size_t size, Alloc&& alloc = {}, const HashU& hash = {}, const EqualsU& eqs = {}
  ) noexcept -> expected<
    fixed_hashmap<K, T, HashU, EqualsU, allocator_delete<uint8, std::remove_cvref_t<Alloc>>>,
    error<void>
  > {
    using alloc_t = std::remove_cvref_t<Alloc>;
    using del_t = allocator_delete<uint8, alloc_t>;

    size_t flag_sz = _flag_count(size);
    uint8* flags = nullptr;
    uint8* array = nullptr;

    try {
      flags = alloc.allocate(flag_sz*sizeof(uint32));
      if (!flags) {
        return unexpected{error<void>{"Failed to allocate flags"}};
      }
      array = alloc.allocate(size*sizeof(value_type));
      if (!array) {
        alloc.deallocate(flags, flag_sz*sizeof(uint32));
        return unexpected{error<void>{"Failed to allocate array"}};
      }
      std::memset(flags, 0, flag_sz*sizeof(uint32));
    } catch (const std::exception& ex) {
      alloc.deallocate(array, size*sizeof(value_type));
      alloc.deallocate(flags, flag_sz*sizeof(uint32));
      return unexpected{error<void>{ex.what()}};
    } catch (...) {
      alloc.deallocate(array, size*sizeof(value_type));
      alloc.deallocate(flags, flag_sz*sizeof(uint32));
      return unexpected{error<void>{"Unknown error"}};
    }

    return fixed_hashmap<K,T,HashU,EqualsU,del_t>{
      reinterpret_cast<value_type*>(array), reinterpret_cast<uint32*>(flags), size,
      hash, eqs,
      del_t{std::forward<Alloc>(alloc)}
    };
  }

public:
  template<typename Key, typename... Args>
  std::pair<iterator, bool> try_emplace(Key&& key, Args&&... args) {
    size_t idx = ops_base::_hash(key) % capacity();
    for (size_t i = 0; i < capacity(); ++i, idx = (idx+1)%capacity()) {
      if (_flag_at(idx) != FLAG_USED) {
        std::construct_at(_values+idx,
                          std::make_pair(std::forward<Key>(key), std::forward<Args>(args)...));
        ++_used;
        _flag_set(idx, FLAG_USED);
        return std::make_pair(iterator{this, idx}, true);
      }
    }
    return std::make_pair(end(), false);
  }

  template<typename Key, typename U = T>
  std::pair<iterator, bool> try_overwrite(Key&& key, U&& obj) {
    size_t idx = ops_base::_hash(key) % capacity();
    for (size_t i = 0; i < capacity(); ++i, idx = (idx+1)%capacity()) {
      const auto flag = _flag_at(idx);
      if (flag == FLAG_EMPTY || flag == FLAG_TOMB) {
        std::construct_at(_values+idx,
                          std::make_pair(std::forward<Key>(key), std::forward<U>(obj)));
        ++_used;
        _flag_set(idx, FLAG_USED);
        return std::make_pair(iterator{this, idx}, true);
      }
      if (flag == FLAG_USED) {
        const key_type k(std::forward<Key>(key));
        if (ops_base::_equals(_values[idx].first, k)) {
          _values[idx].second = std::forward<U>(obj);
          return std::make_pair(iterator{this, idx}, true);
        }
      }
    }
    return std::make_pair(end(), false);
  }

  bool erase(const key_type& key) {
    size_t idx = ops_base::_hash(key) % capacity();
    for (size_t i = 0; i < capacity(); ++i, idx = (idx+1)%capacity()) {
      const auto flag = _flag_at(idx);
      if (flag == FLAG_EMPTY) {
        return false;
      }
      if (flag == FLAG_TOMB) {
        continue;
      }
      if (ops_base::_equals(_values[idx].first, key)) {
        _flag_set(idx, FLAG_TOMB);
        --_used;
        if constexpr (!std::is_trivial_v<value_type>) {
          _values[idx].~value_type();
        }
        return true;
      }
    }
    return false;
  }

  [[nodiscard]] iterator find(const key_type& key) {
    size_t idx = ops_base::_hash(key) % capacity();
    for (size_t i = 0; i < capacity(); ++i, idx = (idx+1)%capacity()) {
      const auto flag = _flag_at(idx);
      if (flag == FLAG_EMPTY) {
        return end();
      }
      if (flag == FLAG_TOMB) {
        continue;
      }
      if (ops_base::_equals(_values[idx].first, key)) {
        return iterator{this, idx};
      }
    }
    return end();
  }

  [[nodiscard]] const_iterator find(const key_type& key) const {
    size_t idx = ops_base::_hash(key) % capacity();
    for (size_t i = 0; i < capacity(); ++i, idx = (idx+1)%capacity()) {
      const auto flag = _flag_at(idx);
      if (flag == FLAG_EMPTY) {
        return end();
      }
      if (flag == FLAG_TOMB) {
        continue;
      }
      if (ops_base::_equals(_values[idx].first, key)) {
        return const_iterator{this, idx};
      }
    }
    return end();
  }

  void clear() noexcept {
    if constexpr (!std::is_trivial_v<value_type>) {
      for (size_t i = 0; i < capacity(); ++i) {
        if (_flag_at(i) == FLAG_USED) {
          _values[i].~value_type();
        }
      }
    }
    _used = 0;
  }

public:
  template<typename F>
  fixed_hashmap& for_each(F&& fun) {
    if (_used == 0) {
      return *this;
    }
    for (size_t i = 0; i < capacity(); ++i) {
      if (_flag_at(i) == FLAG_USED) {
        fun(_values[i]);
      }
    }
    return *this;
  }

  template<typename F>
  const fixed_hashmap& for_each(F&& fun) const {
    if (_used == 0) {
      return *this;
    }
    for (size_t i = 0; i < capacity(); ++i) {
      if (_flag_at(i) == FLAG_USED) {
        fun(_values[i]);
      }
    }
    return *this;
  }

public:
  mapped_type& operator[](const key_type& key) {
    auto it = find(key);
    NTF_ASSERT(it != end());
    return it->second;
  }
  mapped_type& at(const key_type& key) {
    auto it = find(key);
    if (it == end()) {
      throw std::out_of_range{fmt::format("Key '{}' not found", key)};
    }
    return it->second;
  }

  const mapped_type& operator[](const key_type& key) const {
    auto it = find(key);
    NTF_ASSERT(it != end());
    return it->second;
  }
  const mapped_type& at(const key_type& key) const {
    auto it = find(key);
    if (it == end()) {
      throw std::out_of_range{fmt::format("Key '{}' not found", key)};
    }
    return it->second;
  }

  iterator begin() { return iterator{this, 0u}; }
  const_iterator begin() const { return const_iterator{this, 0u}; }
  const_iterator cbegin() const { return const_iterator{this, 0u}; }

  iterator end() { return iterator{this, capacity()}; }
  const_iterator end() const { return const_iterator{this, capacity()}; }
  const_iterator cend() const { return const_iterator{this, capacity()}; }

  size_type size() const { return _used; }
  size_type capacity() const { return _cap; }
  float32 load_factor() const {
    return static_cast<float32>(size())/static_cast<float32>(capacity());
  }

  hasher& hash_function() { return ops_base::_hash_function(); }
  const hasher& hash_function() const { return ops_base::_hash_function(); }

  key_equal& key_eq() { return ops_base::_key_eq(); }
  const key_equal& key_eq() const { return ops_base::_key_eq(); }

  deleter_type& deleter() { return del_base::_get_deleter(); }
  const deleter_type& deleter() const { return del_base::_get_deleter(); }

private:
  static size_t _flag_count(size_t cap) {
    return std::ceil(static_cast<float32>(cap)/static_cast<float32>(FLAGS_PER_ENTRY));
  }

  void _destroy() {
    if (!_values) {
      return;
    }

    clear();
    del_base::_dealloc_values(_values, capacity());
    del_base::_dealloc_flags(_flags, _flag_count(capacity()));
  }

  uint32 _flag_at(size_t idx) const {
    const uint32 flag_idx = idx/FLAGS_PER_ENTRY;
    const uint32 shift = (idx%FLAGS_PER_ENTRY)*2;
    return (_flags[flag_idx] >> shift) & FLAG_MASK;
  }

  void _flag_set(size_t idx, uint32 flag) {
    const uint32 flag_idx = idx/FLAGS_PER_ENTRY;
    const uint32 shift = (idx%FLAGS_PER_ENTRY)*2;
    _flags[flag_idx] &= ~(FLAG_MASK << shift);
    _flags[flag_idx] |= (flag & FLAG_MASK) << shift;
  }

private:
  uint32* _flags;
  value_type* _values;
  size_t _used;
  const size_t _cap;

public:
  ~fixed_hashmap() noexcept { _destroy(); }

  fixed_hashmap(fixed_hashmap&& other) noexcept :
    ops_base{static_cast<ops_base&&>(other)},
    del_base{static_cast<del_base&&>(other)},
    _flags{std::move(other._flags)}, _values{std::move(other._values)},
    _used{std::move(other._used)}, _cap{std::move(other._cap)}
  {
    other._values = nullptr;
  }

  fixed_hashmap& operator=(fixed_hashmap&& other) noexcept {
    _destroy();

    ops_base::operator=(static_cast<ops_base&&>(other));
    del_base::operator=(static_cast<del_base&&>(other));

    _flags = std::move(other._flags);
    _values = std::move(other._values);
    _used = std::move(other._used);
    _cap = std::move(other._cap);

    other._values = nullptr;

    return *this;
  }

  fixed_hashmap(const fixed_hashmap&) = delete;
  fixed_hashmap& operator=(const fixed_hashmap&) = delete;
};

template<typename K, typename T, typename HashT=std::hash<K>, typename EqualsT=std::equal_to<K>>
using virtual_fixed_hashmap =
  fixed_hashmap<K, T, HashT, EqualsT, virtual_alloc_del<uint8>>;

} // namespace ntf
