
#ifndef UTILS_COMMON_H
#define UTILS_COMMON_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
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
