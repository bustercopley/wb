#pragma once

#include "node.hpp"

// Weight-balanced tree [1]

// Note:

// The ordering of items in the dictionary is not given by a static order
// relation known ahead of time, but is maintained dynamically by the line-
// sweep algorithm.

// The algorithm guarantees that at the point in time when a binary search is
// performed, the items in the dictionary are partitioned about the supplied
// order relation.

// This situation is not handled by the standard library associative containers.

// [1] Balancing weight-balanced trees
//     Yoichi Hirai
//     Journal of Functional Programming 21(3): 287-301, 2011

template <typename T> struct tree {
private:
  node<T> sentinel_;

public:
  struct iterator {
  private:
    friend struct tree<T>;
    node<T> *p_;
    iterator(node<T> *p): p_(p) {}

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T *;
    using reference = T &;
    using iterator_category = std::bidirectional_iterator_tag;

    // Singular value required for range iterator
    iterator(): p_(nullptr) {}

    T &operator*() const { return p_->value_; }
    T *operator->() const { return &p_->value_; }

    iterator &operator++() {
      p_ = inorder_successor(p_);
      return *this;
    }

    iterator operator++(int) {
      node<T> *oldp = p_;
      p_ = inorder_successor(p_);
      return iterator{oldp};
    }

    iterator &operator--() {
      p_ = inorder_predecessor(p_);
      return *this;
    }

    iterator operator--(int) {
      node<T> *oldp = p_;
      p_ = inorder_predecessor(p_);
      return iterator{oldp};
    }

    bool operator!=(const iterator &other) const { return p_ != other.p_; }
    bool operator==(const iterator &other) const { return p_ == other.p_; }
  };

  struct const_iterator {
  private:
    friend class tree<T>;
    const node<T> *p_;
    const_iterator(node<T> *p): p_(p) {}

  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = const T *;
    using reference = const T &;
    using iterator_category = std::bidirectional_iterator_tag;

    // Singular value required for range iterator
    const_iterator(): p_(nullptr) {}

    // Convert iterator to const_iterator
    const_iterator(const iterator &other): p_(other.p_) {}

    const T &operator*() const { return p_->value_; }
    const T *operator->() const { return &p_->value_; }

    const_iterator &operator++() {
      p_ = inorder_successor(p_);
      return *this;
    }

    const_iterator operator++(int) {
      const node<T> *oldp = p_;
      p_ = inorder_successor(p_);
      return const_iterator{oldp};
    }

    const_iterator &operator--() {
      p_ = inorder_predecessor(p_);
      return *this;
    }

    const_iterator operator--(int) {
      node<T> *oldp = p_;
      p_ = inorder_predecessor(p_);
      return const_iterator{oldp};
    }

    bool operator!=(const iterator &other) const { return p_ != other.p_; }
    bool operator==(const iterator &other) const { return p_ == other.p_; }
    bool operator!=(const const_iterator &other) const {
      return p_ != other.p_;
    }
    bool operator==(const const_iterator &other) const {
      return p_ == other.p_;
    }
  };

  friend bool operator!=(const iterator &a, const const_iterator &b) {
    return b != a;
  }
  friend bool operator==(const iterator &a, const const_iterator &b) {
    return b == a;
  }

public:
  ~tree() {
    node<T> *p = sentinel_.left_;
    if (p) { p->delete_subtree(); }
  }

  tree() {
    sentinel_.left_ = nullptr;
    sentinel_.right_ = nullptr;
    sentinel_.parent_ = &sentinel_;
  }

  iterator begin() {
    node<T> *p = &sentinel_;
    while (p->left_) { p = p->left_; }
    return iterator(p);
  }

  iterator end() { return iterator(&sentinel_); }

  const_iterator begin() const { return begin(); }

  const_iterator end() const { return end(); }

  std::size_t size() const { return empty() ? 0 : sentinel_.left_->size_; }

  bool empty() const { return !sentinel_.left_; }

  template <typename U> iterator insert(iterator position, U &&value) {
    return iterator(position.p_->insert_before_self((U &&)value));
  }

  iterator erase(iterator position) {
    iterator result(inorder_successor(position.p_));
    position.p_->erase_self();
    return result;
  }

  void exchange_elements(iterator i, iterator j) { exchange_nodes(i.p_, j.p_); }

  // Searching assumes that the tree is partitioned by the comparator, that is
  // there are iterators i and j such that
  //   cmp(n.value) returns -1 for each node n in [begin, i),
  //   cmp(n.value) returns 0 for each node n in [i, j),
  //   cmp(n.value) returns +1 for each node n in [j, end)

  // Return an iterator to the first element 'x' in the tree which satisfies
  // 'cmp(x) >= 0', or if no such element exists, the past-the-end sentinel
  template <typename Comp> iterator lower_bound(Comp &&cmp) {
    if (node<T> *p = sentinel_.left_) {
      return iterator(lower_bound_node(p, (Comp &&)cmp));
    } else {
      return iterator(&sentinel_);
    }
  }

  // Return an iterator to the first element 'x' in the tree which satisfies
  // 'cmp(x) > 0', or if no such element exists, the past-the-end sentinel
  template <typename Comp> iterator upper_bound(Comp &&cmp) {
    if (node<T> *p = sentinel_.left_) {
      return iterator(upper_bound_node(p, (Comp &&)cmp));
    } else {
      return iterator(&sentinel_);
    }
  }

  template <typename Comp>
  std::tuple<iterator, iterator> equal_range(Comp &&cmp) {
    if (node<T> *p = sentinel_.left_) {
      auto [l, r] = equal_range_nodes(p, (Comp &&)cmp);
      return std::make_tuple(iterator(l), iterator(r));
    } else {
      return std::make_tuple(iterator(&sentinel_), iterator(&sentinel_));
    }
  }

  template <typename LComp, typename RComp>
  std::tuple<iterator, iterator> equal_range(LComp &&lcmp, RComp &&rcmp) {
    if (node<T> *p = sentinel_.left_) {
      auto [l, r] = equal_range_nodes(p, (LComp &&)lcmp, (RComp &&)rcmp);
      return std::make_tuple(iterator(l), iterator(r));
    } else {
      return std::make_tuple(iterator(&sentinel_), iterator(&sentinel_));
    }
  }

#if TESTING
  friend std::ostream &dump(std::ostream &stream, tree<T> &dictionary) {
    return dump(stream, dictionary.sentinel_.left_, 0);
  }
#endif
};

static_assert(std::bidirectional_iterator<tree<int>::iterator>);
static_assert(std::bidirectional_iterator<tree<int>::const_iterator>);
