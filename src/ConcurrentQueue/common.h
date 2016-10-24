
#ifndef UTILS_COMMON_H
#define UTILS_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
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
