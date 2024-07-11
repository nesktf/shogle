#pragma once

#include <array>
#include <optional>
#include <cstdint>

namespace ntf {

template<typename T, typename id_t, size_t packed_size, size_t sparse_size>
class sparse_set {
private:
  using data_type = T;
  using id_type = id_t;
  static_assert(packed_size <= sparse_size &&
                packed_size > 0 && sparse_size > 0, "Invalid set sizes");
  static_assert(std::is_integral_v<id_type>, "id_type must be integral");

  // using packed_data_type = std::array<data_type, packed_size>;
  using packed_data_type = std::array<std::pair<data_type, id_type>, packed_size>;
  // using packed_id_type = std::array<id_type, packed_size>;
  using sparse_index_type = std::array<size_t, sparse_size>;

public:
  using iterator = typename packed_data_type::iterator;
  using const_iterator = typename packed_data_type::const_iterator;

public:
  explicit sparse_set() = default;

public:
  bool insert(id_type id, data_type val) {
    if (full() || valid(id)) {
      return false;
    }

    // _packed_data[_count] = std::move(val);
    // _packed_ids[_count] = id;
    _packed_data[_count] = std::make_pair(std::move(val), id);
    _sparse_indices[id] = _count+1;

    _count++;
    return true;
  }

  template<typename... Args>
  bool emplace(id_type id, Args&&... args) {
    if (full() || valid(id)) {
      return false;
    }
    
    // _packed_data[_count] = data_type{std::forward<Args>(args)...};
    // _packed_ids[_count] = id;
    _packed_data[_count] = std::make_pair(data_type{std::forward<Args>(args)...}, id);
    _sparse_indices[id] = _count+1;
    
    _count++;
    return true;
  }

  bool remove(id_type id) {
    if (empty() || !valid(id)) {
      return false;
    }

    // If is not the last element, swap it with the last
    // auto index = _sparse_indices[id]-1;
    auto index = _sparse_indices[id]-1;
    auto last_packed = _count-1;
    if (index != last_packed) {
      _sparse_indices[_packed_data[last_packed].second] = index+1;
      // _sparse_indices[_packed_ids[last_packed]] = index+1;
      std::swap(_packed_data[last_packed], _packed_data[index]);
      // std::swap(_packed_ids[last_packed], _packed_ids[index]);
    }

    _sparse_indices[id] = 0; // Sparce index is now invalid
    _count--; // Pretend the last packed element is empty
    return true;
  }

public:
  [[nodiscard]] data_type& get(id_type id) { return _packed_data[_sparse_indices[id]-1].first; }
  [[nodiscard]] const data_type& get(id_type id) const { return get(id); }
  [[nodiscard]] data_type& operator[](id_type id) { return get(id); }
  [[nodiscard]] const data_type& operator[](id_type id) const { return get(id); }

  [[nodiscard]] std::optional<data_type*> opt(id_type id) {
    if (!valid(id)) {
      return std::optional<data_type*>{};
    }
    return std::optional{&_packed_data[_sparse_indices[id]-1].first};
  }
  [[nodiscard]] const std::optional<data_type*> opt(id_type id) const { return opt(id); }

public:
  bool valid(id_type id) const { return _sparse_indices[id] != 0; }
  bool full() const { return _count == packed_size; }
  bool empty() const { return _count == 0; }
  size_t count() const { return _count; }
  constexpr size_t pack_size() const { return packed_size; }
  constexpr size_t spar_size() const { return sparse_size; }

  const_iterator begin() const { return _packed_data.begin(); }
  const_iterator end() const { return _packed_data.begin()+_count; }
  const_iterator cbegin() const { return _packed_data.cbegin(); }
  const_iterator cend() const { return _packed_data.cbegin()+_count; }
  iterator begin() { return _packed_data.begin(); }
  iterator end() { return _packed_data.begin()+_count; }

private:
  packed_data_type _packed_data;
  // packed_id_type _packed_ids;
  sparse_index_type _sparse_indices{};
  size_t _count {0};
};

} // namespace ntf
