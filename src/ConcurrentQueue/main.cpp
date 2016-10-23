
#include <stdio.h>
#include <stdlib.h>

#include "Sequence.h"
#include "ValueEvent.h"
#include "SingleRingQueue.h"

int main(int argn, char * argv[])
{
    SingleRingQueue<ValueEvent<uint32_t>, uint32_t, 1024> queue;
    ValueEvent<uint32_t> msg;

    queue.init();
    msg.set(32);
    SequenceBase<uint64_t> sequence_u64;
    SequenceBase<int64_t>  sequence_i64;
    printf("Hello world!\n");
    printf("value = %d\n", msg.get());
    sequence_u64.set(1);
    printf("SequenceBase<uint64_t>::min() = %llu\n", sequence_u64.getMinValue());
    printf("SequenceBase<uint64_t>::max() = %llu\n", sequence_u64.getMaxValue());
    printf("SequenceBase<uint64_t>::value = %llu\n", sequence_u64.get());
    sequence_i64.set(1);
    printf("SequenceBase<int64_t>::min() = %lld\n", sequence_i64.getMinValue());
    printf("SequenceBase<int64_t>::max() = %lld\n", sequence_i64.getMaxValue());
    printf("SequenceBase<int64_t>::value = %llu\n", sequence_i64.get());
    printf("\n");
#if defined(NDEBUG)
    system("pause");
#endif
    return 0;
}
