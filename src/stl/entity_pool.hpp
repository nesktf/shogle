#pragma once

#include "./allocator.hpp"

namespace ntf {

using pool_handle = uint32_t;
static constexpr pool_handle tombstone = UINT32_MAX;

template<typename T, typename Alloc = std::allocator<T>>
class entity_pool {
public:
  using value_type = T;
  using allocator_type = Alloc;

private:
  struct node {
    node(pool_handle prev_ = tombstone, pool_handle next_ = tombstone) :
      prev(prev_), next(next_), dummy(), active(false) {} 

    template<typename... Args>
    void construct(Args&&... args) {
      new (&obj) T{std::forward<Args>(args)...};
      active = true;
    }

    void destroy() {
      obj.~T();
      dummy = char{};
      active = false;
    }

    union {
      T obj;
      char dummy;
    };
    bool active;
    pool_handle prev, next;
  };
  using node_alloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<node>;

  struct block {
    node* nodes;
    std::size_t sz;
    std::size_t used;
  };
  using block_alloc = typename std::allocator_traits<allocator_type>::template rebind_alloc<block>;

  class entity {
  public:
    entity(entity_pool& pool, pool_handle handle) :
      _pool(std::addressof(pool)), _handle(handle) {}

  public:
    const T& get() const {
      NTF_ASSERT(_pool && _handle != tombstone);
      return _pool->_node_at(_handle).obj;
    }

    T& get() {
      NTF_ASSERT(_pool && _handle != tombstone);
      return _pool->_node_at(_handle).obj;
    }

    T* operator->() { return std::addressof(get()); }
    const T* operator->() const { return std::addressof(get()); }

    T& operator*() { return get(); }
    const T& operator*() const { return get(); }

  private:
    entity_pool* _pool;
    pool_handle _handle;

  private:
    friend class entity_pool;
  };

  class entity_list {
  public:
    entity_list(entity_pool& pool, pool_handle head) :
      _pool(std::addressof(pool)), _head(head) {}

  private:
    entity_pool* _pool;
    pool_handle _head;

  private:
    friend class entity_pool;
  };


public:
  entity_pool() = default;
  entity_pool(std::size_t initial_alloc) { reserve(initial_alloc); }

  entity_pool(allocator_type& alloc) :
    _block_alloc(alloc), _node_alloc(alloc) {}
  entity_pool(allocator_type& alloc, std::size_t initial_alloc) :
    _block_alloc(alloc), _node_alloc(alloc) { reserve(initial_alloc); }

public:
  template<typename... Args>
  entity construct(Args&&... args) {
    _NTF_ASSERT_avail(1);

    pool_handle curr = _avail;
    node& out = _node_at(curr);
    out.construct(std::forward<Args>(args)...);

    _avail = out.next;
    _avail_count--;
    out.next = tombstone;
    out.prev = tombstone;
    _active++;

    return entity{*this, curr};
  }

  template<typename... Args>
  entity_list construct_list(std::size_t count, Args&&... args) {
    _NTF_ASSERT_avail(count);
    return entity_list{*this, _avail};
  }

  void destroy(entity& ent) {
    NTF_ASSERT(ent._pool == this);
    node& ent_node = _node_at(ent._handle);
    ent_node.destroy();

    if (_avail != tombstone) {
      node& head = _node_at(_avail);
      head.prev = ent._handle;
    } 
    ent_node.next = _avail;
    ent_node.prev = tombstone;
    _avail = ent._handle;
    _avail_count++;
    _active--;
    ent._handle = tombstone;
    ent._pool = nullptr;
  }

  void destroy(entity_list& list) {
    NTF_ASSERT(list._pool == this);

  }

  void reserve(std::size_t required) {
    std::size_t prev_last = _block_count;

    block* new_arr = _block_alloc.allocate(++_block_count);
    new (new_arr) block{};

    if (_blocks) {
      std::memcpy(new_arr, _blocks, prev_last*sizeof(block));
      _block_alloc.deallocate(_blocks, _block_count);
    }
    _blocks = new_arr;

    // Allocate a block with the double of the previous size
    // or the next power of two from the required size
    // whatever is bigger
    _block_size = std::max(
      _block_size*2, 
      static_cast<std::size_t>(std::exp2(std::ceil(std::log2(required))))
    );
    _allocated += _block_size;
    fmt::print("> Reserving block of size {}\n", _block_size);

    node* nodes = _node_alloc.allocate(_block_size);
    for (std::size_t i = 0; i < _block_size; ++i) {
      new (nodes) node{};
    }

    block* last = &_blocks[_block_count-1];
    last->nodes = nodes;
    last->sz = _block_size;
    last->used = 0;
  }

  void clear() {
    if (!_blocks) {
      return;
    }

    if (_active != 0) {
      for (std::size_t i = 0 ; i < _block_count; ++i) {
        block& curr_block = _blocks[i];
        for (std::size_t j = 0; j < curr_block.used; ++j) {
          node& curr_node = curr_block.nodes[j];
          if (curr_node.active) {
            curr_node.destroy();
          }
        }
      }
    }

    for (std::size_t i = 0; i < _block_count; ++i) {
      _node_alloc.deallocate(_blocks[i].nodes, _blocks[i].sz);
    }
    _block_alloc.deallocate(_blocks, _block_count);

    _reset();
  }

private:
  void _NTF_ASSERT_avail(std::size_t count) {
    NTF_ASSERT(count > 0);
    if (count <= _avail_count) {
      return;
    }
    if (_block_count == 0) {
      reserve(count);
    }

    pool_handle last_avail = _avail;
    std::size_t ptr_block_index = _block_index(_block_ptr);
    std::size_t ptr_block_offset = _node_index(_block_ptr);
    NTF_ASSERT(ptr_block_index < _block_count);

    for (std::size_t i = 0; i < count; ++i) {
      fmt::print("> ptr: {:b} -> {} {}\n", _block_ptr, ptr_block_index, ptr_block_offset);
      block& curr_block = _blocks[ptr_block_index];
      node& curr_node = curr_block.nodes[ptr_block_offset];

      if (last_avail != tombstone) {
        node& last_node = _blocks[_block_index(last_avail)].nodes[_node_index(last_avail)];
        last_node.prev = _block_ptr;
      }
      curr_node.next = last_avail;
      curr_node.prev = tombstone;

      curr_block.used++;
      last_avail = ((ptr_block_index << 24) | ptr_block_offset);

      if (curr_block.sz == curr_block.used) {
        ptr_block_index++;
        ptr_block_offset = 0;
        if (ptr_block_index == _block_count) {
          reserve(count);
        }
      } else {
        ptr_block_offset++;
      }

      _block_ptr = ((ptr_block_index << 24) | ptr_block_offset);
    }

    _avail = last_avail;
    _avail_count += count;
  }

  std::size_t _block_index(pool_handle handle) {
    NTF_ASSERT(handle != tombstone);
    return (handle & (0xFF << 24)) >> 24;
  }

  std::size_t _node_index(pool_handle handle) {
    NTF_ASSERT(handle != tombstone);
    return (handle & 0x00FFFFFF);
  }

  node& _node_at(pool_handle handle) {
    NTF_ASSERT(handle != tombstone);

    std::size_t block_index = _block_index(handle);
    NTF_ASSERT(block_index < _block_count);

    block& b = _blocks[block_index];
    std::size_t node_index = _node_index(handle);
    NTF_ASSERT(node_index < b.sz);

    return b.nodes[node_index];
  }

  void _reset() {
    _blocks = nullptr;
    _block_count = 0;
    _block_size = 1;
    _block_ptr = 0;

    _allocated = 0;
    _active = 0;

    _avail = tombstone;
    _avail_count = 0;
  }

public:
  std::size_t available() const { return _avail; }
  std::size_t allocated() const { return _allocated; }
  std::size_t size() const { return _active; }

private:
  block_alloc _block_alloc{};
  node_alloc _node_alloc{};

  block* _blocks{nullptr};
  std::size_t _block_count{0};
  std::size_t _block_size{1};
  pool_handle _block_ptr{0};

  std::size_t _allocated{0};
  std::size_t _active{0};

  pool_handle _avail{tombstone};
  std::size_t _avail_count{0};

private:
  friend class entity;
  friend class entity_list;

public:
  ~entity_pool() noexcept { clear(); }
  entity_pool(const entity_pool&) = delete;
  entity_pool(entity_pool&&) = delete;
  entity_pool& operator=(const entity_pool&) = delete;
  entity_pool& operator=(entity_pool&&) = delete;
};



// template<typename T>
// class entity_pool {
// public:
//   using entity_tag = uint32_t; // 24 bits index, 8 bits block
//
//   struct entity {
//   public:
//     entity(entity_pool& pool, entity_tag tag) :
//       _pool(pool), _tag(tag) {}
//
//   private:
//     entity_tag _tag;
//     entity_pool* _pool;
//   };
//
//   struct node {
//     node(node* prev_ = nullptr, node* next_ = nullptr) :
//       prev(prev_), next(next_), dummy() {}
//
//     template<typename... Args>
//     void construct(Args&&... args) {
//       std::construct_at(&ent, std::forward<Args>(args)...);
//     }
//
//     void destroy() {
//       ent.~T();
//       dummy = char{};
//     }
//
//     union {
//       T ent;
//       char dummy;
//     };
//     node *prev, *next;
//   };
//
// public:
//   entity_pool() = default;
//   
// public:
//   void insert_block(std::size_t sz) {
//     void* mem = std::malloc(sz*sizeof(T));
//     NTF_ASSERT(mem);
//     if (!_blocks) {
//       void* arr_mem = std::malloc(sizeof(node**));
//       _blocks = reinterpret_cast<node**>(arr_mem);
//       _block_count += 1;
//       _blocks[0] = reinterpret_cast<node*>(mem);
//     } else if (_avail_blocks <= _block_count){
//     }
//   }
//
//   node* allocate(std::size_t n) {
//     if (_free_count < n) {
//       _append_extra_nodes(n - _free_count);
//     }
//     return _give_existing_nodes(n);
//   }
//
//   void deallocate(node* obj) {
//     if (!obj) {
//       return;
//     }
//
//     obj->prev = nullptr;
//     obj->next = _free;
//     _free->prev = obj;
//     _free = obj;
//     ++_free_count;
//   }
//
// private:
//   // void _append_extra_nodes(std::size_t extra) {
//   //   node* extra_nodes =
//   //     reinterpret_cast<node*>(_arena.allocate(extra*sizeof(node), alignof(node)));
//   //
//   //   for (std::size_t i = 0; i < extra; ++i) {
//   //     std::construct_at(&extra_nodes[i],
//   //       i == 0 ? nullptr : &extra_nodes[i-1],    // prev
//   //       i == extra-1 ? _free : &extra_nodes[i+1] // next
//   //     );
//   //   }
//   //   _free->prev = extra_nodes[extra-1];
//   //   _free = extra_nodes;
//   //   _free_count += extra;
//   // }
//   //
//   // node* _give_existing_nodes(std::size_t n) {
//   //   node* out = nullptr;
//   //   if (_free_count == n) {
//   //     out = _free;
//   //     _free = nullptr;
//   //     return out;
//   //   }
//   //
//   //   std::size_t diff = _free_count - n;
//   //   // ....
//   //   return out;
//   // }
//
// private:
//   std::vector<std::vector<node>> _blocks;
// };
//
// template<typename T>
// class entity_list {
// private:
//   template<bool is_const>
//   class base_iterator_node {
//   public:
//     base_iterator_node(entity_pool<T>::node* node) :
//       _node(node) {}
//
//   protected:
//     entity_pool<T>::node* _node;
//   };
//
//   template<>
//   class base_iterator_node<true> {
//   public:
//     base_iterator_node(const entity_pool<T>::node* node) :
//       _node(node) {}
//
//   protected:
//     const entity_pool<T>::node* _node;
//   };
//
//   template<bool is_const, bool is_reverse>
//   class base_iterator : public base_iterator_node<is_const> {
//   public:
//     template<typename = T> requires(!is_const)
//     base_iterator(entity_pool<T>::node* node) :
//       base_iterator_node<is_const>(node) {}
//
//     template<typename = T> requires(is_const)
//     base_iterator(const entity_pool<T>::node* node) :
//       base_iterator_node<is_const>(node) {}
//
//   public:
//     template<typename = T> requires(!is_const)
//     T& operator*() { return this->_node->ent;}
//
//     const T& operator*() const { return this->_node->ent; }
//
//     template<typename = T> requires(!is_const)
//     T* operator->() { return &this->_node->ent; }
//
//     const T* operator->() const { return &this->_node->ent; }
//
//     base_iterator& operator++() {
//       if (this->_node) {
//         if constexpr (is_reverse) {
//           this->_node = this->_node->prev;
//         } else {
//           this->_node = this->_node->next;
//         }
//       }
//       return *this;
//     }
//
//     base_iterator& operator--() {
//       if (this->_node) {
//         if constexpr (is_reverse) {
//           this->_node = this->_node->next;
//         } else {
//           this->_node = this->_node->prev;
//         }
//       }
//       return *this;
//     }
//
//     bool operator==(const base_iterator& other) const { return (this->_node == other._node); }
//     bool operator!=(const base_iterator& other) const { return (this->_node != other._node); }
//   };
//
// public:
//   using iterator = base_iterator<false, false>;
//   using reverse_iterator = base_iterator<false, true>;
//   using const_iterator = base_iterator<true, false>;
//   using const_reverse_iterator = base_iterator<true, true>;
//  
// public:
//   entity_list(entity_pool<T>* pool) :
//     _pool(pool) {}
//
// public:
//   template<typename... Args>
//   T& emplace_back(Args&&... args) {
//     _end->next =_pool->allocate(1);
//     _end->next->prev = _end;
//     _end = _end->next;
//     _end->construct(std::forward<Args>(args)...);
//     ++_size;
//     return _end->ent;
//   }
//
//   template<typename... Args>
//   T& emplace_front(Args&&... args) {
//     _begin->prev = _pool->allocate(1);
//     _begin->prev->next = _begin;
//     _begin = _begin->prev;
//     _begin->construct(std::forward<Args>(args)...);
//     ++_size;
//     return _begin->ent;
//   }
//
//   template<typename U>
//   T& push_back(U&& obj) {
//     _end->next = _pool->allocate(1);
//     _end->next->prev = _end;
//     _end = _end->next;
//     _end->construct(std::forward<U>(obj));
//     ++_size;
//     return _end->ent;
//   }
//
//   template<typename U>
//   T& push_front(U&& obj) {
//     _begin->prev = _pool->allocate(1);
//     _begin->prev->next = _begin;
//     _begin = _begin->prev;
//     _begin->construct(std::forward<U>(obj));
//     ++_size;
//     return _begin->ent;
//   }
//
//   void pop_back() {
//     auto* temp = _end;
//     temp->destroy();
//     _end = temp->prev;
//     _end->next = nullptr;
//     _pool->deallocate(temp);
//     --_size;
//   }
//
//   void pop_front() {
//     auto* temp = _begin;
//     temp->destroy();
//     _begin = temp->next;
//     _begin->prev = nullptr;
//     _pool->deallocate(temp);
//     --_size;
//   }
//
// public:
//   std::size_t size() const { return _size; }
//
//   iterator begin() { return iterator{_begin}; }
//   const_iterator begin() const { return const_iterator{_begin}; }
//   const_iterator cbegin() const { return const_iterator{_begin}; }
//
//   iterator end() { return iterator{nullptr}; }
//   const_iterator end() const { return const_iterator{nullptr}; }
//   const_iterator cend() const { return const_iterator{nullptr}; }
//
//   reverse_iterator rbegin() { return reverse_iterator{_end}; }
//   const_reverse_iterator rbegin() const { return const_reverse_iterator{_end}; }
//   const_reverse_iterator crbegin() const { return const_reverse_iterator{_end}; }
//
//   reverse_iterator rend() { return reverse_iterator{nullptr}; }
//   const_reverse_iterator rend() const { return const_reverse_iterator{nullptr}; }
//   const_reverse_iterator crend() { return const_reverse_iterator{nullptr}; }
//
// private:
//   entity_pool<T>* _pool;
//   entity_pool<T>::node *_begin{nullptr}, *_end{nullptr};
//   std::size_t _size{0};
//
// public:
//   ~entity_list() {
//     while (_begin) {
//       _begin->destroy();
//       _pool->deallocate(_begin);
//       _begin = _begin->next;
//     }
//   }
//   entity_list(entity_list&&) = default;
//   entity_list& operator=(entity_list&&) = default;
// };

} // namespace ntf
