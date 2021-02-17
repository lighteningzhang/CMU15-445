//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// buffer_pool_manager.cpp
//
// Identification: src/buffer/buffer_pool_manager.cpp
//
// Copyright (c) 2015-2019, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#include "buffer/buffer_pool_manager.h"
#include <include/common/logger.h>
#include <list>
#include <unordered_map>
#include "storage/disk/disk_manager.h"

namespace bustub {

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager *disk_manager, LogManager *log_manager)
    : pool_size_(pool_size), disk_manager_(disk_manager), log_manager_(log_manager) {
  // We allocate a consecutive memory space for the buffer pool.
  pages_ = new Page[pool_size_];
  replacer_ = new LRUReplacer(pool_size);

  // Initially, every page is in the free list.
  for (size_t i = 0; i < pool_size_; ++i) {
    free_list_.emplace_back(static_cast<int>(i));
  }
}

BufferPoolManager::~BufferPoolManager() {
  delete[] pages_;
  delete replacer_;
}

Page *BufferPoolManager::FetchPageImpl(page_id_t page_id) {
  // 1.     Search the page table for the requested page (P).
  // 1.1    If P exists, pin it and return it immediately.
  // 1.2    If P does not exist, find a replacement page (R) from either the free list or the replacer.
  //        Note that pages are always found from the free list first.
  // 2.     If R is dirty, write it back to the disk.
  // 3.     Delete R from the page table and insert P.
  // 4.     Update P's metadata, read in the page content from disk, and then return a pointer to P.
  if (page_table_.find(page_id) != page_table_.end()) {
    LOG_INFO("Fetch from page table");
    frame_id_t frameId = page_table_[page_id];
    Page *frame = pages_ + frameId;
    // neglect
    frame->pin_count_++;
    return frame;
  }

  Page *frame = nullptr;
  if (!free_list_.empty()) {
    frame_id_t freeFrameId = free_list_.front();
    free_list_.pop_front();
    frame = pages_ + freeFrameId;
    frame->ResetMemory();
    frame->page_id_ = page_id;
    disk_manager_->ReadPage(page_id, frame->data_);
    frame->pin_count_ = 1;
    frame->is_dirty_ = false;
    page_table_[page_id] = freeFrameId;
  } else {
    frame_id_t victimFrameId = INVALID_PAGE_ID;
    if (replacer_->Victim(&victimFrameId) == false) return nullptr;
    frame = pages_ + victimFrameId;
    page_id_t victimPageId = frame->GetPageId();
    // neglect
    if (frame->IsDirty()) {
      FlushPage(victimPageId);
    }
    page_table_.erase(victimPageId);

    frame->ResetMemory();
    frame->pin_count_ = 1;
    frame->page_id_ = page_id;
    frame->is_dirty_ = false;
    disk_manager_->ReadPage(page_id, frame->data_);
    page_table_[page_id] = victimFrameId;
  }

  return frame;
}

bool BufferPoolManager::UnpinPageImpl(page_id_t page_id, bool is_dirty) {
  if (page_table_.find(page_id) == page_table_.end()) return false;

  frame_id_t frameId = page_table_[page_id];
  Page *frame = pages_ + frameId;
  int framePinCnt = frame->GetPinCount();

  if (framePinCnt == 0) {
    return false;
  }

  // ???
  if (is_dirty) {
    FlushPage(page_id);
  }

  // ???
  frame->is_dirty_ |= is_dirty;
  frame->pin_count_--;
  // ???
  if (frame->pin_count_ == 0) {
    replacer_->Unpin(frameId);
  }
  return true;
}

bool BufferPoolManager::FlushPageImpl(page_id_t page_id) {
  // Make sure you call DiskManager::WritePage!
  if (page_table_.find(page_id) == page_table_.end()) {
    return false;
  }

  frame_id_t frameId = page_table_[page_id];
  Page *frame = pages_ + frameId;
  disk_manager_->WritePage(page_id, frame->GetData());
  frame->is_dirty_ = false;
  return true;
}

Page *BufferPoolManager::NewPageImpl(page_id_t *page_id) {
  // 0.   Make sure you call DiskManager::AllocatePage!
  // 1.   If all the pages in the buffer pool are pinned, return nullptr.
  // 2.   Pick a victim page P from either the free list or the replacer. Always pick from the free list first.
  // 3.   Update P's metadata, zero out memory and add P to the page table.
  // 4.   Set the page ID output parameter. Return a pointer to P.
  page_id_t newPageId = disk_manager_->AllocatePage();
  // *page_id = newPageId;
  if (!free_list_.empty()) {
    frame_id_t freeFrameId = free_list_.front();
    free_list_.pop_front();
    Page *frame = pages_ + freeFrameId;
    frame->ResetMemory();
    frame->page_id_ = newPageId;
    frame->pin_count_ = 1;
    frame->is_dirty_ = true;
    page_table_[newPageId] = freeFrameId;
    *page_id = newPageId;
    return frame;
  }

  frame_id_t replaceFrameId = INVALID_PAGE_ID;
  bool update = replacer_->Victim(&replaceFrameId);
  if (!update) {
    return nullptr;
  }

  Page *frame = pages_ + replaceFrameId;
  page_id_t victimPageId = frame->GetPageId();
  if (frame->IsDirty()) {
    FlushPage(victimPageId);
  }
  frame->ResetMemory();
  page_table_.erase(victimPageId);
  page_table_[newPageId] = replaceFrameId;

  frame->page_id_ = newPageId;
  frame->pin_count_ = 1;
  frame->is_dirty_ = true;
  *page_id = newPageId;
  // LOG_INFO("Successfully new a page of frame %d from disk %d, the data is %s", victimPageId, newPageId,
  // frame->data_);
  return frame;
}

bool BufferPoolManager::DeletePageImpl(page_id_t page_id) {
  // 0.   Make sure you call DiskManager::DeallocatePage!
  // 1.   Search the page table for the requested page (P).
  // 1.   If P does not exist, return true.
  // 2.   If P exists, but has a non-zero pin-count, return false. Someone is using the page.
  // 3.   Otherwise, P can be deleted. Remove P from the page table, reset its metadata and return it to the free list.
  return false;
}

void BufferPoolManager::FlushAllPagesImpl() {
  // You can do it!
}

}  // namespace bustub
