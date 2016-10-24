
#define NOMINMAX    // Make the default min(), max() marco invalid.

#include <stdio.h>
#include <stdlib.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <type_traits>

#include "common.h"
#include "ValueMessage.h"
#include "QueueWrapper.h"
#include "LockedRingQueue.h"
#include "SingleRingQueue.h"
#include "DisruptorRingQueue.h"
#include "stop_watch.h"

#if defined(NDEBUG)
static const size_t kMaxMessageCount = (800 * 10000);
#else
static const size_t kMaxMessageCount = (50 * 10000);
#endif

#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) || defined(_M_IA64) \
 || defined(__amd64__) || defined(__x86_64__)
typedef uint64_t index_type;
typedef int64_t sindex_type;
typedef ValueMessage<uint64_t> Message;
#else
typedef uint32_t index_type;
typedef int32_t sindex_type;
typedef ValueMessage<uint32_t> Message;
#endif

#if 0

template <typename QueueType, typename MessageType>
void producer_thread_proc(unsigned index, unsigned message_count, unsigned producers, QueueType * queue)
{
    //printf("Producer Thread: thread_idx = %d, producers = %d.\n", index, producers);

    unsigned messages = message_count / producers;
    for (unsigned i = 0; i < messages; ++i) {
        MessageType * msg = new MessageType();
        queue->push(msg);
    }
}

template <typename QueueType, typename MessageType>
void consumer_thread_proc(unsigned index, unsigned message_count, unsigned consumers, QueueType * queue)
{
    //printf("Consumer Thread: thread_idx = %d, consumers = %d.\n", index, consumers);

    unsigned messages = message_count / consumers;
    for (unsigned i = 0; i < messages; ++i) {
#if 0
        if (!queue->empty()) {
            message_type *& msg = queue->back();
            queue->pop();
        }
#else
        MessageType * msg = nullptr;
        queue->pop(msg);
#endif
    }
}

#else

template <typename QueueType, typename MessageType>
void producer_thread_proc(unsigned index, unsigned message_count, unsigned producers, QueueType * queue)
{
    //printf("Producer Thread: thread_idx = %u, producers = %u.\n", index, producers);

    unsigned messages = message_count / producers;
    unsigned counter = 0;
    uint32_t pause_cnt = 0;
    while (counter < messages) {
        MessageType msg(counter);
        while (queue->push(msg) != (int)QUEUE_OP_SUCCESS) {
            if ((pause_cnt & 0x07) != 0x07) {
                jimi_mm_pause();
            }
            else if ((pause_cnt & 0x0F) == 0x0F) {
                jimi_sleep(0);
            }
            else if ((pause_cnt & 0x1E) == 0x1E) {
                jimi_sleep(1);
            }
            pause_cnt++;
        }
        counter++;
    }
    //printf("Producer Thread: thread_idx = %u, consumers = %u, message_count = %u, messages = %u, counter = %u.\n",
    //        index, producers, message_count, messages, counter);
}

template <typename QueueType, typename MessageType>
void consumer_thread_proc(unsigned index, unsigned message_count, unsigned consumers, QueueType * queue)
{
    //printf("Consumer Thread: thread_idx = %u, consumers = %u.\n", index, consumers);

    unsigned messages = message_count / consumers;
    unsigned counter = 0;
    uint32_t pause_cnt = 0;
    while (counter < messages) {
        MessageType msg;
        while (queue->pop(msg, index) != (int)QUEUE_OP_SUCCESS) {
            if ((pause_cnt & 0x07) != 0x07) {
                jimi_mm_pause();
            }
            else if ((pause_cnt & 0x0F) == 0x0F) {
                jimi_sleep(0);
            }
            else if ((pause_cnt & 0x1E) == 0x1E) {
                jimi_sleep(1);
            }
            pause_cnt++;
        }
        counter++;
    }
    //printf("Consumer Thread: thread_idx = %u, consumers = %u, message_count = %u, messages = %u, counter = %u.\n",
    //        index, consumers, message_count, messages, counter);
}

#endif

#if 0

template <>
void producer_thread_proc<FixedSingleRingQueueWrapper<Message, index_type, 4096>, Message>
    (unsigned index, unsigned message_count, unsigned producers, FixedSingleRingQueueWrapper<Message, index_type, 4096> * queue)
{
    unsigned messages = message_count / producers;
    for (unsigned i = 0; i < messages; ++i) {
        Message * msg = new Message();
        queue->push(*msg);
    }
}

template <>
void consumer_thread_proc<FixedSingleRingQueueWrapper<Message, index_type, 4096>, Message>
    (unsigned index, unsigned message_count, unsigned consumers, FixedSingleRingQueueWrapper<Message, index_type, 4096> * queue)
{
    unsigned messages = message_count / consumers;
    for (unsigned i = 0; i < messages; ++i) {
        Message msg;
        queue->pop(msg);
    }
}

template <>
void producer_thread_proc<FixedSingleRingQueueWrapper<Message, index_type, 16384>, Message>
    (unsigned index, unsigned message_count, unsigned producers, FixedSingleRingQueueWrapper<Message, index_type, 16384> * queue)
{
    unsigned messages = message_count / producers;
    for (unsigned i = 0; i < messages; ++i) {
        Message * msg = new Message();
        queue->push(*msg);
    }
}

template <>
void consumer_thread_proc<FixedSingleRingQueueWrapper<Message, index_type, 16384>, Message>
    (unsigned index, unsigned message_count, unsigned consumers, FixedSingleRingQueueWrapper<Message, index_type, 16384> * queue)
{
    unsigned messages = message_count / consumers;
    for (unsigned i = 0; i < messages; ++i) {
        Message msg;
        queue->pop(msg);
    }
}

#endif

template <typename QueueType, typename MessageType>
void run_test_threads(unsigned message_count, unsigned producers, unsigned consumers, size_t initCapacity)
{
    QueueType queue;
    queue.resize(initCapacity);

    std::thread ** producer_threads = new std::thread *[producers];
    std::thread ** consumer_threads = new std::thread *[consumers];

    if (producer_threads) {
        for (unsigned i = 0; i < producers; ++i) {
            std::thread * thread = new std::thread(producer_thread_proc<QueueType, MessageType>,
                i, message_count, producers, &queue);
            producer_threads[i] = thread;
        }
    }

    if (consumer_threads) {
        for (unsigned i = 0; i < consumers; ++i) {
            std::thread * thread = new std::thread(consumer_thread_proc<QueueType, MessageType>,
                i, message_count, consumers, &queue);
            consumer_threads[i] = thread;
        }
    }

    if (producer_threads) {
        for (unsigned i = 0; i < producers; ++i) {
            producer_threads[i]->join();
        }
    }

    if (consumer_threads) {
        for (unsigned i = 0; i < consumers; ++i) {
            consumer_threads[i]->join();
        }
    }

    if (producer_threads) {
        for (unsigned i = 0; i < producers; ++i) {
            if (producer_threads[i])
                delete producer_threads[i];
        }
        delete[] producer_threads;
    }

    if (consumer_threads) {
        for (unsigned i = 0; i < consumers; ++i) {
            if (consumer_threads[i])
                delete consumer_threads[i];
        }
        delete[] consumer_threads;
    }
}

template <typename QueueType, typename MessageType>
void run_test_queue_impl(unsigned message_count, unsigned producers, unsigned consumers, size_t initCapacity)
{
    printf("Test for: %s\n", typeid(QueueType).name());
    printf("\n");

    StopWatch sw;
    sw.start();
    run_test_threads<QueueType, MessageType>(message_count, producers, consumers, initCapacity);
    sw.stop();
    
    printf("Elapsed time :  %-11.3f second(s)\n", sw.getElapsedSecond());
    printf("Throughput   :  %-11.1f op/sec\n", (double)(message_count) / sw.getElapsedSecond());
    printf("\n");
}

template <typename QueueType, typename MessageType>
void run_test_queue_impl2(unsigned message_count, unsigned producers, unsigned consumers, size_t initCapacity)
{
    printf("Test for: %s\n", typeid(QueueType).name());
    printf("\n");

    using namespace std::chrono;
    time_point<high_resolution_clock> startime = high_resolution_clock::now();

    run_test_threads<QueueType, MessageType>(message_count, producers, consumers, initCapacity);

    time_point<high_resolution_clock> endtime = high_resolution_clock::now();
    duration<double> elapsed_time = duration_cast< duration<double> >(endtime - startime);

    printf("Elapsed time :  %-11.3f second(s)\n", elapsed_time.count());
    printf("Throughput   :  %-11.1f op/sec\n", (double)(message_count) / elapsed_time.count());
    printf("\n");
}

template <size_t Capacity>
void run_test_queue(unsigned message_count, unsigned producers, unsigned consumers)
{
    printf("-------------------------------------------------------------------------\n");
    printf("Messages  = %u\n", message_count);
    printf("Producers = %u\n", producers);
    printf("Consumers = %u\n", consumers);
    printf("Capacity  = %u\n", (uint32_t)Capacity);
#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) || defined(_M_IA64) \
 || defined(__amd64__) || defined(__x86_64__)
    printf("x86_64    = true\n");
#else
    printf("x86_64    = false\n");
#endif
    printf("\n");

    if (producers == 1 && consumers == 1) {
        printf("-------------------------------------------------------------------------\n");
        run_test_queue_impl<FixedSingleRingQueueWrapper<Message, index_type, Capacity>, Message>(message_count, producers, consumers, Capacity);
    }

    printf("-------------------------------------------------------------------------\n");

    //run_test_queue_impl<StdQueueWrapper<Message, native::Mutex>, Message>(message_count, producers, consumers, Capacity);
    //run_test_queue_impl<StdDequeueWrapper<Message, native::Mutex>, Message>(message_count, producers, consumers, Capacity);

    //run_test_queue_impl<LockedRingQueueWrapper<Message, native::Mutex, index_type>, Message>(message_count, producers, consumers, Capacity);
    run_test_queue_impl<FixedLockedRingQueueWrapper<Message, native::Mutex, index_type, Capacity>, Message>(message_count, producers, consumers, Capacity);

    printf("-------------------------------------------------------------------------\n");

    //run_test_queue_impl<StdQueueWrapper<Message, std::mutex>, Message>(message_count, producers, consumers, Capacity);
    //run_test_queue_impl<StdDequeueWrapper<Message, std::mutex>, Message>(message_count, producers, consumers, Capacity);

    //run_test_queue_impl<LockedRingQueueWrapper<Message, std::mutex, index_type>, Message>(message_count, producers, consumers, Capacity);
    run_test_queue_impl<FixedLockedRingQueueWrapper<Message, std::mutex, index_type, Capacity>, Message>(message_count, producers, consumers, Capacity);

    /*
    if (producers == 1 && consumers == 1) {
        printf("-------------------------------------------------------------------------\n");
        run_test_queue_impl<DisruptorRingQueueWrapper<Message, sindex_type, Capacity, 1, 1, 2>, Message>(message_count, producers, consumers, Capacity);
    }
    else if (producers == 2 && consumers == 2) {
        printf("-------------------------------------------------------------------------\n");
        run_test_queue_impl<DisruptorRingQueueWrapper<Message, sindex_type, Capacity, 2, 2, 4>, Message>(message_count, producers, consumers, Capacity);
    }
    else if (producers == 4 && consumers == 4) {
        printf("-------------------------------------------------------------------------\n");
        run_test_queue_impl<DisruptorRingQueueWrapper<Message, sindex_type, Capacity, 4, 4, 8>, Message>(message_count, producers, consumers, Capacity);
    }
    else if (producers == 8 && consumers == 8) {
        printf("-------------------------------------------------------------------------\n");
        run_test_queue_impl<DisruptorRingQueueWrapper<Message, sindex_type, Capacity, 8, 8, 16>, Message>(message_count, producers, consumers, Capacity);
    }
    //*/

    //printf("-------------------------------------------------------------------------\n");
}

void run_unittest()
{
    FixedSingleRingQueue<ValueMessage<uint32_t>, uint32_t, 1024> queue;
    DisruptorRingQueue<ValueMessage<uint32_t>, uint32_t, 1024> disruptor;
    DisruptorRingQueue<ValueMessage<uint32_t>, uint32_t, 1024>::PopThreadStackData stackData;
    typedef DisruptorRingQueue<ValueMessage<uint32_t>, uint32_t, 1024>::Sequence sequence_type;

    sequence_type tailSequence;
    sequence_type * pTailSequence = disruptor.getGatingSequences(0);
    if (pTailSequence == NULL)
        pTailSequence = &tailSequence;
    tailSequence.order_set(sequence_type::INITIAL_CURSOR_VALUE);
    stackData.tailSequence = pTailSequence;
    stackData.nextSequence = stackData.tailSequence->order_get();
    stackData.cachedAvailableSequence = sequence_type::INITIAL_CURSOR_VALUE;
    stackData.processedSequence = true;

    ValueMessage<uint32_t> msg;

    msg.set(32);
    printf("value = %d\n", msg.get());

    queue.push(msg);
    queue.push(std::move(msg));
    queue.pop(msg);
    queue.pop(msg);

    disruptor.push(msg);
    disruptor.pop(msg, stackData);

    printf("value = %d\n", msg.get());

    SequenceBase<uint64_t> sequence_u64;
    SequenceBase<int64_t>  sequence_i64;
    
#if defined(_MSC_VER)
    sequence_u64.set(1);
    printf("SequenceBase<uint64_t>::min() = %llu\n", sequence_u64.getMinValue());
    printf("SequenceBase<uint64_t>::max() = %llu\n", sequence_u64.getMaxValue());
    printf("SequenceBase<uint64_t>::value = %llu\n", sequence_u64.get());

    sequence_i64.set(1);
    printf("SequenceBase<int64_t>::min() = %lld\n", sequence_i64.getMinValue());
    printf("SequenceBase<int64_t>::max() = %lld\n", sequence_i64.getMaxValue());
    printf("SequenceBase<int64_t>::value = %llu\n", sequence_i64.get());
    printf("\n");
#else
    sequence_u64.set(1);
    printf("SequenceBase<uint64_t>::min() = %lu\n", sequence_u64.getMinValue());
    printf("SequenceBase<uint64_t>::max() = %lu\n", sequence_u64.getMaxValue());
    printf("SequenceBase<uint64_t>::value = %lu\n", sequence_u64.get());

    sequence_i64.set(1);
    printf("SequenceBase<int64_t>::min() = %ld\n", sequence_i64.getMinValue());
    printf("SequenceBase<int64_t>::max() = %ld\n", sequence_i64.getMaxValue());
    printf("SequenceBase<int64_t>::value = %lu\n", sequence_i64.get());
    printf("\n");
#endif
}

int main(int argn, char * argv[])
{
#if !defined(NDEBUG)
    run_unittest();
#endif

    run_test_queue<4096>(kMaxMessageCount, 1, 1);
    run_test_queue<16384>(kMaxMessageCount, 1, 1);

    run_test_queue<4096>(kMaxMessageCount, 2, 2);
    run_test_queue<16384>(kMaxMessageCount, 2, 2);

    run_test_queue<4096>(kMaxMessageCount, 4, 4);
    run_test_queue<16384>(kMaxMessageCount, 4, 4);

    printf("-------------------------------------------------------------------------\n");

#if defined(_WIN32) && defined(NDEBUG)
    system("pause");
#endif
    return 0;
}
