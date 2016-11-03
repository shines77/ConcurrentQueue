
#ifndef LOCKEDRINGQUEUE_H
#define LOCKEDRINGQUEUE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <stddef.h>
#include <assert.h>

#include "common.h"
#include "PowerOf2.h"

template <typename T, typename ItemType, typename MutexType>
class LockedRingQueueAbstract {
public:
    typedef T                   impl_type;
    typedef ItemType            item_type;
    typedef MutexType           mutex_type;
    typedef std::size_t         size_type;

    typedef LockedRingQueueAbstract<T, ItemType, MutexType> this_type;

public:
    LockedRingQueueAbstract() {}
    virtual ~LockedRingQueueAbstract() {}

private:
    inline impl_type * staic_cast_to_this() {
        impl_type * pThis = static_cast<impl_type *>(this);
        assert(pThis != nullptr);
        return pThis;
    }

    inline const impl_type * const_cast_to_this() const {
        this_type * pOrigThis = const_cast<this_type *>(this);
        impl_type * pThisNonConst = static_cast<impl_type *>(pOrigThis);
        const impl_type * pThisConst = const_cast<const impl_type *>(pThisNonConst);
        assert(pThisConst != nullptr);
        return pThisConst;
    }

public:
    bool is_valid() const {
        const impl_type * pThis = const_cast_to_this();
        return (pThis->entries_ != nullptr);
    }

    bool is_empty() const {
        const impl_type * pThis = const_cast_to_this();
        std::unique_lock<mutex_type> lock(pThis->mutex_);
        bool _isEmpty = (pThis->head_ == pThis->tail_);
        assert((pThis->head_ - pThis->tail_) <= pThis->capacity());
        return _isEmpty;
    }

    size_type capacity() const {
        const impl_type * pThis = const_cast_to_this();
        return pThis->capacity_;
    }

    size_type sizes() const {
        const impl_type * pThis = const_cast_to_this();
        std::unique_lock<mutex_type> lock(pThis->mutex_);
        size_type size = pThis->head_ - pThis->tail_;
        assert(size <= pThis->capacity());
        return size;
    }

    int push_front(item_type const & item) {
        impl_type * pThis = staic_cast_to_this();
        return pThis->push_front_impl(item);
    }

    int push_front(item_type && item) {
        impl_type * pThis = staic_cast_to_this();
        return pThis->push_front_impl(std::move(item));
    }

    int pop_back(item_type & item) {
        impl_type * pThis = staic_cast_to_this();
        return pThis->pop_back_impl(item);
    }

    int push(item_type const & item) {
        return this->push_front(item);
    }

    int push(item_type && item) {
        return this->push_front(item);
    }

    int pop(item_type & item) {
        return this->pop_back(item);
    }
};

template <typename T, typename MutexType = std::mutex,
          typename IndexType = uint64_t,
          size_t InitCapacity = kQueueDefaultCapacity>
class FixedLockedRingQueue :
    public LockedRingQueueAbstract<FixedLockedRingQueue<T, MutexType, IndexType, InitCapacity>,
                                   T, MutexType> {
public:
    typedef T               item_type;
    typedef T *             value_type;
    typedef MutexType       mutex_type;
    typedef IndexType       index_type;
    typedef std::size_t     size_type;

    static const index_type kInitCursor = (index_type)(-1);
    static const index_type kDefaultCapacity = (index_type)compile_time::round_to_pow2<kQueueDefaultCapacity>::value;
    static const index_type kCapacity = (index_type)compile_time::round_to_pow2<InitCapacity>::value;
    static const index_type kIndexMask = (index_type)(kCapacity - 1);
    static const size_type  kAlignment = compile_time::round_to_pow2<kCacheLineSize>::value;

    //template <typename U, typename T>
    //friend class LockedRingQueueAbstract;

private:
    index_type          head_;
    index_type          tail_;
    value_type          entries_;
    size_type           capacity_;
    value_type          allocEntries_;
    size_type           allocSize_;

public:
    mutable mutex_type  mutex_;

public:
    FixedLockedRingQueue()
        : head_(kInitCursor), tail_(kInitCursor), entries_(nullptr),
          capacity_(kCapacity), allocEntries_(nullptr), allocSize_(0), mutex_() {
        init();
    }

    virtual ~FixedLockedRingQueue() {
        mutex_.lock();
        free_queue();
        mutex_.unlock();
    }

private:
    inline void init() {
        assert(run_time::is_pow2(kAlignment));
        static const size_type kAlignMask = ~(kAlignment - 1);
        static const size_type kAlignPadding = kAlignment - 1;
        assert(kAlignment != 0);
        assert(kAlignMask != 0);
        assert((kAlignPadding != (size_type)(-1)) && ((unsigned)kAlignPadding < (unsigned)kAlignment));
        // entries_ addr must align to kAlignment byte.
        assert(run_time::is_pow2(kCapacity));
        size_type allocSize = kCapacity + kAlignPadding;
        value_type newEntries = new item_type[allocSize];
        if (newEntries) {
            allocEntries_ = newEntries;
            allocSize_ = allocSize;
            entries_ = reinterpret_cast<value_type>(reinterpret_cast<uintptr_t>(reinterpret_cast<char *>(newEntries) + kAlignPadding) & kAlignMask);
        }
    }

protected:
    inline void free_queue() {
        if (allocEntries_) {
            delete[] allocEntries_;
            allocEntries_ = nullptr;
#if defined(_DEBUG) || !defined(NDEBUG)
            allocSize_ = 0;
            entries_ = nullptr;
            head_ = tail_ = kInitCursor;
#endif
        }
    }

public:
    template <typename U>
    int push_front_impl(U && item) {
        mutex_.lock();

        if ((head_ - tail_) > kCapacity) {
            mutex_.unlock();
            return QUEUE_OP_EMPTY;
        }

        index_type next = head_ + 1;
        index_type index = next & kIndexMask;
        assert(entries_ != nullptr);
        entries_[index] = item;
        head_ = next;

        mutex_.unlock();
        return QUEUE_OP_SUCCESS;
    }

    template <typename U>
    int pop_back_impl(U & item) {
        mutex_.lock();

        if (head_ == tail_) {
            mutex_.unlock();
            return QUEUE_OP_EMPTY;
        }

        index_type next = tail_ + 1;
        index_type index = next & kIndexMask;
        assert(entries_ != nullptr);
        item = entries_[index];
        tail_ = next;

        mutex_.unlock();
        return QUEUE_OP_SUCCESS;
    }

public:
    void resize(size_type new_size) {
        // Do nothing!!
    }

}; // class Fixed_LockedRingQueue<T, ...>

template <typename T, typename MutexType = std::mutex,
          typename IndexType = uint64_t>
class LockedRingQueue :
    public LockedRingQueueAbstract<LockedRingQueue<T, MutexType, IndexType>,
                                   T, MutexType> {
public:
    typedef T               item_type;
    typedef T *             value_type;
    typedef MutexType       mutex_type;
    typedef IndexType       index_type;
    typedef std::size_t     size_type;

    static const index_type kInitCursor = (index_type)(-1);
    static const index_type kDefaultCapacity = (index_type)compile_time::round_to_pow2<kQueueDefaultCapacity>::value;
    static const size_type  kAlignment = compile_time::round_to_pow2<kCacheLineSize>::value;

    //template <typename U, typename T>
    //friend class LockedRingQueueAbstract;

private:
    index_type          head_;
    index_type          tail_;
    value_type          entries_;
    index_type          index_mask_;
    size_type           capacity_;
    value_type          allocEntries_;
    size_type           allocSize_;

public:
    mutable mutex_type  mutex_;

public:
    LockedRingQueue(size_type nCapacity = kDefaultCapacity)
        : head_(kInitCursor), tail_(kInitCursor), capacity_(nCapacity), index_mask_((index_type)(nCapacity - 1)),
          entries_(nullptr), allocEntries_(nullptr), allocSize_(0), mutex_() {
        capacity_ = internal_init(nCapacity);
        index_mask_ = (index_type)capacity_ - 1;
        assert(run_time::is_pow2(capacity_));
    }

    virtual ~LockedRingQueue() {
        mutex_.lock();
        free_queue_fast();
        mutex_.unlock();
    }

private:
    inline size_type internal_init(size_type nCapacity) {
        assert(run_time::is_pow2(kAlignment));
        static const size_type kAligntMask = ~(kAlignment - 1);
        static const size_type kAlignPadding = kAlignment - 1;
        assert(kAlignment != 0);
        assert(kAligntMask != 0);
        assert((kAlignPadding != (size_type)(-1)) && ((unsigned)kAlignPadding < (unsigned)kAlignment));
        // nCapacity must align to power of 2.
        size_type newCapacity = run_time::round_to_pow2<size_type>(nCapacity);
        assert(run_time::is_pow2(newCapacity));
        // entries_ addr must align to kAlignment byte.
        size_type allocSize = newCapacity + kAlignPadding;
        value_type newEntries = new item_type[allocSize];
        if (newEntries) {
            allocEntries_ = newEntries;
            allocSize_ = allocSize;
            entries_ = reinterpret_cast<value_type>(reinterpret_cast<uintptr_t>(newEntries + kAlignPadding) & kAligntMask);
            return newCapacity;
        }
        else {
            return capacity_;
        }
    }

protected:
    inline void init(size_type nCapacity) {
        // capacity_(capacity of entries) must align to power of 2.
        capacity_ = internal_init(nCapacity);
    }

    inline void free_queue_fast() {
        if (allocEntries_) {
            delete[] allocEntries_;
            allocEntries_ = nullptr;
#if defined(_DEBUG) || !defined(NDEBUG)
            allocSize_ = 0;
            entries_ = nullptr;
            capacity_ = 0;
            index_mask_ = 0;
            head_ = tail_ = kInitCursor;
#endif
        }
    }

    inline void free_queue() {
        if (allocEntries_) {
            delete[] allocEntries_;
            allocEntries_ = nullptr;
            allocSize_ = 0;
            entries_ = nullptr;
            capacity_ = 0;
            index_mask_ = 0;
            head_ = tail_ = kInitCursor;
        }
    }

public:
    template <typename U>
    int push_front_impl(U && item) {
        mutex_.lock();

        if ((head_ - tail_) > capacity_) {
            mutex_.unlock();
            return QUEUE_OP_EMPTY;
        }

        index_type next = head_ + 1;
        index_type index = next & index_mask_;
        assert(entries_ != nullptr);
        entries_[index] = item;
        head_ = next;

        mutex_.unlock();
        return QUEUE_OP_SUCCESS;
    }

    template <typename U>
    int pop_back_impl(U & item) {
        mutex_.lock();

        if (head_ == tail_) {
            mutex_.unlock();
            return QUEUE_OP_EMPTY;
        }

        index_type next = tail_ + 1;
        index_type index = next & index_mask_;
        assert(entries_ != nullptr);
        item = entries_[index];
        tail_ = next;

        mutex_.unlock();
        return QUEUE_OP_SUCCESS;
    }

public:
    void resize(size_type newCapacity) {
        mutex_.lock();
        free_queue();
        init(newCapacity);
        mutex_.unlock();
    }

    void create(size_type nCapacity) {
        resize(nCapacity);
    }

}; // class LockedRingQueue<T, ...>

#endif  /* LOCKEDRINGQUEUE_H */
