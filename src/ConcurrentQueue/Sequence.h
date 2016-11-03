#pragma once

#define NOMINMAX    // Make the default min(), max() marco invalid.

#include <stdint.h>
#include <atomic>
#include <memory>
#include <limits>

#include "common.h"

#define USE_CXX11_ATOMIC    1

#if defined(_MSC_VER) || defined(__GNUC__) || defined(__GNUG__) || defined(__clang__)
#pragma pack(push)
#pragma pack(1)
#endif // packed(push, 1)

template <typename T>
class CACHE_ALIGN_PREFIX SequenceBase
{
public:
    static const size_t kSizeOfInt32 = sizeof(uint32_t);
    static const size_t kSizeOfInt64 = sizeof(uint64_t);
    static const size_t kSizeOfValue = sizeof(T);

    static const T INITIAL_VALUE        = static_cast<T>(0);
    static const T INITIAL_CURSOR_VALUE = static_cast<T>(-1);

    static const T kMinSequenceValue;
    static const T kMaxSequenceValue;

protected:
    CACHE_ALIGN_PREFIX std::atomic<T> value_ CACHE_ALIGN_SUFFIX;
    char padding[(JIMI_CACHELINE_SIZE >= sizeof(std::atomic<T>))
                 ? (JIMI_CACHELINE_SIZE - sizeof(std::atomic<T>))
                 : ((sizeof(std::atomic<T>) - JIMI_CACHELINE_SIZE) & (JIMI_CACHELINE_SIZE - 1))];

public:
    SequenceBase() : value_(INITIAL_CURSOR_VALUE) {
        init(INITIAL_CURSOR_VALUE);
    }
    SequenceBase(T initial_val) : value_(initial_val) {
        init(initial_val);
    }
    ~SequenceBase() {}

    void init(T initial_val) {
        std::atomic_thread_fence(std::memory_order_acquire);
        if (sizeof(T) > sizeof(uint32_t)) {
            *(uint64_t *)(&this->value_) = (uint64_t)initial_val;
        }
        else {
            *(uint32_t *)(&this->value_) = (uint32_t)initial_val;
        }
        std::atomic_thread_fence(std::memory_order_release);
    }

    T getMinValue() const { return kMinSequenceValue; }
    T getMaxValue() const { return kMaxSequenceValue; }

    void setMinValue() { set(kMinSequenceValue); }
    void setMaxValue() { set(kMaxSequenceValue); }

    T get() const {
        return value_.load(std::memory_order_acquire);
    }

    void set(T value) {
        value_.store(value, std::memory_order_release);
    }

    T relax_get() const {
        return value_.load(std::memory_order_relaxed);
    }

    void relax_set(T value) {
        value_.store(value, std::memory_order_relaxed);
    }

    T local_get() const {
        return value_.load(std::memory_order_acquire);
    }

    void local_set(T value) {
        value_.store(value, std::memory_order_release);
    }

    T explicit_get() const {
        return value_.load(std::memory_order_seq_cst);
    }

    void explicit_set(T value) {
        value_.store(value, std::memory_order_seq_cst);
    }

#if !defined(USE_CXX11_ATOMIC) || (USE_CXX11_ATOMIC == 0)
    T compareAndSwap(T old_value, T new_value) {
        return new_value;
    }
#else
    T compareAndSwap(T old_value, T new_value) {
        T old_value_ = old_value;
        T orig_value = value_.load(std::memory_order_relaxed);
        if (!value_.compare_exchange_weak(old_value_, new_value,
            std::memory_order_release, std::memory_order_relaxed)) {
            // Some value have changed, maybe is old_value, maybe is value_.
            if (old_value_ != old_value)
                return old_value_;
            else
                return new_value;
        }
        return old_value;
    }
#endif // !USE_CXX11_ATOMIC

} CACHE_ALIGN_SUFFIX;

#if !defined(USE_CXX11_ATOMIC) || (USE_CXX11_ATOMIC == 0)

template <>
inline int32_t SequenceBase<int32_t>::compareAndSwap(int32_t old_value, int32_t new_value) 
{
    return jimi_val_compare_and_swap32(&(this->value_), old_value, new_value);
}

template <>
inline uint32_t SequenceBase<uint32_t>::compareAndSwap(uint32_t old_value, uint32_t new_value) 
{
    return jimi_val_compare_and_swap32u(&(this->value_), old_value, new_value);
}

template <>
inline int64_t SequenceBase<int64_t>::compareAndSwap(int64_t old_value, int64_t new_value) 
{
    return jimi_val_compare_and_swap64(&(this->value_), old_value, new_value);
}

template <>
inline uint64_t SequenceBase<uint64_t>::compareAndSwap(uint64_t old_value, uint64_t new_value) 
{
    return jimi_val_compare_and_swap64u(&(this->value_), old_value, new_value);
}

#endif // !USE_CXX11_ATOMIC

#if defined(_MSC_VER) || defined(__GNUC__) || defined(__clang__)
#pragma pack(pop)
#endif

/* Special define for MIN_SEQUENCE_VALUE and MAX_SEQUENCE_VALUE. */

//////////////////////////////////////////////////////////////////////////

template <typename T>
const T SequenceBase<T>::kMinSequenceValue = std::numeric_limits<T>::min();

template <typename T>
const T SequenceBase<T>::kMaxSequenceValue = std::numeric_limits<T>::max();

//////////////////////////////////////////////////////////////////////////

typedef SequenceBase<uint64_t>  SequenceU64;
typedef SequenceBase<uint32_t>  SequenceU32;
typedef SequenceBase<uint16_t>  SequenceU16;
typedef SequenceBase<uint8_t>   SequenceU8;

typedef SequenceBase<int64_t>   Sequence64;
typedef SequenceBase<int32_t>   Sequence32;
typedef SequenceBase<int16_t>   Sequence16;
typedef SequenceBase<int8_t>    Sequence8;

#if defined(USE_64BIT_SEQUENCE) && (USE_64BIT_SEQUENCE != 0)
typedef SequenceBase<uint64_t> SequenceStd;
#else
typedef SequenceBase<uint32_t> SequenceStd;
#endif

//typedef SequenceBase<int64_t> Sequence;
