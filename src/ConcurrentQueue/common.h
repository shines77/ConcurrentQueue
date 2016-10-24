
#ifndef UTILS_COMMON_H
#define UTILS_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifdef _MSC_VER
#include <intrin.h>     // For _ReadWriteBarrier(), InterlockedCompareExchange().
#endif  // _MSC_VER
#include <emmintrin.h>  // For _mm_pause().

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

enum queue_trait_value_t {
    kQueueDefaultCapacity = 1024,
    kCacheLineSize = 64
};

enum queue_op_state_t {
    QUEUE_OP_FAILURE = -2,
    QUEUE_OP_EMPTY = -1,
    QUEUE_OP_SUCCESS = 0
};

#endif  /* UTILS_COMMON_H */
