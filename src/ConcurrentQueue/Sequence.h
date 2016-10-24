#pragma once

#define NOMINMAX    // Make the default min(), max() marco invalid.

#include <stdint.h>
#include <atomic>
#include <memory>
#include <limits>

#define USE_64BIT_SEQUENCE      1       // 1: SequenceStd ==> SequenceBase<uint64_t>
                                        // 0: SequenceStd ==> SequenceBase<uint32_t>

#ifndef JIMI_CACHELINE_SIZE
#define JIMI_CACHELINE_SIZE     64      // This value must be equal power of 2.
#endif

#if defined(_MSC_VER) || defined(__GNUC__) || defined(__clang__)
#pragma pack(push)
#pragma pack(1)
#endif

#if (defined(__cplusplus) && (__cplusplus >= 201300L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900L))
// C++ 11
#define ALIGN_PREFIX(N)         alignas(N)
#define ALIGN_SUFFIX(N)

#define CACHE_ALIGN_PREFIX      alignas(JIMI_CACHELINE_SIZE)
#define CACHE_ALIGN_SUFFIX

#define PACKED_ALIGN_PREFIX(N)  alignas(N)
#define PACKED_ALIGN_SUFFIX(N)

#elif (defined(_MSC_VER) && (_MSC_VER >= 1400L)) || (defined(__INTEL_COMPILER) || defined(__ICC))
// msvc & intel c++
#define ALIGN_PREFIX(N)         __declspec(align(N))
#define ALIGN_SUFFIX(N)

#define CACHE_ALIGN_PREFIX      __declspec(align(JIMI_CACHELINE_SIZE))
#define CACHE_ALIGN_SUFFIX

#define PACKED_ALIGN_PREFIX(N)  __declspec(align(N))
#define PACKED_ALIGN_SUFFIX(N)

#elif (defined(__GUNC__) || defined(__GNUG__)) || defined(__clang__) || defined(__MINGW32__) || defined(__CYGWIN__) \
   || defined(__linux) || defined(__APPLE__) || defined(__FreeBSD__)
// gcc, g++, clang, MinGW, cygwin
#define ALIGN_PREFIX(N)         __attribute__((__aligned__((N))))
#define ALIGN_SUFFIX(N)

#define CACHE_ALIGN_PREFIX      __attribute__((__aligned__((JIMI_CACHELINE_SIZE))))
#define CACHE_ALIGN_SUFFIX

#define PACKED_ALIGN_PREFIX(N)
#define PACKED_ALIGN_SUFFIX(N)  __attribute__((packed, aligned(N)))

#else
// Not support
#define ALIGN_PREFIX(N)
#define ALIGN_SUFFIX(N)

#define CACHE_ALIGN_PREFIX
#define CACHE_ALIGN_SUFFIX

#define PACKED_ALIGN_PREFIX(N)
#define PACKED_ALIGN_SUFFIX(N)

#error "Warning: alignas(N) is not support, you can comment on this line."
#endif

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
    CACHE_ALIGN_PREFIX std::atomic<T>  value_ CACHE_ALIGN_SUFFIX;
    char            padding[(JIMI_CACHELINE_SIZE >= sizeof(std::atomic<T>))
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
        std::atomic_thread_fence(std::memory_order::memory_order_acquire);
        if (sizeof(T) > sizeof(uint32_t)) {
            *(uint64_t *)(&this->value_) = (uint64_t)initial_val;
        }
        else {
            *(uint32_t *)(&this->value_) = (uint32_t)initial_val;
        }
        std::atomic_thread_fence(std::memory_order::memory_order_release);
    }

    T getMinValue() const { return kMinSequenceValue; }
    T getMaxValue() const { return kMaxSequenceValue; }

    void setMinValue() { set(kMinSequenceValue); }
    void setMaxValue() { set(kMaxSequenceValue); }

    T get() const {
        return value_.load(std::memory_order::memory_order_relaxed);
    }

    void set(T value) {
        value_.store(value, std::memory_order::memory_order_relaxed);
    }

    T order_get() const {
        return value_.load(std::memory_order::memory_order_acquire);
    }

    void order_set(T value) {
        value_.store(value, std::memory_order::memory_order_release);
    }

    T explicit_get() const {
        return std::atomic_load_explicit(&value_, std::memory_order::memory_order_acquire);
    }

    void explicit_set(T value) {
        std::atomic_store_explicit(&value_, value, std::memory_order::memory_order_release);
    }
} CACHE_ALIGN_SUFFIX;

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
