
#ifndef UTILS_COMMON_H
#define UTILS_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef _MSC_VER
#include <intrin.h>     // For _ReadWriteBarrier(), InterlockedCompareExchange().
#endif  // _MSC_VER
#include <emmintrin.h>  // For _mm_pause().

/// 分别定义push(推送)和pop(弹出)的线程数
#define PUSH_CNT                2
#define POP_CNT                 2

///
/// 在 Sequence 类中是否使用 seq_spinlock() 锁 ?
/// 对于Disruptor C++: 如果当 (PUSH_CNT + POP_CNT) 大于 CPU核心总数时, 把该值设为 1 可能更快.
///         但当 (PUSH_CNT + POP_CNT) 小于等于 CPU核心总数时, x64模式下把该值设为 0 可能更快.
#define USE_SEQUENCE_SPIN_LOCK      0

#if (PUSH_CNT <= 1) && (POP_CNT <= 1)
#undef  USE_SEQUENCE_SPIN_LOCK
#define USE_SEQUENCE_SPIN_LOCK      0
#endif

#define USE_64BIT_SEQUENCE      1       // 1: SequenceStd ==> SequenceBase<uint64_t>
                                        // 0: SequenceStd ==> SequenceBase<uint32_t>

#ifndef JIMI_CACHELINE_SIZE
#define JIMI_CACHELINE_SIZE     64      // This value must be equal power of 2.
#endif

#if (defined(__cplusplus) && (__cplusplus >= 201402L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900L))
// C++ 14 or vc2015, for std::put_time().
#elif (defined(__cplusplus) && (__cplusplus >= 201300L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900L))
// C++ 11 or vc2015, for alignas(N), noexcept.
#endif

#if (defined(__cplusplus) && (__cplusplus >= 201300L)) || (defined(_MSC_VER) && (_MSC_VER >= 1900L))
// C++ 11
#define JIMI_NOEXCEPT  noexcept
#else
#define JIMI_NOEXCEPT
#endif // (__cplusplus >= 201300L) || (_MSC_VER >= 1900L)

#define jimi_nbits2(n)          (((n) & 2) ? 1 : 0)
#define jimi_nbits4(n)          (((n) & 0x0000000CU) ? (2  +  jimi_nbits2((n) >> 2)) : ( jimi_nbits2(n)))
#define jimi_nbits8(n)          (((n) & 0x000000F0U) ? (4  +  jimi_nbits4((n) >> 4)) : ( jimi_nbits4(n)))
#define jimi_nbits16(n)         (((n) & 0x0000FF00U) ? (8  +  jimi_nbits8((n) >> 8)) : ( jimi_nbits8(n)))
#define jimi_nbits32(n)         (((n) & 0xFFFF0000U) ? (16 + jimi_nbits16((n) >>16)) : (jimi_nbits16(n)))
#define jimi_nbits_t(n)         (((n) == 0) ? 0 : (jimi_nbits32(n) + 1))

#ifndef JIMI_POPCONUT
#define JIMI_POPCONUT(N)        jimi_nbits_t(N)
#endif  /* JIMI_POPCONUT */

#define jimi_popcnt1(n)         ((n) - (((n) >> 1) & 0x55555555U))
#define jimi_popcnt2(n)         ((jimi_popcnt1(n) & 0x33333333U) + ((jimi_popcnt1(n) >> 2) & 0x33333333U))
#define jimi_popcnt3(n)         ((jimi_popcnt2(n) & 0x0F0F0F0FU) + ((jimi_popcnt2(n) >> 4) & 0x0F0F0F0FU))
#define jimi_popcnt4(n)         ((jimi_popcnt3(n) & 0x0000FFFFU) +  (jimi_popcnt3(n) >>16))
#define jimi_popcnt5(n)         ((jimi_popcnt4(n) & 0x000000FFU) +  (jimi_popcnt4(n) >> 8))

#ifndef JIMI_POPCONUT32
#define JIMI_POPCONUT32(N)      jimi_popcnt5(N)
#endif  /* JIMI_POPCONUT32 */

#ifndef jimi_mm_pause
#define jimi_mm_pause           _mm_pause
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
#endif // CACHE_ALIGN_PREFIX, CACHE_ALIGN_SUFFIX

#if defined(_MSC_VER) || defined(__INTER_COMPILER) || defined(__ICC)

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include "msvc/targetver.h"
#include <windows.h>
#include <intrin.h>

#define jimi_val_compare_and_swap32(destPtr, oldValue, newValue)        \
    (int32_t)(InterlockedCompareExchange((volatile LONG *)(destPtr),    \
                            (LONG)(newValue), (LONG)(oldValue)))

#define jimi_val_compare_and_swap32u(destPtr, oldValue, newValue)       \
    (uint32_t)(InterlockedCompareExchange((volatile LONG *)(destPtr),   \
                            (LONG)(newValue), (LONG)(oldValue)))

#define jimi_val_compare_and_swap64(destPtr, oldValue, newValue)           \
    (int64_t)(InterlockedCompareExchange64((volatile LONG64 *)(destPtr),   \
                            (LONG64)(newValue), (LONG64)(oldValue)))

#define jimi_val_compare_and_swap64u(destPtr, oldValue, newValue)          \
    (uint64_t)(InterlockedCompareExchange64((volatile LONG64 *)(destPtr),  \
                            (LONG64)(newValue), (LONG64)(oldValue)))

#define jimi_bool_compare_and_swap32(destPtr, oldValue, newValue)       \
    (InterlockedCompareExchange((volatile LONG *)(destPtr),             \
                            (LONG)(newValue), (LONG)(oldValue))         \
                                == (LONG)(oldValue))

#define jimi_bool_compare_and_swap64(destPtr, oldValue, newValue)       \
    (InterlockedCompareExchange64((volatile LONG64 *)(destPtr),         \
                            (LONG64)(newValue), (LONG64)(oldValue))     \
                                == (LONG64)(oldValue))

#define jimi_lock_test_and_set32(destPtr, newValue)                     \
    (int32_t)(InterlockedExchange((volatile LONG *)(destPtr), (LONG)(newValue)))

#define jimi_lock_test_and_set32u(destPtr, newValue)                    \
    (uint32_t)(InterlockedExchange((volatile LONG *)(destPtr), (LONG)(newValue)))

#define jimi_lock_test_and_set64(destPtr, newValue)                     \
    (int64_t)(InterlockedExchange64((volatile LONGLONG *)(destPtr),     \
                                    (LONGLONG)(newValue)))

#define jimi_lock_test_and_set64u(destPtr, newValue)                    \
    (uint64_t)(InterlockedExchange64((volatile LONGLONG *)(destPtr),    \
                                    (LONGLONG)(newValue)))

#define jimi_fetch_and_add32(destPtr, addValue)                         \
    (uint32_t)(InterlockedExchangeAdd((volatile LONG *)(destPtr), (LONG)(addValue)))

#define jimi_fetch_and_add64(destPtr, addValue)                         \
    (uint64_t)(InterlockedExchangeAdd64((volatile LONGLONG *)(destPtr), \
                                        (LONGLONG)(addValue)))

#elif defined(__GUNC__) || defined(__GUNG__) || defined(__clang__) \
   || defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) \
   || defined(__CYGWIN__) || defined(__MINGW32__)

#define jimi_val_compare_and_swap32(destPtr, oldValue, newValue)       \
    __sync_val_compare_and_swap((volatile int32_t *)(destPtr),         \
                            (int32_t)(oldValue), (int32_t)(newValue))

#define jimi_val_compare_and_swap32u(destPtr, oldValue, newValue)       \
    __sync_val_compare_and_swap((volatile uint32_t *)(destPtr),         \
                            (uint32_t)(oldValue), (uint32_t)(newValue))

#define jimi_val_compare_and_swap64(destPtr, oldValue, newValue)        \
    __sync_val_compare_and_swap((volatile int64_t *)(destPtr),          \
                            (int64_t)(oldValue), (int64_t)(newValue))

#define jimi_val_compare_and_swap64u(destPtr, oldValue, newValue)       \
    __sync_val_compare_and_swap((volatile uint64_t *)(destPtr),         \
                            (uint64_t)(oldValue), (uint64_t)(newValue))

#define jimi_val_compare_and_swap(destPtr, oldValue, newValue)          \
    __sync_val_compare_and_swap((destPtr), (oldValue), (newValue))

#define jimi_bool_compare_and_swap32(destPtr, oldValue, newValue)       \
    __sync_bool_compare_and_swap((volatile uint32_t *)(destPtr),        \
                            (uint32_t)(oldValue), (uint32_t)(newValue))

#define jimi_bool_compare_and_swap64(destPtr, oldValue, newValue)       \
    __sync_bool_compare_and_swap((volatile uint64_t *)(destPtr),        \
                            (uint64_t)(oldValue), (uint64_t)(newValue))

#define jimi_bool_compare_and_swap(destPtr, oldValue, newValue)         \
    __sync_bool_compare_and_swap((destPtr), (oldValue), (newValue))

#define jimi_lock_test_and_set32(destPtr, newValue)                     \
    __sync_lock_test_and_set((volatile int32_t *)(destPtr),             \
                             (int32_t)(newValue))

#define jimi_lock_test_and_set32u(destPtr, newValue)                    \
    __sync_lock_test_and_set((volatile uint32_t *)(destPtr),            \
                             (uint32_t)(newValue))

#define jimi_lock_test_and_set64(destPtr, newValue)                     \
    __sync_lock_test_and_set((volatile int64_t *)(destPtr),             \
                             (int64_t)(newValue))

#define jimi_lock_test_and_set64u(destPtr, newValue)                    \
    __sync_lock_test_and_set((volatile uint64_t *)(destPtr),            \
                             (uint64_t)(newValue))

#define jimi_fetch_and_add32(destPtr, addValue)                         \
    __sync_fetch_and_add((volatile uint32_t *)(destPtr),                \
                         (uint32_t)(addValue))

#define jimi_fetch_and_add64(destPtr, addValue)                         \
    __sync_fetch_and_add((volatile uint64_t *)(destPtr),                \
                         (uint64_t)(addValue))

#else

#error "Unknown os or compiler."

#endif /* defined(_MSC_VER) || defined(__INTER_COMPILER) */

enum queue_trait_value_t {
    kQueueDefaultCapacity = 1024,
    kCacheLineSize = 64
};

enum queue_op_state_t {
    QUEUE_OP_EMPTY = -2,
    QUEUE_OP_FAILURE = -1,
    QUEUE_OP_SUCCESS = 0
};

#endif  /* UTILS_COMMON_H */
