
#include <stdio.h>
#include <stdlib.h>

#include <cstdio>
#include <iostream>
#include <string>
#include <atomic>
#include <mutex>
#include <thread>
#include <queue>
#include <deque>
#include <type_traits>

#include "Sequence.h"
#include "ValueMessage.h"
#include "SingleRingQueue.h"
#include "LockedRingQueue.h"

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64) \
 || defined(_WINDOWS) || defined(__MINGW__) || defined(__MINGW32__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // WIN32_LEAN_AND_MEAN
#else
#include <pthread.h>
#endif

#define CXX11_DEFAULT_METHOD_DECLEAR    1
#define CXX11_DELETE_METHOD_DECLEAR     1

#if defined(CXX11_DEFAULT_METHOD_DECLEAR) && (CXX11_DEFAULT_METHOD_DECLEAR != 0)
#define JIMI_DEFAULT_METHOD     = default
#else
#define JIMI_DEFAULT_METHOD     {}
#endif

#if defined(CXX11_DELETE_METHOD_DECLEAR) && (CXX11_DELETE_METHOD_DECLEAR != 0)
#define JIMI_DELETE_METHOD      = delete
#else
#define JIMI_DELETE_METHOD      {}
#endif

#if defined(NDEBUG)
static const size_t kMaxMessageCount = (800 * 10000);
#else
static const size_t kMaxMessageCount = (50 * 10000);
#endif

typedef ValueMessage<uint64_t> Message;

namespace native {

template <typename T, bool HasLocked = false>
class scoped_lock {
private:
    typedef T mutex_type;
    mutex_type & mutex_;

    scoped_lock(mutex_type const &) JIMI_DELETE_METHOD;
    scoped_lock & operator = (mutex_type const &) JIMI_DELETE_METHOD;

public:
    explicit scoped_lock(mutex_type & mutex) : mutex_(mutex) {
        if (!HasLocked) {
            mutex_.lock();
        }
    }

    ~scoped_lock() {
        mutex_.unlock();
    }
};

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64) \
 || defined(_WINDOWS) || defined(__MINGW__) || defined(__MINGW32__)
class Mutex {
private:
    CRITICAL_SECTION  mutex_;

public:
    Mutex() {
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0403)
        ::InitializeCriticalSectionAndSpinCount(&mutex_, 1);
#else
        ::InitializeCriticalSection(&mutex_);
#endif
    }

    ~Mutex() {
        ::DeleteCriticalSection(&mutex_);
    }

    void lock() {
        ::EnterCriticalSection(&mutex_);
    }

    bool try_lock() {
#if defined(_WIN32_WINNT) && (_WIN32_WINNT > 0x0400)
        return (::TryEnterCriticalSection(&mutex_) != 0);
#else
        return false;
#endif
    }

    void unlock() {
        ::LeaveCriticalSection(&mutex_);
    }
};
#elif defined(__GNUC__) || defined(__linux__) || defined(__CYGWIN__) || defined(PTHREAD_H) || defined(_PTHREAD_H)
class Mutex {
private:
    pthread_mutex_t  mutex_;

public:
    Mutex() {
        pthread_mutex_init(&mutex_);
    }

    ~Mutex() {
        pthread_mutex_destroy(&mutex_);
    }

    void lock() {
        pthread_mutex_lock(&mutex_);
    }

    bool try_lock() {
        return (pthread_mutex_trylock(&mutex_) != 0);
    }

    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }
};
#else // Other OS
#error WTF
#endif // _WIN32

} // namespace native

template <typename T>
class QueueWrapper {
public:
    QueueWrapper() {}
    ~QueueWrapper() {}
};

template <typename T, typename MutexType>
class StdQueueWrapper : public QueueWrapper< StdQueueWrapper<T, MutexType> > {
public:
    typedef std::queue<T>   queue_type;
    typedef T               item_type;
    typedef MutexType       mutex_type;

private:
    mutex_type mutex_;
    queue_type queue_;

public:
    StdQueueWrapper() : mutex_(), queue_() {}
    ~StdQueueWrapper() {}

    bool empty() const {
        native::scoped_lock<mutex_type> lock(mutex_);
        bool is_empty = queue_.empty();
        return is_empty;
    }

    size_t sizes() const {
        native::scoped_lock<mutex_type> lock(mutex_);
        size_t size = queue_.sizes();
        return size;
    }

    void resize(size_t new_size) {
        // Do nothing!!
    }

    template <typename U>
    void push(U && item) {
        native::scoped_lock<mutex_type> lock(mutex_);
        queue_.push(std::forward<U>(item));
    }

    item_type & front() {
        native::scoped_lock<mutex_type> lock(mutex_);
        item_type & item = queue_.front();
        return item;
    }

    item_type & back() {
        native::scoped_lock<mutex_type> lock(mutex_);
        item_type & item = queue_.back();
        return item;
    }

    void pop() {
        native::scoped_lock<mutex_type> lock(mutex_);
        queue_.pop();
    }

    int pop(item_type & item) {
        native::scoped_lock<mutex_type> lock(mutex_);
        if (!queue_.empty()) {
            item_type & ret = queue_.back();
            item = std::move(ret);
            queue_.pop();
            return true;
        }
        else {
            return false;
        }
    }
};

template <typename T, typename MutexType>
class StdDequeueWrapper : public QueueWrapper< StdDequeueWrapper<T, MutexType> > {
public:
    typedef std::deque<T>   queue_type;
    typedef T               item_type;
    typedef MutexType       mutex_type;

private:
    mutex_type mutex_;
    queue_type queue_;

public:
    StdDequeueWrapper() : mutex_(), queue_() {}
    ~StdDequeueWrapper() {}

    bool empty() const {
        native::scoped_lock<mutex_type> lock(mutex_);
        bool is_empty = queue_.empty();
        return is_empty;
    }

    size_t sizes() const {
        native::scoped_lock<mutex_type> lock(mutex_);
        size_t size = queue_.sizes();
        return size;
    }

    void resize(size_t new_size) {
        native::scoped_lock<mutex_type> lock(mutex_);
        queue_.resize(new_size);
    }

    template <typename U>
    void push(U && item) {
        native::scoped_lock<mutex_type> lock(mutex_);
        queue_.push_back(std::forward<U>(item));
    }

    item_type & front() {
        native::scoped_lock<mutex_type> lock(mutex_);
        item_type & item = queue_.front();
        return item;
    }

    item_type & back() {
        native::scoped_lock<mutex_type> lock(mutex_);
        item_type & item = queue_.back();
        return item;
    }

    void pop() {
        native::scoped_lock<mutex_type> lock(mutex_);
        queue_.pop_front();
    }

    int pop(item_type & item) {
        native::scoped_lock<mutex_type> lock(mutex_);
        if (!queue_.empty()) {
            item_type & ret = queue_.back();
            item = std::move(ret);
            queue_.pop_front();
            return true;
        }
        else {
            return false;
        }
    }
};

template <typename T, typename MutexType, typename IndexType>
class LockedRingQueueWrapper
    : public QueueWrapper< LockedRingQueueWrapper<T, MutexType, IndexType> > {
public:
    typedef LockedRingQueue<T, MutexType, IndexType> queue_type;
    typedef T item_type;

private:
    queue_type queue_;

public:
    LockedRingQueueWrapper() : queue_() {}
    ~LockedRingQueueWrapper() {}

    bool empty() const {
        return queue_.is_empty();
    }

    size_t sizes() const {
        return queue_.sizes();
    }

    void resize(size_t new_size) {
        queue_.resize(new_size);
    }

    template <typename U>
    void push(U && item) {
        queue_.push_front(std::forward<U>(item));
    }

    item_type & back() {
        item_type item;
        if (queue_.pop_back(item) == QUEUE_OP_SUCCESS) {
            return std::move(item);
        }
        else {
            throw ("LockedRingQueue<T> is empty!");
        }
    }

    void pop() {
        item_type item;
        queue_.pop_back(item);
    }

    int pop(item_type & item) {
        return queue_.pop_back(item);
    }
};

template <typename T, typename MutexType, typename IndexType,
          size_t InitCapacity = kQueueDefaultCapacity>
class FixedLockedRingQueueWrapper
    : public QueueWrapper< FixedLockedRingQueueWrapper<T, MutexType, IndexType, InitCapacity> > {
public:
    typedef FixedLockedRingQueue<T, MutexType, IndexType, InitCapacity> queue_type;
    typedef T item_type;

private:
    queue_type queue_;

public:
    FixedLockedRingQueueWrapper() : queue_() {}
    ~FixedLockedRingQueueWrapper() {}

    bool empty() const {
        return queue_.is_empty();
    }

    size_t sizes() const {
        return queue_.sizes();
    }

    void resize(size_t new_size) {
        queue_.resize(new_size);
    }

    template <typename U>
    void push(U && item) {
        queue_.push_front(std::forward<U>(item));
    }

    item_type & back() {
        item_type item;
        if (queue_.pop_back(item) == QUEUE_OP_SUCCESS) {
            return std::move(item);
        }
        else {
            throw ("FixedLockedRingQueue<T> is empty!");
        }
    }

    void pop() {
        item_type item;
        queue_.pop_back(item);
    }

    int pop(item_type & item) {
        return queue_.pop_back(item);
    }
};

template <typename T, typename SequenceType,
          size_t InitCapacity = kQueueDefaultCapacity>
class FixedSingleRingQueueWrapper
    : public QueueWrapper< FixedSingleRingQueueWrapper<T, SequenceType, InitCapacity> > {
public:
    typedef SingleRingQueue<T, SequenceType, InitCapacity> queue_type;
    typedef T item_type;

private:
    queue_type queue_;

public:
    FixedSingleRingQueueWrapper() : queue_() {}
    ~FixedSingleRingQueueWrapper() {}

    bool empty() const {
        return queue_.is_empty();
    }

    size_t sizes() const {
        return queue_.sizes();
    }

    void resize(size_t new_size) {
        queue_.resize(new_size);
    }

    template <typename U>
    void push(U && item) {
        queue_.push(std::forward<U>(item));
    }

    item_type & back() {
        item_type item;
        if (queue_.pop(item) == QUEUE_OP_SUCCESS) {
            return std::move(item);
        }
        else {
            throw ("SingleRingQueue<T> is empty!");
        }
    }

    void pop() {
        item_type item;
        queue_.pop(item);
    }

    int pop(item_type & item) {
        return queue_.pop(item);
    }
};

template <typename QueueType, typename MessageType>
void producer_thread_proc(unsigned index, unsigned message_count, unsigned producers, QueueType * queue)
{
    typedef QueueType queue_type;
    typedef MessageType message_type;

    //printf("Producer Thread: thread_idx = %d, producers = %d.\n", index, producers);

    unsigned messages = message_count / producers;
    for (unsigned i = 0; i < messages; ++i) {
        message_type * msg = new message_type();
        queue->push(msg);
    }
}

template <typename QueueType, typename MessageType>
void consumer_thread_proc(unsigned index, unsigned message_count, unsigned consumers, QueueType * queue)
{
    typedef QueueType queue_type;
    typedef MessageType message_type;

    //printf("Consumer Thread: thread_idx = %d, consumers = %d.\n", index, consumers);

    unsigned messages = message_count / consumers;
    for (unsigned i = 0; i < messages; ++i) {
#if 0
        if (!queue->empty()) {
            message_type *& msg = queue->back();
            queue->pop();
        }
#else
        message_type * msg = nullptr;
        queue->pop(msg);
#endif
    }
}

template <>
void producer_thread_proc<FixedSingleRingQueueWrapper<Message, uint64_t, 4096>, Message>
    (unsigned index, unsigned message_count, unsigned producers, FixedSingleRingQueueWrapper<Message, uint64_t, 4096> * queue)
{
    typedef FixedSingleRingQueueWrapper<Message, uint64_t, 4096> queue_type;
    typedef Message message_type;

    //printf("Producer Thread: thread_idx = %d, producers = %d.\n", index, producers);

    unsigned messages = message_count / producers;
    for (unsigned i = 0; i < messages; ++i) {
        message_type * msg = new message_type();
        queue->push(*msg);
    }
}

template <>
void consumer_thread_proc<FixedSingleRingQueueWrapper<Message, uint64_t, 4096>, Message>
    (unsigned index, unsigned message_count, unsigned consumers, FixedSingleRingQueueWrapper<Message, uint64_t, 4096> * queue)
{
    typedef FixedSingleRingQueueWrapper<Message, uint64_t, 4096> queue_type;
    typedef Message message_type;

    //printf("Consumer Thread: thread_idx = %d, consumers = %d.\n", index, consumers);

    unsigned messages = message_count / consumers;
    for (unsigned i = 0; i < messages; ++i) {
        message_type msg;
        queue->pop(msg);
    }
}

template <>
void producer_thread_proc<FixedSingleRingQueueWrapper<Message, uint64_t, 16384>, Message>
    (unsigned index, unsigned message_count, unsigned producers, FixedSingleRingQueueWrapper<Message, uint64_t, 16384> * queue)
{
    typedef FixedSingleRingQueueWrapper<Message, uint64_t, 16384> queue_type;
    typedef Message message_type;

    //printf("Producer Thread: thread_idx = %d, producers = %d.\n", index, producers);

    unsigned messages = message_count / producers;
    for (unsigned i = 0; i < messages; ++i) {
        message_type * msg = new message_type();
        queue->push(*msg);
    }
}

template <>
void consumer_thread_proc<FixedSingleRingQueueWrapper<Message, uint64_t, 16384>, Message>
    (unsigned index, unsigned message_count, unsigned consumers, FixedSingleRingQueueWrapper<Message, uint64_t, 16384> * queue)
{
    typedef FixedSingleRingQueueWrapper<Message, uint64_t, 16384> queue_type;
    typedef Message message_type;

    //printf("Consumer Thread: thread_idx = %d, consumers = %d.\n", index, consumers);

    unsigned messages = message_count / consumers;
    for (unsigned i = 0; i < messages; ++i) {
        message_type msg;
        queue->pop(msg);
    }
}

template <typename QueueType, typename MessageType>
void run_test_threads(unsigned message_count, unsigned producers, unsigned consumers, size_t initCapacity)
{
    typedef QueueType queue_type;
    typedef MessageType message_type;

    queue_type queue;
    queue.resize(initCapacity);

    std::thread ** producer_threads = new std::thread *[producers];
    std::thread ** consumer_threads = new std::thread *[consumers];

    if (producer_threads) {
        for (unsigned i = 0; i < producers; ++i) {
            std::thread * thread = new std::thread(producer_thread_proc<queue_type, message_type>,
                i, message_count, producers, &queue);
            producer_threads[i] = thread;
        }
    }

    if (consumer_threads) {
        for (unsigned i = 0; i < consumers; ++i) {
            std::thread * thread = new std::thread(consumer_thread_proc<queue_type, message_type>,
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
    typedef QueueType queue_type;
    typedef MessageType message_type;

    printf("Test for: %s\n", typeid(queue_type).name());
    printf("\n");

    using namespace std::chrono;
    time_point<high_resolution_clock> startime = high_resolution_clock::now();

    run_test_threads<queue_type, message_type>(message_count, producers, consumers, initCapacity);

    time_point<high_resolution_clock> endtime = high_resolution_clock::now();
    duration<double> elapsed_time = duration_cast< duration<double> >(endtime - startime);

    printf("Elapsed time :  %-11.3f second(s)\n", elapsed_time.count());
    printf("Throughput   :  %-11.1f op/sec\n", (double)(message_count * 1) / elapsed_time.count());
    printf("\n");
}

template <size_t Capacity>
void run_test_queue(unsigned message_count, unsigned producers, unsigned consumers)
{
    printf("ConcurrentTest.\n");
    printf("\n");
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
        run_test_queue_impl<FixedSingleRingQueueWrapper<Message, uint64_t, Capacity>, Message>(message_count, producers, consumers, Capacity);
    }

    printf("-------------------------------------------------------------------------\n");

    //run_test_queue_impl<StdQueueWrapper<Message *, native::Mutex>, Message>(message_count, producers, consumers, Capacity);
    //run_test_queue_impl<StdDequeueWrapper<Message *, native::Mutex>, Message>(message_count, producers, consumers, Capacity);

    //run_test_queue_impl<LockedRingQueueWrapper<Message *, native::Mutex, uint64_t>, Message>(message_count, producers, consumers, Capacity);
    run_test_queue_impl<FixedLockedRingQueueWrapper<Message *, native::Mutex, uint64_t, Capacity>, Message>(message_count, producers, consumers, Capacity);

    printf("-------------------------------------------------------------------------\n");

    //run_test_queue_impl<StdQueueWrapper<Message *, std::mutex>, Message>(message_count, producers, consumers, Capacity);
    //run_test_queue_impl<StdDequeueWrapper<Message *, std::mutex>, Message>(message_count, producers, consumers, Capacity);

    //run_test_queue_impl<LockedRingQueueWrapper<Message *, std::mutex, uint64_t>, Message>(message_count, producers, consumers, Capacity);
    run_test_queue_impl<FixedLockedRingQueueWrapper<Message *, std::mutex, uint64_t, Capacity>, Message>(message_count, producers, consumers, Capacity);

    printf("-------------------------------------------------------------------------\n");
}

void run_unittest()
{
    SingleRingQueue<ValueMessage<uint32_t>, uint32_t, 1024> queue;
    ValueMessage<uint32_t> msg;

    msg.set(32);
    printf("value = %d\n", msg.get());

    queue.push(msg);
    queue.push(std::move(msg));
    queue.pop(msg);
    queue.pop(msg);

    printf("value = %d\n", msg.get());

    SequenceBase<uint64_t> sequence_u64;
    SequenceBase<int64_t>  sequence_i64;
    
    sequence_u64.set(1);
    printf("SequenceBase<uint64_t>::min() = %llu\n", sequence_u64.getMinValue());
    printf("SequenceBase<uint64_t>::max() = %llu\n", sequence_u64.getMaxValue());
    printf("SequenceBase<uint64_t>::value = %llu\n", sequence_u64.get());

    sequence_i64.set(1);
    printf("SequenceBase<int64_t>::min() = %lld\n", sequence_i64.getMinValue());
    printf("SequenceBase<int64_t>::max() = %lld\n", sequence_i64.getMaxValue());
    printf("SequenceBase<int64_t>::value = %llu\n", sequence_i64.get());
    printf("\n");
}

int main(int argn, char * argv[])
{
    //run_unittest();

    run_test_queue<4096>(8000000, 1, 1);
    run_test_queue<16384>(8000000, 1, 1);
    run_test_queue<4096>(8000000, 2, 2);
    run_test_queue<16384>(8000000, 2, 2);

#if defined(NDEBUG)
    system("pause");
#endif
    return 0;
}
