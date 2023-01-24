#pragma once

#include <cstddef>
#include <iostream>
#include <memory>

template <typename T> struct tree;

template <typename T> struct node {
  T value_;
  std::size_t size_;
  node<T> *left_;
  node<T> *right_;
  node<T> *parent_;

  // Construct a singleton
  template <typename U>
  node(U &&value):
      value_((U &&)value), size_(1), left_(nullptr), right_(nullptr),
      parent_(nullptr) {}

private:
  friend class tree<T>;
  node() = default;

  friend std::size_t size(const node<T> *s) { return s ? s->size_ : 0; }

  friend bool is_sentinel(node<T> *p) { return p->parent_ == p; }

  friend node<T> *inorder_successor(node<T> *p) {
    if (p->right_) {
      p = p->right_;
      while (p->left_) { p = p->left_; }
    } else {
      while (!is_sentinel(p->parent_) && p == p->parent_->right_) {
        p = p->parent_;
      }
      p = p->parent_;
    }
    return p;
  }

  friend node<T> *inorder_predecessor(node<T> *p) {
    if (p->left_) {
      p = p->left_;
      while (p->right_) { p = p->right_; }
    } else {
      while (!is_sentinel(p->parent_) && p == p->parent_->left_) {
        p = p->parent_;
      }
      p = p->parent_;
    }
    return p;
  }

  friend node<T> *postorder_successor(node<T> *p) {
    node<T> *q = p->parent_;
    if (p == q->left_) {
      if (q->right_) {
        q = q->right_;
        while (q->left_ || q->right_) { q = (q->left_ ? q->left_ : q->right_); }
      }
    }
    return q;
  }

  friend node<T> *&owner(node<T> *p) {
    return p->parent_->left_ == p ? p->parent_->left_ : p->parent_->right_;
  }

  void recalculate_size() { size_ = size(left_) + size(right_) + 1; }

  bool is_balanced(node<T> *left, node<T> *right) {
    return 3 * (size(left) + 1) >= size(right) + 1;
    return true;
  }

  bool is_single(node<T> *left, node<T> *right) {
    return size(left) + 1 <= 2 * (size(right) + 1);
    return true;
  }

  // Returns the subtree
  node<T> *balance_left() {
    if (!is_balanced(left_, right_)) {
      if (is_single(right_->left_, right_->right_)) {
        node<T> *a = this;
        node<T> *b = a->right_;
        node<T> *c = b->left_;
        owner(a) = b;
        b->parent_ = a->parent_;
        a->right_ = c;
        if (c) { c->parent_ = a; }
        b->left_ = a;
        a->parent_ = b;
        recalculate_size();
        b->recalculate_size();
        return b;
      } else {
        node<T> *a = this;
        node<T> *b = a->right_;
        node<T> *c = b->left_;
        node<T> *d = c->left_;
        node<T> *e = c->right_;
        owner(a) = c;
        c->parent_ = a->parent_;
        a->right_ = d;
        if (d) { d->parent_ = a; }
        b->left_ = e;
        if (e) { e->parent_ = b; }
        c->left_ = a;
        a->parent_ = c;
        c->right_ = b;
        b->parent_ = c;
        a->recalculate_size();
        b->recalculate_size();
        c->recalculate_size();
        return c;
      }
    } else {
      return this;
    }
  }

  // Returns the subtree
  node<T> *balance_right() {
    if (!is_balanced(right_, left_)) {
      if (is_single(left_->right_, left_->left_)) {
        node<T> *a = this;
        node<T> *b = a->left_;
        node<T> *c = b->right_;
        owner(a) = b;
        b->parent_ = a->parent_;
        a->left_ = c;
        if (c) { c->parent_ = a; }
        b->right_ = a;
        a->parent_ = b;
        recalculate_size();
        b->recalculate_size();
        return b;
      } else {
        node<T> *a = this;
        node<T> *b = a->left_;
        node<T> *c = b->right_;
        node<T> *d = c->right_;
        node<T> *e = c->left_;
        owner(a) = c;
        c->parent_ = a->parent_;
        a->left_ = d;
        if (d) { d->parent_ = a; }
        b->right_ = e;
        if (e) { e->parent_ = b; }
        c->right_ = a;
        a->parent_ = c;
        c->left_ = b;
        b->parent_ = c;
        a->recalculate_size();
        b->recalculate_size();
        c->recalculate_size();
        return c;
      }
    } else {
      return this;
    }
  }

  void balance_above(int increment) {
    node<T> *p = this;
    while (!is_sentinel(p->parent_)) {
      bool is_right = p == p->parent_->right_;
      p = p->parent_;
      p->size_ += increment;
      if (is_right == (increment > 0)) {
        p = p->balance_left();
      } else {
        p = p->balance_right();
      }
    }
  }

  template <typename U> node<T> *insert_before_self(U &&value) {
    node<T> *result = new node<T>((U &&)value);
    if (left_) {
      node<T> *p = left_;
      while (p->right_) { p = p->right_; }
      p->right_ = result;
      result->parent_ = p;
    } else {
      left_ = result;
      result->parent_ = this;
    }
    result->balance_above(1);
    return result;
  }

  void replace_self(node<T> *p) {
    owner(this) = p;
    if (p) { p->parent_ = parent_; }
  }

  void erase_self() {
    if (!left_ || !right_) {
      --parent_->size_;
      node<T> *p = !left_ ? right_ : left_;
      if (p) { p->parent_ = parent_; }
      if (this == parent_->left_) {
        parent_->left_ = p;
        if (!is_sentinel(parent_)) {
          p = parent_->balance_left();
          p->balance_above(-1);
        }
      } else {
        parent_->right_ = p;
        p = parent_->balance_right();
        p->balance_above(-1);
      }
      delete this;
    } else {
      node<T> *p = inorder_successor(this);
      if (p != right_) {
        node<T> *q = p->parent_;
        q->left_ = p->right_;
        if (p->right_) { p->right_->parent_ = q; }
        p->right_ = right_;
        right_->parent_ = p;
        p->left_ = left_;
        left_->parent_ = p;
        replace_self(p);
        p->size_ = size_;
        --q->size_;
        q = q->balance_left();
        q->balance_above(-1);
        delete this;
      } else {
        replace_self(p);
        p->left_ = left_;
        left_->parent_ = p;
        p->size_ = size_ - 1;
        p = p->balance_right();
        p->balance_above(-1);
        delete this;
      }
    }
  }

  friend void exchange_nodes(node<T> *p, node<T> *q) {
    std::swap(p->size_, q->size_);
    if (p->parent_ == q) { std::swap(p, q); }

    if (q->parent_ == p) {
      owner(p) = q;
      q->parent_ = p->parent_;
      p->parent_ = q;
      if (q == p->left_) {
        p->left_ = q->left_;
        q->left_ = p;
        std::swap(p->right_, q->right_);
        if (p->left_) { p->left_->parent_ = p; }
        if (p->right_) { p->right_->parent_ = p; }
        if (q->right_) { q->right_->parent_ = q; }
      } else {
        p->right_ = q->right_;
        q->right_ = p;
        std::swap(p->left_, q->left_);
        if (p->left_) { p->left_->parent_ = p; }
        if (p->right_) { p->right_->parent_ = p; }
        if (q->left_) { q->left_->parent_ = q; }
      }
    } else {
      std::swap(owner(p), owner(q));
      std::swap(p->parent_, q->parent_);
      std::swap(p->left_, q->left_);
      std::swap(p->right_, q->right_);
      if (p->left_) { p->left_->parent_ = p; }
      if (p->right_) { p->right_->parent_ = p; }
      if (q->left_) { q->left_->parent_ = q; }
      if (q->right_) { q->right_->parent_ = q; }
    }
  }

  void delete_subtree() {
    node<T> *p = this;
    while (p->left_) { p = p->left_; }
    while (p != this) {
      node<T> *q = postorder_successor(p);
      delete p;
      p = q;
    }
    delete this;
  }

  friend node<T> *lower_bound_node(node<T> *p, auto &&cmp) {
    while (true) {
      if (cmp(p->value_) < 0) {
        if (p->right_) {
          p = p->right_;
        } else {
          p = inorder_successor(p);
          break;
        }
      } else {
        if (p->left_) {
          p = p->left_;
        } else {
          break;
        }
      }
    }
    return p;
  }

  friend node<T> *upper_bound_node(node<T> *p, auto &&cmp) {
    while (true) {
      if (cmp(p->value_) <= 0) {
        if (p->right_) {
          p = p->right_;
        } else {
          p = inorder_successor(p);
          break;
        }
      } else {
        if (p->left_) {
          p = p->left_;
        } else {
          break;
        }
      }
    }
    return p;
  }

#if TESTING
  friend std::ostream &dump(std::ostream &stream, node<T> *subtree, int indentation = 0) {
    for (int i = 0; i != indentation; ++i) { stream.put(' '); }
    if (subtree) {
      stream << subtree->value_                 //
             << "; address " << (void *)subtree //
             << "; size " << size(subtree);
      if (subtree == subtree->parent_->left_) {
        stream << "; left child of " << (void *)subtree->parent_;
      } else if (subtree == subtree->parent_->right_) {
        stream << "; right child of " << (void *)subtree->parent_;
      } else {
        stream << "; disowned by " << (void *)subtree->parent_;
      }
      if (subtree->left_ && subtree->left_->parent_ != subtree) {
        stream << "; bad left_->parent " << (void *)subtree->left_->parent_;
      }
      if (subtree->right_ && subtree->right_->parent_ != subtree) {
        stream << "; bad right_->parent " << (void *)subtree->right_->parent_;
      }
      stream << '\n';
      dump(stream, subtree->left_, indentation + 2);
      dump(stream, subtree->right_, indentation + 2);
      return stream;
    } else {
      return stream << "null\n";
    }
  }
#endif
};
