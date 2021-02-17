//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.h
//
// Identification: src/include/buffer/lru_replacer.h
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <list>
#include <map>
#include <mutex>  // NOLINT
#include <unordered_map>
#include <vector>
#include "buffer/replacer.h"
#include "common/config.h"

namespace bustub {

/**
 * LRUReplacer implements the lru replacement policy, which approximates the Least Recently Used policy.
 */
class LRUReplacer : public Replacer {
 public:
  /**
   * Create a new LRUReplacer.
   * @param num_pages the maximum number of pages the LRUReplacer will be required to store
   */
  explicit LRUReplacer(size_t num_pages);

  /**
   * Destroys the LRUReplacer.
   */
  ~LRUReplacer() override;

  bool Victim(frame_id_t *frame_id) override;

  void Pin(frame_id_t frame_id) override;

  void Unpin(frame_id_t frame_id) override;

  size_t Size() override;

  void insert(frame_id_t frame_id);

  void erase(frame_id_t frame_id);

 private:
  struct ListNode {
    frame_id_t key, val;
    ListNode *nxt = nullptr;
    ListNode *pre = nullptr;
    ListNode(int key_, int val_) : key(key_), val(val_) {}
    explicit ListNode(int key_) : key(key_), val(0) {}
  };
  ListNode *head, *tail;
  std::unordered_map<frame_id_t, ListNode *> hash;
  size_t capacity, size;
  // TODO(student): implement me!
};

}  // namespace bustub
