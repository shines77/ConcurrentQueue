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
    typedef T                           item_type;
    typedef item_type                   value_type;
    typedef uint32_t                    size_type;
    typedef SequenceType                sequence_type;
    typedef uint32_t                    index_type;
    typedef SequenceBase<SequenceType>  Sequence;
    typedef T *                         pointer;
    typedef const T *                   const_pointer;
    typedef T &                         reference;
    typedef const T &                   const_reference;

public:
    static const bool       kIsAllocOnHeap  = true;
    static const size_type  kCapacity       = math::maxinum<size_type, compile_time::round_to_pow2<Capacity>::value, 2>::value;
    static const index_type kMask           = (index_type)(kCapacity - 1);

protected:
    Sequence head_;
    Sequence tail_;
    std::unique_ptr<value_type []> entries_;

public:
    SingleRingQueue() : head_(0), tail_(0), entries_() {
        init();
    }

    ~SingleRingQueue() {
    }

public:
    index_type mask() const      { return kMask;     };
    size_type capacity() const   { return kCapacity; };
    size_type length() const     { return sizes();   };

    size_type sizes() const {
        sequence_type head, tail;
        
        std::atomic_thread_fence(std::memory_order::memory_order_acquire);
        head = this->head_.get();
        tail = this->tail_.get();
        std::atomic_thread_fence(std::memory_order::memory_order_release);

        return (size_type)((head - tail) <= kMask) ? (head - tail) : (size_type)(-1);
    }

    void init() {
        std::unique_ptr<value_type []> newData(new value_type[kCapacity]);
        value_type * pNewData = newData.get();
        if (pNewData != nullptr) {
            memset((void *)pNewData, 0, sizeof(value_type) * kCapacity);
            this->entries_.reset(newData.get());
        }
    }

    int push(T const & entry) {}
    int pop(T & entry) {}
};
