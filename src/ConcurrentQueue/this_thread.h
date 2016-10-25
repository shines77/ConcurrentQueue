#pragma once

#include <stdint.h>
#include <stddef.h>

#include <atomic>

#include "common.h"
#include "sleep.h"
#include "stop_watch.h"

class this_thread_t
{
#if defined(USE_SEQUENCE_SPIN_LOCK) && (USE_SEQUENCE_SPIN_LOCK != 0)
    static const size_t kYieldThreshold = 20;
#else
    static const size_t kYieldThreshold = 8;
#endif
private:
    size_t loop_cnt_;
    size_t spin_cnt_;
    double timeout_;
    StopWatch_v2 sw_;

public:
    this_thread_t() : loop_cnt_(0), spin_cnt_(1), timeout_(0.0) {
        sw_.start();
    }
    ~this_thread_t() {}

    void reset() {
        loop_cnt_ = 0;
        spin_cnt_ = 1;
    }

    double getTimeout() const { return timeout_; }
    void setTimeout(double timeout) { timeout_ = timeout; }

    void yield() {
        // Need yiled() or sleep() a while.
        std::atomic_thread_fence(std::memory_order_acq_rel);
        if (loop_cnt_ >= kYieldThreshold) {
            ptrdiff_t yield_cnt = loop_cnt_ - kYieldThreshold;
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
            for (size_t pause_cnt = 0; pause_cnt < spin_cnt_; ++pause_cnt) {
                jimi_mm_pause();
            }
            if (spin_cnt_ < 8192)
                spin_cnt_ = spin_cnt_ + 1;
        }
        loop_cnt_++;
        if (loop_cnt_ >= 256) {
            loop_cnt_ = 0;
        }
    }

    bool yield_timeout() {
        // Need yiled() or sleep() a while.
        std::atomic_thread_fence(std::memory_order_acq_rel);
        if (loop_cnt_ >= kYieldThreshold) {
            ptrdiff_t yield_cnt = loop_cnt_ - kYieldThreshold;
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
            for (size_t pause_cnt = 0; pause_cnt < spin_cnt_; ++pause_cnt) {
                jimi_mm_pause();
            }
            if (spin_cnt_ < 8192)
                spin_cnt_ = spin_cnt_ + 1;
        }
        loop_cnt_++;
        if (loop_cnt_ >= 256) {
            loop_cnt_ = 0;
            if (sw_.peekElapsedMillisec() <= timeout_)
                return false;
            else
                return true;
        }
        return false;
    }
};
