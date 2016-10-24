#pragma once

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
    CRITICAL_SECTION mutex_;

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
