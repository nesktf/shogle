#pragma once

#include <shogle/core/allocator.hpp>

namespace ntf {

template<typename T>
struct entity_traits {
  static constexpr std::size_t arena_block = 4096;
};

template<typename T>
class entity_pool {
private:
  static constexpr std::size_t _arena_block = entity_traits<T>::arena_block;

public:
  struct node {
    node(node* prev_ = nullptr, node* next_ = nullptr) :
      prev(prev_), next(next_), dummy() {}

    template<typename... Args>
    void construct(Args&&... args) {
      std::construct_at(&ent, std::forward<Args>(args)...);
    }

    void destroy() {
      ent.~T();
      dummy = char{};
    }

    union {
      T ent;
      char dummy;
    };
    node *prev, *next;
  };

public:
  entity_pool() { _arena.init(); }
  
public:
  node* allocate(std::size_t n) {
    if (_free_count < n) {
      _append_extra_nodes(n - _free_count);
    }
    return _give_existing_nodes(n);
  }

  void deallocate(node* obj) {
    if (!obj) {
      return;
    }

    obj->prev = nullptr;
    obj->next = _free;
    _free->prev = obj;
    _free = obj;
    ++_free_count;
  }

private:
  void _append_extra_nodes(std::size_t extra) {
    node* extra_nodes =
      reinterpret_cast<node*>(_arena.allocate(extra*sizeof(node), alignof(node)));

    for (std::size_t i = 0; i < extra; ++i) {
      std::construct_at(&extra_nodes[i],
        i == 0 ? nullptr : &extra_nodes[i-1],    // prev
        i == extra-1 ? _free : &extra_nodes[i+1] // next
      );
    }
    _free->prev = extra_nodes[extra-1];
    _free = extra_nodes;
    _free_count += extra;
  }

  node* _give_existing_nodes(std::size_t n) {
    node* out = nullptr;
    if (_free_count == n) {
      out = _free;
      _free = nullptr;
      return out;
    }

    std::size_t diff = _free_count - n;
    // ....
    return out;
  }

private:
  ntf::memory_arena<_arena_block> _arena;

  node* _free{nullptr};
  std::size_t _free_count{0};
};

template<typename T>
class entity_list {
private:
  template<bool is_const>
  class base_iterator_node {
  public:
    base_iterator_node(entity_pool<T>::node* node) :
      _node(node) {}

  protected:
    entity_pool<T>::node* _node;
  };

  template<>
  class base_iterator_node<true> {
  public:
    base_iterator_node(const entity_pool<T>::node* node) :
      _node(node) {}

  protected:
    const entity_pool<T>::node* _node;
  };

  template<bool is_const, bool is_reverse>
  class base_iterator : public base_iterator_node<is_const> {
  public:
    template<typename = T> requires(!is_const)
    base_iterator(entity_pool<T>::node* node) :
      base_iterator_node<is_const>(node) {}

    template<typename = T> requires(is_const)
    base_iterator(const entity_pool<T>::node* node) :
      base_iterator_node<is_const>(node) {}

  public:
    template<typename = T> requires(!is_const)
    T& operator*() { return this->_node->ent;}

    const T& operator*() const { return this->_node->ent; }

    template<typename = T> requires(!is_const)
    T* operator->() { return &this->_node->ent; }

    const T* operator->() const { return &this->_node->ent; }

    base_iterator& operator++() {
      if (this->_node) {
        if constexpr (is_reverse) {
          this->_node = this->_node->prev;
        } else {
          this->_node = this->_node->next;
        }
      }
      return *this;
    }

    base_iterator& operator--() {
      if (this->_node) {
        if constexpr (is_reverse) {
          this->_node = this->_node->next;
        } else {
          this->_node = this->_node->prev;
        }
      }
      return *this;
    }

    bool operator==(const base_iterator& other) const { return (this->_node == other._node); }
    bool operator!=(const base_iterator& other) const { return (this->_node != other._node); }
  };

public:
  using iterator = base_iterator<false, false>;
  using reverse_iterator = base_iterator<false, true>;
  using const_iterator = base_iterator<true, false>;
  using const_reverse_iterator = base_iterator<true, true>;
 
public:
  entity_list(entity_pool<T>* pool) :
    _pool(pool) {}

public:
  template<typename... Args>
  T& emplace_back(Args&&... args) {
    _end->next =_pool->allocate(1);
    _end->next->prev = _end;
    _end = _end->next;
    _end->construct(std::forward<Args>(args)...);
    ++_size;
    return _end->ent;
  }

  template<typename... Args>
  T& emplace_front(Args&&... args) {
    _begin->prev = _pool->allocate(1);
    _begin->prev->next = _begin;
    _begin = _begin->prev;
    _begin->construct(std::forward<Args>(args)...);
    ++_size;
    return _begin->ent;
  }

  template<typename U>
  T& push_back(U&& obj) {
    _end->next = _pool->allocate(1);
    _end->next->prev = _end;
    _end = _end->next;
    _end->construct(std::forward<U>(obj));
    ++_size;
    return _end->ent;
  }

  template<typename U>
  T& push_front(U&& obj) {
    _begin->prev = _pool->allocate(1);
    _begin->prev->next = _begin;
    _begin = _begin->prev;
    _begin->construct(std::forward<U>(obj));
    ++_size;
    return _begin->ent;
  }

  void pop_back() {
    auto* temp = _end;
    temp->destroy();
    _end = temp->prev;
    _end->next = nullptr;
    _pool->deallocate(temp);
    --_size;
  }

  void pop_front() {
    auto* temp = _begin;
    temp->destroy();
    _begin = temp->next;
    _begin->prev = nullptr;
    _pool->deallocate(temp);
    --_size;
  }

public:
  std::size_t size() const { return _size; }

  iterator begin() { return iterator{_begin}; }
  const_iterator begin() const { return const_iterator{_begin}; }
  const_iterator cbegin() const { return const_iterator{_begin}; }

  iterator end() { return iterator{nullptr}; }
  const_iterator end() const { return const_iterator{nullptr}; }
  const_iterator cend() const { return const_iterator{nullptr}; }

  reverse_iterator rbegin() { return reverse_iterator{_end}; }
  const_reverse_iterator rbegin() const { return const_reverse_iterator{_end}; }
  const_reverse_iterator crbegin() const { return const_reverse_iterator{_end}; }

  reverse_iterator rend() { return reverse_iterator{nullptr}; }
  const_reverse_iterator rend() const { return const_reverse_iterator{nullptr}; }
  const_reverse_iterator crend() { return const_reverse_iterator{nullptr}; }

private:
  entity_pool<T>* _pool;
  entity_pool<T>::node *_begin{nullptr}, *_end{nullptr};
  std::size_t _size{0};

public:
  ~entity_list() {
    while (_begin) {
      _begin->destroy();
      _pool->deallocate(_begin);
      _begin = _begin->next;
    }
  }
  entity_list(entity_list&&) = default;
  entity_list& operator=(entity_list&&) = default;
};

} // namespace ntf
