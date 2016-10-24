#pragma once

#include <memory.h>
#include <memory>
#include <type_traits>

#include "common.h"
#include "Sequence.h"
#include "PowerOf2.h"
#include "MinMax.h"

template <typename T,
          typename SequenceType = uint32_t,
          int32_t Capacity = 1024U>
class FixedSingleRingQueue
{
public:
    typedef T                               item_type;
    typedef typename std::remove_pointer<T>::type   rp_item_type;
    typedef typename rp_item_type::value_type       value_type;
    typedef uint32_t                        size_type;
    typedef uint32_t                        index_type;
    typedef SequenceType                    sequence_type;
    typedef SequenceBase<SequenceType>      Sequence;
    typedef T *                             pointer;
    typedef const T *                       const_pointer;
    typedef T &                             reference;
    typedef const T &                       const_reference;

public:
    static const bool kIsAllocOnHeap = true;
    static const size_type kCapacity = math::maxinum<size_type, compile_time::round_to_pow2<Capacity>::value, 2>::value;
    static const index_type kMask    = (index_type)(kCapacity - 1);

protected:
    Sequence head_;
    Sequence tail_;
    std::unique_ptr<item_type [], void(*)(item_type *)> entries_;

public:
#if (defined(__cplusplus) && (__cplusplus >= 201300L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900L))
    FixedSingleRingQueue() : head_(0), tail_(0),
        entries_(new item_type[kCapacity],
        [](item_type * p) { if (p != nullptr) delete[] p; }) {
        init();
    }
#else
    FixedSingleRingQueue() : head_(0), tail_(0),
        entries_(new item_type[kCapacity], &FixedSingleRingQueue<T, SequenceType, Capacity>::entries_deleter) {
        init();
    }
#endif

    ~FixedSingleRingQueue() {
    }

private:
    void init() JIMI_NOEXCEPT {
        item_type * pNewData = this->entries_.get();
        if (pNewData != nullptr) {
            ::memset((void *)pNewData, 0, sizeof(item_type) * kCapacity);
        }
    }

    static void entries_deleter(item_type * p) {
        if (p != nullptr)
            delete[] p;
    }

public:
    index_type mask() const      { return kMask;     }
    size_type capacity() const   { return kCapacity; }
    size_type length() const     { return sizes();   }
    bool is_empty() const        { return (sizes() == 0); }
    void resize(size_t n)        { /* Do Nothing!! */ }

    size_type sizes() const {
        sequence_type head, tail;
        
        std::atomic_thread_fence(std::memory_order::memory_order_acquire);
        head = this->head_.get();
        tail = this->tail_.get();
        std::atomic_thread_fence(std::memory_order::memory_order_release);

        return (size_type)((head - tail) <= kMask) ? (head - tail) : (size_type)(-1);
    }

    template <typename U>
    int push(U && entry) {
        sequence_type head = this->head_.order_get();
        sequence_type tail = this->tail_.order_get();
        if ((head - tail) > kMask) {
            return -1;
        }

        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);
#if 1
        this->entries_[(index_type)head & kMask] = entry;
#else
        this->entries_[head & (sequence_type)kMask] = entry;
#endif
        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);

        sequence_type next = head + 1;

        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);
        this->head_.order_set(next);
        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);

        return 0;
    }

    int pop(T & entry) {
        sequence_type head = this->head_.order_get();
        sequence_type tail = this->tail_.order_get();
        if ((tail == head) || (tail > head && (head - tail) > kMask)) {
            return -1;
        }

        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);
#if 1
        entry = this->entries_[(index_type)tail & kMask];
#else
        entry = this->entries_[tail & (sequence_type)kMask];
#endif
        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);

        sequence_type next = tail + 1;

        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);
        this->tail_.order_set(next);
        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);

        return 0;
    }
};
