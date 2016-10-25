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
    double interval_time_;
	double total_elapsed_time_;
    bool running_;

    static std::chrono::time_point<high_resolution_clock> base_time_;

public:
    StopWatch() : interval_time_(0.0), total_elapsed_time_(0.0), running_(false) {
        start_time_ = std::chrono::high_resolution_clock::now();
    };
    ~StopWatch() {};

	void reset() {
        COMPILER_BARRIER();
        interval_time_ = 0.0;
        total_elapsed_time_ = 0.0;
        start_time_ = std::chrono::high_resolution_clock::now();
        running_ = false;
        COMPILER_BARRIER();
	}

    void restart() {
        COMPILER_BARRIER();
        running_ = false;
        interval_time_ = 0.0;
        total_elapsed_time_ = 0.0;
        start_time_ = std::chrono::high_resolution_clock::now();
        running_ = true;
        COMPILER_BARRIER();
    }

    void start() {
        if (!running_) {
            interval_time_ = 0.0;
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

    double getIntervalSecond() {
        COMPILER_BARRIER();
        if (!running_) {
            std::chrono::duration<double> interval_time =
                std::chrono::duration_cast< std::chrono::duration<double> >(stop_time_ - start_time_);
            interval_time_ = interval_time.count();
        }
        return interval_time_;
    }

    double getIntervalMillisec() {
        return getIntervalSecond() * 1000.0;
    }

    void continues() {
        start();
    }

	void pause() {
        COMPILER_BARRIER();
        stop();
        COMPILER_BARRIER();
		double elapsed_time = getIntervalSecond();
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

    double peekElapsedSecond() {
        COMPILER_BARRIER();
        std::chrono::time_point<high_resolution_clock> now_time = std::chrono::high_resolution_clock::now();
        COMPILER_BARRIER();
        std::chrono::duration<double> interval_time =
            std::chrono::duration_cast< std::chrono::duration<double> >(now_time - start_time_);
        return interval_time.count();
    }

    double peekElapsedMillisec() {
        return peekElapsedSecond() * 1000.0;
    }

    double getElapsedSecond() {
        COMPILER_BARRIER();
        stop();
        COMPILER_BARRIER();
        return getIntervalSecond();
    }

    double getElapsedMillisec() {
        return getElapsedMillisec() * 1000.0;
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
        COMPILER_BARRIER();
        interval_time_ = 0.0;
        total_elapsed_time_ = 0.0;
        start_time_ = timeGetTime();
        running_ = false;
        COMPILER_BARRIER();
	}

    void restart() {
        COMPILER_BARRIER();
        running_ = false;
        interval_time_ = 0.0;
        total_elapsed_time_ = 0.0;
        start_time_ = timeGetTime();
        running_ = true;
        COMPILER_BARRIER();
    }

    void start() {
        if (!running_) {
            interval_time_ = 0.0;
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

    double getIntervalSecond() {
        COMPILER_BARRIER();
        if (!running_) {
            interval_time_ = (double)(stop_time_ - start_time_) / 1000.0;
        }
        return interval_time_;
    }

    double getIntervalMillisec() {
        return getIntervalSecond() * 1000.0;
    }

    void continues() {
        start();
    }

	void pause() {
        COMPILER_BARRIER();
        stop();
        COMPILER_BARRIER();
		double elapsed_time = getIntervalSecond();
        COMPILER_BARRIER();
		total_elapsed_time_ += elapsed_time;
	}

    static double now() {
        COMPILER_BARRIER();
        double _now = static_cast<double>(timeGetTime()) / 1000.0;
        COMPILER_BARRIER();
        return _now;
    }

    double peekElapsedSecond() {
        COMPILER_BARRIER();
        size_t now_time = timeGetTime();
        COMPILER_BARRIER();
        double interval_time = (double)(now_time - start_time_) / 1000.0;
        return interval_time;
    }

    double peekElapsedMillisec() {
        return peekElapsedSecond() * 1000.0;
    }

    double getElapsedSecond() {
        COMPILER_BARRIER();
        stop();
        COMPILER_BARRIER();
        return getIntervalSecond();
    }

    double getElapsedMillisec() {
        return getElapsedSecond() * 1000.0;
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

#else
typedef StopWatch StopWatch_v2;
#endif // _WIN32

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
typedef StopWatch stop_watch;
#else
typedef StopWatch stop_watch;
#endif // _WIN32

#undef COMPILER_BARRIRER
