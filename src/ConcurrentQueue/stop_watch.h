#pragma once

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // WIN32_LEAN_AND_MEAN
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif // _WIN32
#include <chrono>
#include <mutex>

#ifndef COMPILER_BARRIER
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define COMPILER_BARRIER()		_ReadWriteBarrier()
#else
#define COMPILER_BARRIER()		asm volatile ("" : : : "memory")
#endif
#endif

using namespace std::chrono;

class StopWatch {
private:
    std::chrono::time_point<high_resolution_clock> start_time_;
    std::chrono::time_point<high_resolution_clock> stop_time_;
    std::chrono::duration<double> interval_time_;
	double total_elapsed_time_;
    bool running_;

    static std::chrono::time_point<high_resolution_clock> base_time_;

public:
    StopWatch() : interval_time_{0}, total_elapsed_time_(0.0), running_(false) {
        start_time_ = std::chrono::high_resolution_clock::now();
    };
    ~StopWatch() {};

	void reset() {
        start_time_ = std::chrono::high_resolution_clock::now();
        interval_time_ = std::chrono::duration_cast< std::chrono::duration<double> >(start_time_ - start_time_);
        total_elapsed_time_ = 0.0;
        running_ = false;
	}

    void restart() {
        reset();
        start();
    }

    void start() {
        if (!running_) {
            start_time_ = std::chrono::high_resolution_clock::now();
            running_ = true;
        }
        COMPILER_BARRIER();
    }

    void stop() {
		COMPILER_BARRIER();
        if (running_) {
            stop_time_ = std::chrono::high_resolution_clock::now();
            running_ = false;
        }
    }

    void mark_start() {
        start_time_ = std::chrono::high_resolution_clock::now();
        running_ = true;
        COMPILER_BARRIER();
    }

    void mark_stop() {
        COMPILER_BARRIER();
        stop_time_ = std::chrono::high_resolution_clock::now();
        running_ = false;
    }

	void again() {
        stop();
		double elapsed_time = getIntervalTime();
        COMPILER_BARRIER();
		total_elapsed_time_ += elapsed_time;
	}

    static double now() {
        COMPILER_BARRIER();
        std::chrono::duration<double> _now = std::chrono::duration_cast< std::chrono::duration<double> >
                                            (std::chrono::high_resolution_clock::now() - base_time_);
        COMPILER_BARRIER();
        return _now.count();
    }
   
    double getIntervalTime() {
        COMPILER_BARRIER();
        interval_time_ = std::chrono::duration_cast< std::chrono::duration<double> >(stop_time_ - start_time_);
        return interval_time_.count();
    }

    double getIntervalMillisec() {
        return getIntervalTime() * 1000.0;
    }

    double getSecond() {
        stop();
        COMPILER_BARRIER();
        return getIntervalTime();
    }

    double getMillisec() {
        stop();
        COMPILER_BARRIER();
        return getIntervalTime() * 1000.0;
    }

    double getElapsedTime() {
        return getSecond();
    }

    double getElapsedSecond() {
        return getSecond();
    }

    double getElapsedMillisec() {
        return getMillisec();
    }

    double getTotalSecond() const {
        COMPILER_BARRIER();
        return total_elapsed_time_;
    }

    double getTotalMillisec() const {
        COMPILER_BARRIER();
        return total_elapsed_time_ * 1000.0;
    }
};

std::chrono::time_point<high_resolution_clock> StopWatch::base_time_ = std::chrono::high_resolution_clock::now();

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
class StopWatch_v2 {
private:
    size_t start_time_;
    size_t stop_time_;
    double interval_time_;
	double total_elapsed_time_;
    bool running_;

public:
    StopWatch_v2() : start_time_(0), stop_time_(0), interval_time_(0.0),
                     total_elapsed_time_(0.0), running_(false) {};
    ~StopWatch_v2() {};

	void reset() {
        start_time_ = timeGetTime();
        interval_time_ = 0.0;
        total_elapsed_time_ = 0.0;
        running_ = false;
	}

    void restart() {
        reset();
        start();
    }

    void start() {
        if (!running_) {
            start_time_ = timeGetTime();
            running_ = true;
        }
		COMPILER_BARRIER();
    }

    void stop() {
		COMPILER_BARRIER();
        if (running_) {
            stop_time_ = timeGetTime();
            running_ = false;
        }
    }

    void mark_start() {
        start_time_ = timeGetTime();
        running_ = true;
        COMPILER_BARRIER();
    }

    void mark_stop() {
        COMPILER_BARRIER();
        stop_time_ = timeGetTime();
        running_ = false;
    }

	void again() {
        stop();
		double elapsed_time = getIntervalTime();
        COMPILER_BARRIER();
		total_elapsed_time_ += elapsed_time;
	}

    static double now() {
        COMPILER_BARRIER();
        double _now = static_cast<double>(timeGetTime()) / 1000.0;
        COMPILER_BARRIER();
        return _now;
    }
    
    double getIntervalTime() {
        COMPILER_BARRIER();
        interval_time_ = (double)(stop_time_ - start_time_) / 1000.0;
        return interval_time_;
    }

    double getIntervalMillisec() {
        return getIntervalTime() * 1000.0;
    }

    double getSecond() {
        stop();
        COMPILER_BARRIER();
        return getIntervalTime();
    }

    double getMillisec() {
        stop();
        COMPILER_BARRIER();
        return getIntervalTime() * 1000.0;
    }

    double getElapsedTime() {
        return getSecond();
    }

    double getElapsedSecond() {
        return getSecond();
    }

    double getElapsedMillisec() {
        return getMillisec();
    }

    double getTotalSecond() const {
        COMPILER_BARRIER();
        return total_elapsed_time_;
    }

    double getTotalMillisec() const {
        COMPILER_BARRIER();
        return total_elapsed_time_ * 1000.0;
    }
};
#endif // _WIN32

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
typedef StopWatch stop_watch;
#else
typedef StopWatch stop_watch;
#endif // _WIN32

#undef COMPILER_BARRIRER
