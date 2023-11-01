// Copyright 2023 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_CONTAINERS_LINKED_LIST_H_
#define BASE_CONTAINERS_LINKED_LIST_H_

#include "base/debug/logging.h"

namespace base {

template <typename T>
class LinkedList;

template <typename T>
class LinkNode {
 public:
  LinkNode() : previous_(nullptr), next_(nullptr) {}
  ~LinkNode() { RemoveFromList(); }

  LinkNode(const LinkNode &) = delete;
  LinkNode &operator=(const LinkNode &) = delete;

  T *value() { return static_cast<T *>(this); }
  const T *value() const { return static_cast<const T *>(this); }

  LinkNode<T> *previous() { return previous_; }
  LinkNode<T> *next() { return next_; }

  void RemoveFromList() {
    if (previous_ && next_) {
      next_->previous_ = previous_;
      previous_->next_ = next_;
    }

    previous_ = nullptr;
    next_ = nullptr;
  }

 private:
  friend class LinkedList<T>;
  LinkNode<T> *previous_;
  LinkNode<T> *next_;
};

template <typename T>
class LinkedList {
 public:
  LinkedList() : root_() {
    root_.previous_ = &root_;
    root_.next_ = &root_;
  }

  void Prepend(LinkNode<T> *node) {
    root_.next_->previous_ = node;
    node->previous_ = &root_;
    node->next_ = root_.next_;
    root_.next_ = node;
  }

  void Append(LinkNode<T> *node) {
    root_.previous_->next_ = node;
    node->next_ = &root_;
    node->previous_ = root_.previous_;
    root_.previous_ = node;
  }

  void InsertBefore(LinkNode<T> *prev, LinkNode<T> *node) {
    node->next_ = prev;
    node->previous_ = prev->previous_;
    prev->previous_->next_ = node;
    prev->previous_ = node;
  }

  LinkNode<T> *head() { return root_.next_; }
  LinkNode<T> *end() { return &root_; }

  bool empty() const { return root_.next_ == &root_; }

 private:
  LinkNode<T> root_;
};

}  // namespace base

#endif  // BASE_CONTAINERS_LINKED_LIST_H_
