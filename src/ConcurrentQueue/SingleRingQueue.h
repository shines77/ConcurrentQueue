#pragma once

#include <memory>

#include "Sequence.h"
#include "PowerOf2.h"
#include "MinMax.h"

template <typename T,
          typename SequenceType = uint32_t,
          int32_t Capacity = 1024U>
class SingleRingQueue
{
public:
    typedef T                               item_type;
    typedef typename item_type::value_type  value_type;
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
    SingleRingQueue() : head_(0), tail_(0),
        entries_(new item_type[kCapacity],
        [](item_type * p) { if (p != nullptr) delete[] p; }) {
        init();
    }

    ~SingleRingQueue() {
    }

private:
    void init() noexcept {
        item_type * pNewData = this->entries_.get();
        if (pNewData != nullptr) {
            memset((void *)pNewData, 0, sizeof(item_type) * kCapacity);
        }
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
        this->entries_[head & (sequence_type)kMask] = entry;
        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);

        sequence_type next = head + 1;

        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);
        this->head_.order_set(next);
        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);

        return 0;
    }

    int push(T const & entry) {
        return push(entry);
    }

    int pop(T & entry) {
        sequence_type head = this->head_.order_get();
        sequence_type tail = this->tail_.order_get();
        if ((tail == head) || (tail > head && (head - tail) > kMask)) {
            return -1;
        }

        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);
        entry = this->entries_[tail & (sequence_type)kMask];
        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);

        sequence_type next = tail + 1;

        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);
        this->tail_.order_set(next);
        std::atomic_thread_fence(std::memory_order::memory_order_acq_rel);

        return 0;
    }
};
