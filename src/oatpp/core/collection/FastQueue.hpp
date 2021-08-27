/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef oatpp_collection_FastQueue_hpp
#define oatpp_collection_FastQueue_hpp

#include "oatpp/core/concurrency/SpinLock.hpp"
#include "oatpp/core/base/Environment.hpp"

namespace oatpp { namespace collection {
  
template<typename T>
class FastQueue {
public:

  class iterator {
   friend class FastQueue<T>;

   private:
    T* i;
    T* prev;
    iterator(T* pRef, T* pLast = nullptr) : i(pRef), prev(pLast) {}

   public:
    iterator(const iterator& other) {
      i = other.i;
      prev = other.prev;
    }
    ~iterator() {

    }
    iterator& operator=(const iterator& other) {
      if (this != &other) {
        i = other.i;
        prev = other.prev;
      }
      return *this;
    }
    bool operator==(const iterator& other) {
      return i == other.i;
    }
    bool operator!=(const iterator& other) {
      return i != other.i;
    }
    iterator& operator++() {
      prev = i;
      i = i->_next;
      return *this;
    }
    T* operator*() const {
      return i;
    }
    friend void swap(iterator& lhs, iterator& rhs) {
      T* tmp = lhs.i;
      lhs.i = rhs.i;
      rhs.i = tmp;
      tmp = lhs.prev;
      lhs.prev = rhs.prev;
      rhs.prev = tmp;
    }
  };

  FastQueue()
    : first(nullptr)
    , last(nullptr)
    , count(0)
  {}
  
  ~FastQueue(){
    clear();
  }

  FastQueue(const FastQueue &) = delete;

  FastQueue(FastQueue &&other) noexcept
    : FastQueue() {
      using std::swap;
      swap(first, other.first);
      swap(last, other.last);
      swap(count, other.count);
  }

  FastQueue &operator=(const FastQueue &) = delete;

  FastQueue &operator=(FastQueue &&other) noexcept {
      if (this != std::addressof(other)) {
          using std::swap;
          swap(first, other.first);
          swap(last, other.last);
          swap(count, other.count);
      }
      return *this;
  }

  
  T* first;
  T* last;
  v_int32 count{};

  v_int32 Count() {
      return count;
  }

  bool empty() {
      return count == 0;
  }
  
  void pushFront(T* entry) {
    entry->_next = first;
    first = entry;
    if(last == nullptr) {
      last = first;
    }
    ++ count;
  }
  
  void pushBack(T* entry) {
    entry->_next = nullptr;
    if(last == nullptr) {
      first = entry;
      last = entry;
    } else {
      last->_next = entry;
      last = entry;
    }
    ++ count;
  }
  
  void round(){
    if(count > 1) {
      last->_next = first;
      last = first;
      first = first->_next;
      last->_next = nullptr;
    }
  }
  
  T* popFront() {
    T* result = first;
    first = first->_next;
    if(first == nullptr) {
      last = nullptr;
    }
    -- count;
    return result;
  }
  
  void popFrontNoData() {
    T* result = first;
    first = first->_next;
    if(first == nullptr) {
      last = nullptr;
    }
    delete result;
    -- count;
  }

  static void moveAll(FastQueue& fromQueue, FastQueue& toQueue) {

    if(fromQueue.count > 0) {

      if (toQueue.last == nullptr) {
        toQueue.first = fromQueue.first;
        toQueue.last = fromQueue.last;
      } else {
        toQueue.last->_next = fromQueue.first;
        toQueue.last = fromQueue.last;
      }

      toQueue.count += fromQueue.count;
      fromQueue.count = 0;

      fromQueue.first = nullptr;
      fromQueue.last = nullptr;

    }

  }

  void cutEntry(T* entry, T* prevEntry){

    if(prevEntry == nullptr) {
      popFront();
    } else {
      prevEntry->_next = entry->_next;
      -- count;
      if(prevEntry->_next == nullptr) {
        last = prevEntry;
      }
    }

  }
  
  void clear() {
    T* curr = first;
    while (curr != nullptr) {
      T* next = curr->_next;
      delete curr;
      curr = next;
    }
    first = nullptr;
    last = nullptr;
    count = 0;
  }

  iterator begin() const {
    return iterator(first, nullptr);
  }

  iterator end() const {
    return iterator(nullptr, last);
  }

  void erase(iterator& i) {
    if(i.prev == nullptr) {
      popFront();
      i.i = first;
    } else {
      i.prev->_next = i.i->_next;
      -- count;
      if(i.prev->_next == nullptr) {
        last = i.prev;
      }
    }
  }
  
};
  
}}

#endif /* FastQueue_hpp */
