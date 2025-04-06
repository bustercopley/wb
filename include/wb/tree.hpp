#pragma once

#include "node.hpp"

// An ordered associative container representing an arbitrary sequence
// of values and allowing binary search with arbitrary comparators.
// The user must establish the precondition that the sequence is ordered
// compatibly with the comparators before performing a binary search.

// This may be useful in situations where the order changes dynamically, such
// as the Bentley-Ottmann algorithm [BentleyOttmann1979] and similar sweep-line
// algorithms. It is implemented as a weight-balanced tree [HiraiYamamoto2011].

// The class template 'tree' provides the following standard container methods:
//   ~tree(); // destructor
//   tree(); // default constructor
//   iterator begin();
//   iterator end();
//   const_iterator begin() const;
//   const_iterator end() const;
//   std::size_t size() const;
//   bool empty() const;

// The method 'insert(position, value)' inserts 'value' before 'position',
// which must be a valid iterator pointing to an element or to the end of
// the sequence. It returns an iterator to the inserted element.
// No existing iterators are invalidated.

// The method 'erase(position)' erases the element pointed to by the
// iterator 'position', which must be a valid iterator pointing to an
// element. Iterators to the erased element are invalidated. No other
// iterators are invalidated.

// The method 'exchange_elements(i, j)' exchanges the elements pointed
// to by the iterators 'i' and 'j', which must be valid iterators pointing
// to elements, without moving any other values in the sequence. No iterators
// are invalidated. No iterators are invalidated, but iterators equal to
// 'i' or 'j' are relocated: after 'exchange_elements' returns they point
// to the old element in its new position in the sequence.

// The binary search methods 'lower_bound(cmp)', 'upper_bound(cmp)',
// 'equal_range(cmp)' assume that the tree is partitioned by the
// comparator 'cmp', that is, there are iterators 'i' and 'j' such that
//   cmp(x) returns -1 for each element x in [begin, i)
//   cmp(x) returns 0 for each element x in [i, j)
//   cmp(x) returns +1 for each element x in [j, end)
// Then 'lower_bound' returns 'i', 'upper_bound' returns 'j' and
// 'equal_range' returns the tuple '(i, j)'.

// The remaining binary search method 'equal_range(lcmp, rcmp)' assumes
// that the tree is partitioned by both comparators 'lcmp' and 'rcmp'
// and that 'lcmp(x) <= rcmp(x)' for all elements 'x' and returns the
// tuple '(i, j)' of iterators such that
//   lcmp(x) returns -1 if and only if x is in [begin, i)
//   rcmp(x) returns +1 if and only if x is in [j, end)

namespace wb {

template <typename T> struct tree {
private:
  detail::node<T> sentinel_;

public:
  struct iterator {
  private:
    friend struct tree<T>;
    detail::node<T> *p_;
    iterator(detail::node<T> *p): p_(p) {}

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
      detail::node<T> *oldp = p_;
      p_ = inorder_successor(p_);
      return iterator{oldp};
    }

    iterator &operator--() {
      p_ = inorder_predecessor(p_);
      return *this;
    }

    iterator operator--(int) {
      detail::node<T> *oldp = p_;
      p_ = inorder_predecessor(p_);
      return iterator{oldp};
    }

    bool operator!=(const iterator &other) const { return p_ != other.p_; }
    bool operator==(const iterator &other) const { return p_ == other.p_; }
  };

  struct const_iterator {
  private:
    friend struct tree<T>;
    const detail::node<T> *p_;
    const_iterator(const detail::node<T> *p): p_(p) {}

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
      const detail::node<T> *oldp = p_;
      p_ = inorder_successor(p_);
      return const_iterator{oldp};
    }

    const_iterator &operator--() {
      p_ = inorder_predecessor(p_);
      return *this;
    }

    const_iterator operator--(int) {
      detail::node<T> *oldp = p_;
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
    detail::node<T> *p = sentinel_.left_;
    if (p) { p->delete_subtree(); }
  }

  tree() {
    sentinel_.left_ = nullptr;
    sentinel_.right_ = nullptr;
    sentinel_.parent_ = &sentinel_;
  }

  iterator begin() {
    detail::node<T> *p = &sentinel_;
    while (p->left_) { p = p->left_; }
    return iterator{p};
  }
  iterator end() { return iterator(&sentinel_); }
  const_iterator begin() const {
    const detail::node<T> *p = &sentinel_;
    while (p->left_) { p = p->left_; }
    return const_iterator{p};
  }
  const_iterator end() const { return const_iterator(&sentinel_); }

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

  // Return an iterator to the first element 'x' in the tree which satisfies
  // 'cmp(x) >= 0', or if no such element exists, the past-the-end sentinel
  template <typename Comp> iterator lower_bound(Comp &&cmp) {
    if (detail::node<T> *p = sentinel_.left_) {
      return iterator(lower_bound_node(p, (Comp &&)cmp));
    } else {
      return iterator(&sentinel_);
    }
  }

  // Return an iterator to the first element 'x' in the tree which satisfies
  // 'cmp(x) > 0', or if no such element exists, the past-the-end sentinel
  template <typename Comp> iterator upper_bound(Comp &&cmp) {
    if (detail::node<T> *p = sentinel_.left_) {
      return iterator(upper_bound_node(p, (Comp &&)cmp));
    } else {
      return iterator(&sentinel_);
    }
  }

  // Return a range containing all elements 'x' in the tree which satisfy
  // 'cmp(x) == 0'
  template <typename Comp>
  std::tuple<iterator, iterator> equal_range(Comp &&cmp) {
    if (detail::node<T> *p = sentinel_.left_) {
      auto [l, r] = equal_range_nodes(p, (Comp &&)cmp);
      return std::make_tuple(iterator(l), iterator(r));
    } else {
      return std::make_tuple(iterator(&sentinel_), iterator(&sentinel_));
    }
  }

  // Return a range containing all elements 'x' in the tree which satisfy
  // 'lcmp(x) >= 0' and 'rcmp(x) <= 0', assuming the tree is partitioned
  // with respect to both comparators
  template <typename LComp, typename RComp>
  std::tuple<iterator, iterator> range_between(LComp &&lcmp, RComp &&rcmp) {
    if (detail::node<T> *p = sentinel_.left_) {
      auto [l, r] = range_between_nodes(p, (LComp &&)lcmp, (RComp &&)rcmp);
      return std::make_tuple(iterator(l), iterator(r));
    } else {
      return std::make_tuple(iterator(&sentinel_), iterator(&sentinel_));
    }
  }
};

static_assert(std::bidirectional_iterator<tree<int>::iterator>);
static_assert(std::bidirectional_iterator<tree<int>::const_iterator>);

}
