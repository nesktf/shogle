#include "./types.hpp"
#include "./allocator.hpp"

namespace ntf {

namespace impl {

template<typename K, typename HashT, typename EqualsT>
struct fixed_hashmap_ops : 
  protected HashT, protected EqualsT
{
  template<typename HashU, typename EqualsU>
  fixed_hashmap_ops(HashU&& hash, EqualsU&& equals) :
    HashT{std::forward<HashU>(hash)},
    EqualsT{std::forward<EqualsU>(equals)} {}

  bool _equals(const K& a, const K& b) const {
    return EqualsT::operator()(a, b);
  }
  size_t _hash(const K& x) const {
    return HashT::operator()(x);
  }
};

} // namespace impl

template<
  typename K, typename T,
  typename HashT = std::hash<K>,
  typename EqualsT = std::equal_to<K>
>
class fixed_hashmap :
  private impl::fixed_hashmap_ops<K, HashT, EqualsT>
{
private:
  static constexpr size_t FLAGS_PER_ENTRY = 16; // 2 bits each, uses 32 bits
  static constexpr uint32_t FLAG_EMPTY = 0b00;
  static constexpr uint32_t FLAG_USED = 0b01;
  static constexpr uint32_t FLAG_TOMB = 0b10;
  static constexpr uint32_t FLAG_MASK = 0b11;

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

public:
  using key_type = K;
  using mapped_type = T;

  using value_type = std::pair<const key_type, mapped_type>;

  using hasher = HashT;
  using key_equal = EqualsT;

  using size_type = std::size_t;
  using reference = value_type&;
  using const_reference = const value_type&;

  using iterator = forward_it<fixed_hashmap, value_type>;
  using const_iterator = forward_it<const fixed_hashmap, const value_type>;

private:
  fixed_hashmap(value_type* values, uint32_t* flags, size_t cap,
            const HashT& hash, const EqualsT& eqs) noexcept :
    ops_base{hash, eqs},
    _values{values}, _flags{flags},_used{0}, _cap{cap} {}

public:
  template<
    typename Alloc = std::allocator<value_type>,
    typename HashU = HashT,
    typename EqualsU = EqualsT
  >
  fixed_hashmap(size_t cap,
            Alloc&& alloc = {}, HashU&& hash = {}, EqualsU&& eqs = {}) :
    ops_base{std::forward<HashU>(hash), std::forward<EqualsU>(eqs)},
    _used{0}, _cap{cap}
  {
    auto pair_alloc = rebind_alloc_t<Alloc, value_type>{std::forward<Alloc>(alloc)};
    auto flag_alloc = rebind_alloc_t<Alloc, uint32_t>{std::forward<Alloc>(alloc)};

    auto* vals = pair_alloc.allocate(cap);
    auto* flags = flag_alloc.allocate(cap/FLAGS_PER_ENTRY);
    std::memset(flags, 0, sizeof(uint32_t)*cap/FLAGS_PER_ENTRY);

    _values = vals;
    _flags = flags;
  }

public:
  template<
    typename Alloc = std::allocator<value_type>,
    typename HashU = HashT,
    typename EqualsU = EqualsT
  >
  static auto create(
    size_t cap, Alloc&& alloc = {}, HashU&& hash = {}, EqualsU&& equals = {}
  ) -> fixed_hashmap<K, T, HashT, EqualsT> {
    return fixed_hashmap<K, T, HashT, EqualsT>{
      cap, std::forward<Alloc>(alloc), std::forward<HashU>(hash), std::forward<EqualsU>(equals)
    };
  }

public:
  template<typename Key, typename... Args>
  requires(
    std::convertible_to<std::remove_cvref_t<Key>, key_type> &&
    std::constructible_from<mapped_type, Args...>
  )
  std::pair<iterator, bool> try_emplace(Key&& key, Args&&... args) {
    size_t idx = this->_hash(key) % capacity();
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
  requires(std::convertible_to<std::remove_cvref_t<Key>, key_type>)
  std::pair<iterator, bool> try_overwrite(Key&& key, U&& obj) {
    size_t idx = this->_hash(key) % capacity();
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
        if (this->_equals(_values[idx].first, k)) {
          _values[idx].second = std::forward<U>(obj);
          return std::make_pair(iterator{this, idx}, true);
        }
      }
    }
    return std::make_pair(end(), false);
  }

  bool erase(const key_type& key) {
    size_t idx = _hash(key) % capacity();
    for (size_t i = 0; i < capacity(); ++i, idx = (idx+1)%capacity()) {
      const auto flag = _flag_at(idx);
      if (flag == FLAG_EMPTY) {
        return false;
      }
      if (flag == FLAG_TOMB) {
        continue;
      }
      if (_equals(_values[idx].first, key)) {
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
    size_t idx = this->_hash(key) % capacity();
    for (size_t i = 0; i < capacity(); ++i, idx = (idx+1)%capacity()) {
      const auto flag = _flag_at(idx);
      if (flag == FLAG_EMPTY) {
        return end();
      }
      if (flag == FLAG_TOMB) {
        continue;
      }
      if (this->_equals(_values[idx].first, key)) {
        return iterator{this, idx};
      }
    }
    return end();
  }

  [[nodiscard]] const_iterator find(const key_type& key) const {
    size_t idx = _hash(key) % capacity();
    for (size_t i = 0; i < capacity(); ++i, idx = (idx+1)%capacity()) {
      const auto flag = _flag_at(idx);
      if (flag == FLAG_EMPTY) {
        return end();
      }
      if (flag == FLAG_TOMB) {
        continue;
      }
      if (_equals(_values[idx].first, key)) {
        return const_iterator{this, idx};
      }
    }
    return end();
  }

  void clear() noexcept {
    if (!_values) {
      return;
    }

    std::allocator<value_type> alloc;
    if constexpr (!std::is_trivial_v<value_type>) {
      for (size_t i = 0; i < capacity(); ++i) {
        if (_flag_at(i) == FLAG_USED) {
          _values[i].~value_type();
        }
      }
    }
    alloc.deallocate(_values, capacity());
    std::allocator<uint32_t> alloc2;
    alloc2.deallocate(_flags, capacity()/FLAGS_PER_ENTRY);
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

  void print() const {
    fmt::print("BEGIN\n");
    for (size_t i = 0; i < capacity(); ++i) {
      if (_flag_at(i) == FLAG_USED) {
        fmt::print(" {} -> ({},{})\n", i, _values[i].first, _values[i].second);
      } else if (_flag_at(i) == FLAG_TOMB) {
        fmt::print(" {} -> TONTON\n", i);
      } else {
        fmt::print(" {} -> x\n", i);
      }
    }
    fmt::print("END\n");
  }

public:
  reference operator[](const key_type& key) {
    auto it = find(key);
    assert(it != end());
    return *it;
  }
  reference at(const key_type& key) {
    auto it = find(key);
    if (it == end()) {
      throw std::out_of_range{fmt::format("Key '{}' not found", key)};
    }
    return *it;
  }

  const_reference operator[](const key_type& key) const {
    auto it = find(key);
    assert(it != end());
    return *it;
  }
  const_reference at(const key_type& key) const {
    auto it = find(key);
    if (it == end()) {
      throw std::out_of_range{fmt::format("Key '{}' not found", key)};
    }
    return *it;
  }

  iterator begin() { return iterator{this, 0u}; }
  const_iterator begin() const { return const_iterator{this, 0u}; }
  const_iterator cbegin() const { return const_iterator{this, 0u}; }

  iterator end() { return iterator{this, capacity()}; }
  const_iterator end() const { return const_iterator{this, capacity()}; }
  const_iterator cend() const { return const_iterator{this, capacity()}; }

  size_type size() const { return _used; }
  size_type capacity() const { return _cap; }
  float load_factor() const { return static_cast<float>(size())/static_cast<float>(capacity()); }

  const hasher& hash_function() const { return static_cast<const hasher&>(*this); }
  const key_equal& key_eq() const { return static_cast<const key_equal&>(*this); }

private:
  uint32_t _flag_at(size_t idx) const {
    const uint32_t flag_idx = idx/FLAGS_PER_ENTRY;
    const uint32_t shift = (idx%FLAGS_PER_ENTRY)*2;
    return (_flags[flag_idx] >> shift) & FLAG_MASK;
  }

  void _flag_set(size_t idx, uint32_t flag) {
    const uint32_t flag_idx = idx/FLAGS_PER_ENTRY;
    const uint32_t shift = (idx%FLAGS_PER_ENTRY)*2;
    _flags[flag_idx] &= ~(FLAG_MASK << shift);
    _flags[flag_idx] |= (flag & FLAG_MASK) << shift;
  }

private:
  uint32_t* _flags;
  value_type* _values;
  size_t _used, _cap;

public:
  ~fixed_hashmap() noexcept { clear(); }

  fixed_hashmap(fixed_hashmap&& other) noexcept :
    ops_base{static_cast<ops_base&&>(other)},
    _flags{std::move(other._flags)}, _values{std::move(other._values)},
    _used{std::move(other._used)}, _cap{std::move(other._cap)}
  {
    other._values = nullptr;
  }

  fixed_hashmap& operator=(fixed_hashmap&& other) noexcept {
    ops_base::operator=(static_cast<ops_base&&>(other));

    clear();

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

} // namespace ntf
