//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// lru_replacer.cpp
//
// Identification: src/buffer/lru_replacer.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/lru_replacer.h"

namespace bustub {
void LRUReplacer::insert(frame_id_t frame_id) {
  ListNode *newNode = new ListNode(frame_id);
  ListNode *nxtNode = head->nxt;
  // add in list
  head->nxt = newNode;
  newNode->pre = head;

  newNode->nxt = nxtNode;
  nxtNode->pre = newNode;

  // add in hash table
  hash[newNode->key] = newNode;
  size++;
}

void LRUReplacer::erase(frame_id_t frame_id) {
  ListNode *rmNode = hash[frame_id];
  ListNode *preNode = rmNode->pre, *nxtNode = rmNode->nxt;
  preNode->nxt = nxtNode;
  nxtNode->pre = preNode;
  hash.erase(frame_id);
  size--;
}

LRUReplacer::LRUReplacer(size_t num_pages) {
  head = new ListNode(-1, -1);
  tail = new ListNode(-1, -1);
  head->nxt = tail;
  tail->pre = head;
  capacity = num_pages;
  size = 0;
}

LRUReplacer::~LRUReplacer() = default;

bool LRUReplacer::Victim(frame_id_t *frame_id) {
  if (size == 0) {
    return false;
  }

  frame_id_t val = tail->pre->key;
  *frame_id = val;
  erase(val);
  return true;
}

void LRUReplacer::Pin(frame_id_t frame_id) {
  if (hash.find(frame_id) != hash.end()) {
    ListNode *curNode = hash[frame_id];
    ListNode *nxtNode = curNode->nxt, *preNode = curNode->pre;
    nxtNode->pre = preNode;
    preNode->nxt = nxtNode;
    hash.erase(frame_id);
    size--;
  }
}

void LRUReplacer::Unpin(frame_id_t frame_id) {
  if (hash.count(frame_id) == 0) {
    if (size == capacity) {
      frame_id_t val = tail->pre->key;
      erase(val);
    }
    insert(frame_id);
  }
}

size_t LRUReplacer::Size() { return size; }

}  // namespace bustub
