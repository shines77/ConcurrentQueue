#pragma once

#include <queue>
#include <deque>
#include <type_traits>

#include "common.h"
#include "NativeMutex.h"
#include "LockedRingQueue.h"
#include "SingleRingQueue.h"

template <typename T, typename MutexType>
class StdQueueWrapper
{
public:
    typedef std::queue<T>   queue_type;
    typedef T               item_type;
    typedef MutexType       mutex_type;

private:
    mutex_type mutex_;
    queue_type queue_;

public:
    StdQueueWrapper() : mutex_(), queue_() {}
    ~StdQueueWrapper() {}

    bool empty() const {
        native::scoped_lock<mutex_type> lock(mutex_);
        bool is_empty = queue_.empty();
        return is_empty;
    }

    size_t sizes() const {
        native::scoped_lock<mutex_type> lock(mutex_);
        size_t size = queue_.sizes();
        return size;
    }

    void resize(size_t new_size) {
        // Do nothing!!
    }

    template <typename U>
    void push(U && item) {
        native::scoped_lock<mutex_type> lock(mutex_);
        queue_.push(std::forward<U>(item));
    }

    item_type & front() {
        native::scoped_lock<mutex_type> lock(mutex_);
        item_type & item = queue_.front();
        return item;
    }

    item_type & back() {
        native::scoped_lock<mutex_type> lock(mutex_);
        item_type & item = queue_.back();
        return item;
    }

    void pop() {
        native::scoped_lock<mutex_type> lock(mutex_);
        queue_.pop();
    }

    int pop(item_type & item) {
        native::scoped_lock<mutex_type> lock(mutex_);
        if (!queue_.empty()) {
            item_type & ret = queue_.back();
            item = std::move(ret);
            queue_.pop();
            return true;
        }
        else {
            return false;
        }
    }
};

template <typename T, typename MutexType>
class StdDequeueWrapper
{
public:
    typedef std::deque<T>   queue_type;
    typedef T               item_type;
    typedef MutexType       mutex_type;

private:
    mutex_type mutex_;
    queue_type queue_;

public:
    StdDequeueWrapper() : mutex_(), queue_() {}
    ~StdDequeueWrapper() {}

    bool empty() const {
        native::scoped_lock<mutex_type> lock(mutex_);
        bool is_empty = queue_.empty();
        return is_empty;
    }

    size_t sizes() const {
        native::scoped_lock<mutex_type> lock(mutex_);
        size_t size = queue_.sizes();
        return size;
    }

    void resize(size_t new_size) {
        native::scoped_lock<mutex_type> lock(mutex_);
        queue_.resize(new_size);
    }

    template <typename U>
    void push(U && item) {
        native::scoped_lock<mutex_type> lock(mutex_);
        queue_.push_back(std::forward<U>(item));
    }

    item_type & front() {
        native::scoped_lock<mutex_type> lock(mutex_);
        item_type & item = queue_.front();
        return item;
    }

    item_type & back() {
        native::scoped_lock<mutex_type> lock(mutex_);
        item_type & item = queue_.back();
        return item;
    }

    void pop() {
        native::scoped_lock<mutex_type> lock(mutex_);
        queue_.pop_front();
    }

    int pop(item_type & item) {
        native::scoped_lock<mutex_type> lock(mutex_);
        if (!queue_.empty()) {
            item_type & ret = queue_.back();
            item = std::move(ret);
            queue_.pop_front();
            return true;
        }
        else {
            return false;
        }
    }
};

template <typename T, typename MutexType, typename IndexType>
class LockedRingQueueWrapper
{
public:
    typedef LockedRingQueue<T, MutexType, IndexType> queue_type;
    typedef T item_type;

private:
    queue_type queue_;

public:
    LockedRingQueueWrapper() : queue_() {}
    ~LockedRingQueueWrapper() {}

    bool empty() const {
        return queue_.is_empty();
    }

    size_t sizes() const {
        return queue_.sizes();
    }

    void resize(size_t new_size) {
        queue_.resize(new_size);
    }

    template <typename U>
    void push(U && item) {
        queue_.push_front(std::forward<U>(item));
    }

    item_type & back() {
        item_type item;
        if (queue_.pop_back(item) == QUEUE_OP_SUCCESS) {
            return std::move(item);
        }
        else {
            throw ("LockedRingQueue<T> is empty!");
        }
    }

    void pop() {
        item_type item;
        queue_.pop_back(item);
    }

    int pop(item_type & item) {
        return queue_.pop_back(item);
    }
};

template <typename T, typename MutexType, typename IndexType,
          size_t InitCapacity = kQueueDefaultCapacity>
class FixedLockedRingQueueWrapper
{
public:
    typedef FixedLockedRingQueue<T, MutexType, IndexType, InitCapacity> queue_type;
    typedef T item_type;

private:
    queue_type queue_;

public:
    FixedLockedRingQueueWrapper() : queue_() {}
    ~FixedLockedRingQueueWrapper() {}

    bool empty() const {
        return queue_.is_empty();
    }

    size_t sizes() const {
        return queue_.sizes();
    }

    void resize(size_t new_size) {
        queue_.resize(new_size);
    }

    template <typename U>
    void push(U && item) {
        queue_.push_front(std::forward<U>(item));
    }

    item_type & back() {
        item_type item;
        if (queue_.pop_back(item) == QUEUE_OP_SUCCESS) {
            return std::move(item);
        }
        else {
            throw ("FixedLockedRingQueue<T> is empty!");
        }
    }

    void pop() {
        item_type item;
        queue_.pop_back(item);
    }

    int pop(item_type & item) {
        return queue_.pop_back(item);
    }
};

template <typename T, typename SequenceType,
          size_t InitCapacity = kQueueDefaultCapacity>
class FixedSingleRingQueueWrapper
{
public:
    typedef FixedSingleRingQueue<T, SequenceType, InitCapacity> queue_type;
    typedef T item_type;

private:
    queue_type queue_;

public:
    FixedSingleRingQueueWrapper() : queue_() {}
    ~FixedSingleRingQueueWrapper() {}

    bool empty() const {
        return queue_.is_empty();
    }

    size_t sizes() const {
        return queue_.sizes();
    }

    void resize(size_t new_size) {
        queue_.resize(new_size);
    }

    template <typename U>
    void push(U && item) {
        queue_.push(std::forward<U>(item));
    }

    item_type & back() {
        item_type item;
        if (queue_.pop(item) == QUEUE_OP_SUCCESS) {
            return std::move(item);
        }
        else {
            throw ("SingleRingQueue<T> is empty!");
        }
    }

    void pop() {
        item_type item;
        queue_.pop(item);
    }

    int pop(item_type & item) {
        return queue_.pop(item);
    }
};
