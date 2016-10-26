
#ifndef DISRUPTOR_RINGQUEUE_H
#define DISRUPTOR_RINGQUEUE_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <assert.h>

#include "common.h"
#include "sleep.h"
#include "Sequence.h"

#define JIMI_ALIGNED_TO(n, alignment)   \
    (((n) + ((alignment) - 1)) & ~(size_t)((alignment) - 1))

///////////////////////////////////////////////////////////////////////////////////////////
// class DisruptorRingQueue<T, SequenceType, Capacity, Producers, Consumers, NumThreads>
///////////////////////////////////////////////////////////////////////////////////////////

template <typename T, typename SequenceType = int64_t, uint32_t Capacity = 1024U,
          uint32_t Producers = 0, uint32_t Consumers = 0, uint32_t NumThreads = 0>
class DisruptorRingQueue
{
public:
    typedef T                           item_type;
    typedef item_type                   value_type;
#if 0
    typedef size_t                      size_type;    
#else
    typedef uint32_t                    size_type;
#endif
    typedef uint32_t                    flag_type;
    typedef uint32_t                    index_type;
    typedef SequenceType                sequence_type;
    typedef SequenceBase<SequenceType>  Sequence;

    
    typedef item_type *                 pointer;
    typedef const item_type *           const_pointer;
    typedef item_type &                 reference;
    typedef const item_type &           const_reference;

public:
    static const size_type  kCapacity       = math::maxinum<size_type, compile_time::round_to_pow2<Capacity>::value, 2>::value;;
    static const index_type kIndexMask      = (index_type)(kCapacity - 1);
    static const uint32_t   kIndexShift     = JIMI_POPCONUT32(kIndexMask);

    static const size_type  kBlockSize      = JIMI_ALIGNED_TO(sizeof(T), JIMI_CACHELINE_SIZE);

    static const size_type  kProducers      = Producers;
    static const size_type  kConsumers      = Consumers;
    static const size_type  kProducersAlloc = (Producers <= 1) ? 1 : ((Producers + 1) & ((size_type)(~1U)));
    static const size_type  kConsumersAlloc = (Consumers <= 1) ? 1 : ((Consumers + 1) & ((size_type)(~1U)));
    static const bool       kIsAllocOnHeap  = true;

    struct PopThreadStackData
    {
        Sequence *      pTailSequence;
        sequence_type   nextSequence;
        sequence_type   cachedAvailableSequence;
        bool            processedSequence;
    };

    typedef struct PopThreadStackData PopThreadStackData;

protected:
    Sequence        cursor, workSequence;
    Sequence        gatingSequences[kConsumersAlloc];
    Sequence        gatingSequenceCache;
    Sequence        gatingSequenceCaches[kProducersAlloc];

    item_type *     entries;
    flag_type *     availableBuffer;

public:
    DisruptorRingQueue(bool bFillQueue = true) : entries(nullptr), availableBuffer(nullptr) {
        init(bFillQueue);
    }

    ~DisruptorRingQueue() {
        // If the queue is allocated on system heap, release them.
        if (kIsAllocOnHeap) {
            if (this->availableBuffer != nullptr) {
                delete [] this->availableBuffer;
                this->availableBuffer = nullptr;
            }

            if (this->entries != nullptr) {
                delete [] this->entries;
                this->entries = nullptr;
            }
        }
    }

public:
    static sequence_type getMinimumSequence(const Sequence *sequences, const Sequence &workSequence,
                                            sequence_type current) {
        assert(sequences != nullptr);

#if 0
        sequence_type minSequence = sequences->get();
        for (size_type i = 1; i < kConsumers; ++i) {
            ++sequences;
            sequence_type seq = sequences->get();
#if 1
            minSequence = (seq < minSequence) ? seq : minSequence;
#else
            if (seq < minSequence)
                minSequence = seq;
#endif
        }

        sequence_type cachedWorkSequence;
        cachedWorkSequence = workSequence.get();
        if (cachedWorkSequence < minSequence)
            minSequence = cachedWorkSequence;

        if (current < minSequence)
            minSequence = current;
#else
        sequence_type minSequence = current;
        for (size_type i = 0; i < kConsumers; ++i) {
            sequence_type seq = sequences->get();
#if 1
            minSequence = (seq < minSequence) ? seq : minSequence;
#else
            if (seq < minSequence)
                minSequence = seq;
#endif
            ++sequences;
        }

        sequence_type cachedWorkSequence;
        cachedWorkSequence = workSequence.get();
        if (cachedWorkSequence < minSequence)
            minSequence = cachedWorkSequence;
#endif

        return minSequence;
    }

    void dump() {
        //ReleaseUtils::dump(&core, sizeof(core));
        //dump_memory(this, sizeof(*this), false, 16, 0, 0);
    }

    void dump_detail() {
        printf("---------------------------------------------------------\n");
        printf("DisruptorRingQueue: (head = %llu, tail = %llu)\n",
               this->cursor.get(), this->workSequence.get());
        printf("---------------------------------------------------------\n");

        printf("\n");
    }

    index_type mask() const     { return kIndexMask; }
    size_type  capacity() const { return kCapacity;  }
    size_type  length() const   { return sizes();    }
    bool       is_empty() const { return (sizes() == 0); }

    size_type  sizes() const {
        sequence_type head, tail;

        std::atomic_thread_fence(std::memory_order_acq_rel);
        head = this->cursor.get();
        tail = this->workSequence.get();
        std::atomic_thread_fence(std::memory_order_acq_rel);

        return (size_type)((head - tail) <= kIndexMask) ? (head - tail) : (size_type)(-1);
    }

    void init(bool bFillQueue = true) {
        this->cursor.set(Sequence::INITIAL_CURSOR_VALUE);
        this->workSequence.set(Sequence::INITIAL_CURSOR_VALUE);

        for (size_type i = 0; i < kConsumersAlloc; ++i) {
            this->gatingSequences[i].set(Sequence::INITIAL_CURSOR_VALUE);
        }
        init_queue(bFillQueue);

#if defined(_DEBUG) || !defined(NDEBUG)
  #if 0
        printf("kProducers      = %lu\n", kProducers);
        printf("kConsumers      = %lu\n", kConsumers);
        printf("kConsumersAlloc = %lu\n", kConsumersAlloc);
        printf("kCapacity       = %lu\n", kCapacity);
        printf("kIndexMask      = %lu\n", kIndexMask);
        printf("kIndexShift     = %lu\n", kIndexShift);
        printf("\n");
  #endif
#endif  /* _DEBUG */
    }

    void init_queue(bool bFillQueue = true) {
        item_type * newData = new T[kCapacity];
        if (newData != nullptr) {
            if (bFillQueue) {
                ::memset((void *)newData, 0, sizeof(item_type) * kCapacity);
            }
            std::atomic_thread_fence(std::memory_order_seq_cst);
            this->entries = newData;
        }

        flag_type * newBufferData = new flag_type[kCapacity];
        if (newBufferData != nullptr) {
            if (bFillQueue) {
                //::memset((void *)newBufferData, 0, sizeof(flag_type) * kCapacity);
                for (size_type i = 0; i < kCapacity; ++i) {
                    newBufferData[i] = (flag_type)(-1);
                }
            }
            std::atomic_thread_fence(std::memory_order_seq_cst);
            this->availableBuffer = newBufferData;
        }
    }

    void start() {
        sequence_type cursor = this->cursor.get();
        this->workSequence.set(cursor);
        this->gatingSequenceCache.set(cursor);

        size_type i;
        for (i = 0; i < kConsumersAlloc; ++i) {
            this->gatingSequences[i].set(cursor);
        }
        /*
        for (i = 0; i < kProducersAlloc; ++i) {
            this->gatingSequenceCaches[i].set(cursor);
        }
        //*/
    }

    void shutdown(int32_t timeOut = -1) {
        // TODO: do shutdown procedure
    }

    Sequence * getGatingSequences(int index) {
        if (index >= 0 && index < (int)kConsumersAlloc) {
            return &this->gatingSequences[index];
        }
        return nullptr;
    }

    void publish(sequence_type sequence) {
        std::atomic_thread_fence(std::memory_order_acq_rel);
        setAvailable(sequence);
    }

    bool isAvailable(sequence_type sequence) {
        index_type index = (index_type)((index_type)sequence &  kIndexMask);
        flag_type  flag  = (flag_type) (            sequence >> kIndexShift);

        flag_type  flagValue = this->availableBuffer[index];
        std::atomic_thread_fence(std::memory_order_acq_rel);
        return (flagValue == flag);
    }

    void setAvailable(sequence_type sequence) {
        index_type index = (index_type)((index_type)sequence &  kIndexMask);
        flag_type  flag  = (flag_type) (            sequence >> kIndexShift);

        if (kIsAllocOnHeap) {
            assert(this->availableBuffer != nullptr);
        }
        std::atomic_thread_fence(std::memory_order_acq_rel);
        this->availableBuffer[index] = flag;
    }

    sequence_type getHighestPublishedSequence(sequence_type lowerBound,
                                              sequence_type availableSequence) {
        for (sequence_type sequence = lowerBound; sequence <= availableSequence; ++sequence) {
            if (!isAvailable(sequence)) {
                return (sequence - 1);
            }
        }
        return availableSequence;
    }

    //template <typename U>
    //int push(U && entry) {
    int push(T const & entry) {
        sequence_type current, nextSequence;
        do {
            current = this->cursor.get();
            nextSequence = current + 1;

            //sequence_type wrapPoint = nextSequence - kCapacity;
            sequence_type wrapPoint = current - kIndexMask;
            sequence_type cachedGatingSequence = this->gatingSequenceCache.get();

            if (wrapPoint > cachedGatingSequence || cachedGatingSequence > current) {
                //if ((current - cachedGatingSequence) >= kIndexMask) {
                sequence_type gatingSequence = DisruptorRingQueue<T, SequenceType, Capacity, Producers, Consumers, NumThreads>
                    ::getMinimumSequence(this->gatingSequences, this->workSequence, current);
                //current = this->cursor.get();
                if (wrapPoint > gatingSequence) {
                    //if ((current - gatingSequence) >= kIndexMask) {
                        // Push() failed, maybe queue is full.
                        //this->gatingSequenceCaches[id].set(gatingSequence);
#if 0
                    for (int i = 2; i > 0; --i)
                        jimi_mm_pause();
                    continue;
#else
                    return QUEUE_OP_FAILURE;
#endif
                }

                this->gatingSequenceCache.set(gatingSequence);
            }
            else if (this->cursor.compareAndSwap(current, nextSequence) != current) {
                // Need yiled() or sleep() a while.
                //jimi_wsleep(0);
            }
            else {
                // Claim a sequence succeeds.
                break;
            }
        } while (true);

        this->entries[nextSequence & kIndexMask] = entry;
        //this->entries[nextSequence & kIndexMask].copy(entry);

        std::atomic_thread_fence(std::memory_order_acq_rel);

        publish(nextSequence);

        std::atomic_thread_fence(std::memory_order_acq_rel);
        return QUEUE_OP_SUCCESS;
    }

    int pop (T & entry, PopThreadStackData & data) {
        assert(data.pTailSequence != nullptr);

        sequence_type current, cursor, limit;
        while (true) {
            if (data.processedSequence) {
                data.processedSequence = false;
                do {
                    cursor = this->cursor.get();
                    limit = cursor - 1;
                    current = this->workSequence.get();
                    data.nextSequence = current + 1;
                    data.pTailSequence->set(current);
#if 0
                    if ((current == limit) || (current > limit && (limit - current) > kIndexMask)) {
#if 0
                        std::atomic_thread_fence(std::memory_order_acq_rel);
                        //processedSequence = true;
                        return QUEUE_OP_FAILURE;
#else
                        //jimi_wsleep(0);
#endif
                    }
#endif
                } while (this->workSequence.compareAndSwap(current, data.nextSequence) != current);
            }

            if (data.cachedAvailableSequence >= data.nextSequence) {
                //if ((cachedAvailableSequence - current) <= kIndexMask * 2) {
                //if ((cachedAvailableSequence - nextSequence) <= (kIndexMask + 1)) {
                    // Read the message data
                entry = this->entries[data.nextSequence & kIndexMask];

                std::atomic_thread_fence(std::memory_order_acq_rel);
                //data.tailSequence->set(data.nextSequence);
                data.processedSequence = true;

                std::atomic_thread_fence(std::memory_order_acq_rel);
                return QUEUE_OP_SUCCESS;
            }
            else {
                // Maybe queue is empty now.
                data.cachedAvailableSequence = waitFor(data.nextSequence);
                //data.tailSequence->set(cachedAvailableSequence);
                if (data.cachedAvailableSequence < data.nextSequence)
                    return QUEUE_OP_FAILURE;
            }
        }
    }

    sequence_type waitFor(sequence_type sequence) {
        sequence_type availableSequence;

#if defined(USE_SEQUENCE_SPIN_LOCK) && (USE_SEQUENCE_SPIN_LOCK != 0)
        static const uint32_t YIELD_THRESHOLD = 20;
#else
        static const uint32_t YIELD_THRESHOLD = 8;
#endif
        int32_t  pause_cnt;
        uint32_t loop_cnt, yield_cnt, spin_cnt;
        //StopWatch sw;
        //sw.start();

        loop_cnt = 0;
        spin_cnt = 1;
        while ((availableSequence = this->cursor.get()) < sequence) {
            // Need yiled() or sleep() a while.
            if (loop_cnt >= YIELD_THRESHOLD) {
                yield_cnt = loop_cnt - YIELD_THRESHOLD;
#if (defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)) \
 && !(defined(WIN32) || defined(_WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__))
                if ((yield_cnt & 31) == 31) {
                    jimi_sleep(1);
                }
                else {
                    jimi_yield();
                }
#else
                if ((yield_cnt & 63) == 63) {
                    jimi_sleep(1);
                }
                else if ((yield_cnt & 7) == 7) {
                    jimi_sleep(0);
                }
                else {
                    if (!jimi_yield()) {
                        jimi_sleep(0);
                    }
                }
#endif
            }
            else {
                for (pause_cnt = spin_cnt; pause_cnt > 0; --pause_cnt) {
                    jimi_mm_pause();
                }
                if (spin_cnt < 8192)
                    spin_cnt = spin_cnt + 2;
            }
            loop_cnt++;
            if (loop_cnt >= 256) {
                loop_cnt = 0;
                //if (sw.peekElapsedMillisec() > 1000.0)
                //    break;
            }
        }

        if (availableSequence < sequence)
            return availableSequence;

        return getHighestPublishedSequence(sequence, availableSequence);
    }
};

#endif  /* DISRUPTOR_RINGQUEUE_H */
