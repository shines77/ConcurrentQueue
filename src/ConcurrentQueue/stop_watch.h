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

#ifndef __COMPILER_BARRIER
#if defined(_MSC_VER) || defined(__INTEL_COMPILER) || defined(__ICL)
#define __COMPILER_BARRIER()		_ReadWriteBarrier()
#else
#define __COMPILER_BARRIER()		asm volatile ("" : : : "memory")
#endif
#endif

using namespace std::chrono;

class StopWatch {
private:
    std::chrono::time_point<high_resolution_clock> start_time_;
    std::chrono::time_point<high_resolution_clock> stop_time_;
    double elapsed_time_;
	double total_elapsed_time_;
    bool running_;

    static std::chrono::time_point<high_resolution_clock> base_time_;

public:
    StopWatch() : elapsed_time_(0.0), total_elapsed_time_(0.0), running_(false) {
        start_time_ = std::chrono::high_resolution_clock::now();
    };
    ~StopWatch() {};

	void reset() {
        __COMPILER_BARRIER();
        elapsed_time_ = 0.0;
        total_elapsed_time_ = 0.0;
        start_time_ = std::chrono::high_resolution_clock::now();
        running_ = false;
        __COMPILER_BARRIER();
	}

    void restart() {
        __COMPILER_BARRIER();
        running_ = false;
        elapsed_time_ = 0.0;
        total_elapsed_time_ = 0.0;
        start_time_ = std::chrono::high_resolution_clock::now();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void start() {
        if (!running_) {
            elapsed_time_ = 0.0;
            start_time_ = std::chrono::high_resolution_clock::now();
            running_ = true;
        }
        __COMPILER_BARRIER();
    }

    void stop() {
		__COMPILER_BARRIER();
        if (running_) {
            stop_time_ = std::chrono::high_resolution_clock::now();
            running_ = false;
        }
    }

    void mark_start() {
        start_time_ = std::chrono::high_resolution_clock::now();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void mark_stop() {
        __COMPILER_BARRIER();
        stop_time_ = std::chrono::high_resolution_clock::now();
        running_ = false;
    }

    double getIntervalSecond() {
        __COMPILER_BARRIER();
        if (!running_) {
            std::chrono::duration<double> interval_time =
                std::chrono::duration_cast< std::chrono::duration<double> >(stop_time_ - start_time_);
            elapsed_time_ = interval_time.count();
        }
        return elapsed_time_;
    }

    double getIntervalMillisec() {
        return getIntervalSecond() * 1000.0;
    }

    void continues() {
        start();
    }

	void pause() {
        __COMPILER_BARRIER();
        stop();
        __COMPILER_BARRIER();
		double elapsed_time = getIntervalSecond();
        __COMPILER_BARRIER();
		total_elapsed_time_ += elapsed_time;
	}

    static double now() {
        __COMPILER_BARRIER();
        std::chrono::duration<double> _now = std::chrono::duration_cast< std::chrono::duration<double> >
                                            (std::chrono::high_resolution_clock::now() - base_time_);
        __COMPILER_BARRIER();
        return _now.count();
    }

    double peekElapsedSecond() {
        __COMPILER_BARRIER();
        std::chrono::time_point<high_resolution_clock> now_time = std::chrono::high_resolution_clock::now();
        __COMPILER_BARRIER();
        std::chrono::duration<double> interval_time =
            std::chrono::duration_cast< std::chrono::duration<double> >(now_time - start_time_);
        return interval_time.count();
    }

    double peekElapsedMillisec() {
        return peekElapsedSecond() * 1000.0;
    }

    double getElapsedSecond() {
        __COMPILER_BARRIER();
        stop();
        __COMPILER_BARRIER();
        return getIntervalSecond();
    }

    double getElapsedMillisec() {
        return getElapsedMillisec() * 1000.0;
    }

    double getTotalSecond() const {
        __COMPILER_BARRIER();
        return total_elapsed_time_;
    }

    double getTotalMillisec() const {
        __COMPILER_BARRIER();
        return total_elapsed_time_ * 1000.0;
    }
};

std::chrono::time_point<high_resolution_clock> StopWatch::base_time_ = std::chrono::high_resolution_clock::now();

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
class StopWatch_v2 {
private:
    size_t start_time_;
    size_t stop_time_;
    double elapsed_time_;
	double total_elapsed_time_;
    bool running_;

public:
    StopWatch_v2() : start_time_(0), stop_time_(0), elapsed_time_(0.0),
                     total_elapsed_time_(0.0), running_(false) {};
    ~StopWatch_v2() {};

	void reset() {
        __COMPILER_BARRIER();
        elapsed_time_ = 0.0;
        total_elapsed_time_ = 0.0;
        start_time_ = timeGetTime();
        running_ = false;
        __COMPILER_BARRIER();
	}

    void restart() {
        __COMPILER_BARRIER();
        running_ = false;
        elapsed_time_ = 0.0;
        total_elapsed_time_ = 0.0;
        start_time_ = timeGetTime();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void start() {
        if (!running_) {
            elapsed_time_ = 0.0;
            start_time_ = timeGetTime();
            running_ = true;
        }
		__COMPILER_BARRIER();
    }

    void stop() {
		__COMPILER_BARRIER();
        if (running_) {
            stop_time_ = timeGetTime();
            running_ = false;
        }
    }

    void mark_start() {
        start_time_ = timeGetTime();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void mark_stop() {
        __COMPILER_BARRIER();
        stop_time_ = timeGetTime();
        running_ = false;
    }

    double getIntervalSecond() {
        __COMPILER_BARRIER();
        if (!running_) {
            elapsed_time_ = (double)(stop_time_ - start_time_) / 1000.0;
        }
        return elapsed_time_;
    }

    double getIntervalMillisec() {
        return getIntervalSecond() * 1000.0;
    }

    void continues() {
        start();
    }

	void pause() {
        __COMPILER_BARRIER();
        stop();
        __COMPILER_BARRIER();
		double elapsed_time = getIntervalSecond();
        __COMPILER_BARRIER();
		total_elapsed_time_ += elapsed_time;
	}

    static double now() {
        __COMPILER_BARRIER();
        double _now = static_cast<double>(timeGetTime()) / 1000.0;
        __COMPILER_BARRIER();
        return _now;
    }

    double peekElapsedSecond() {
        __COMPILER_BARRIER();
        size_t now_time = timeGetTime();
        __COMPILER_BARRIER();
        double interval_time = (double)(now_time - start_time_) / 1000.0;
        return interval_time;
    }

    double peekElapsedMillisec() {
        return peekElapsedSecond() * 1000.0;
    }

    double getElapsedSecond() {
        __COMPILER_BARRIER();
        stop();
        __COMPILER_BARRIER();
        return getIntervalSecond();
    }

    double getElapsedMillisec() {
        return getElapsedSecond() * 1000.0;
    }

    double getTotalSecond() const {
        __COMPILER_BARRIER();
        return total_elapsed_time_;
    }

    double getTotalMillisec() const {
        __COMPILER_BARRIER();
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

template <typename T>
class StopWatchBase {
public:
    typedef T                                   impl_type;
    typedef typename impl_type::time_stamp_t    time_stamp_t;
    typedef typename impl_type::time_float_t    time_float_t;
    typedef typename impl_type::time_point_t    time_point_t;
    typedef typename impl_type::duration_type   duration_type;

private:
    time_point_t start_time_;
    time_point_t stop_time_;
    time_float_t elapsed_time_;
    time_float_t total_elapsed_time_;
    bool         running_;

    static time_point_t base_time_;

public:
    StopWatchBase() : elapsed_time_(static_cast<time_float_t>(0)),
        total_elapsed_time_(static_cast<time_float_t>(0)), running_(false) {
        start_time_ = impl_type::get_now();
    };
    ~StopWatchBase() {};

	void reset() {
        __COMPILER_BARRIER();
        elapsed_time_ = static_cast<time_float_t>(0);
        total_elapsed_time_ = static_cast<time_float_t>(0);
        start_time_ = impl_type::get_now();
        running_ = false;
        __COMPILER_BARRIER();
	}

    void restart() {
        __COMPILER_BARRIER();
        running_ = false;
        elapsed_time_ = static_cast<time_float_t>(0);
        total_elapsed_time_ = static_cast<time_float_t>(0);
        start_time_ = impl_type::get_now();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void start() {
        if (!running_) {
            elapsed_time_ = static_cast<time_float_t>(0);
            start_time_ = impl_type::get_now();
            running_ = true;
        }
        __COMPILER_BARRIER();
    }

    void stop() {
		__COMPILER_BARRIER();
        if (running_) {
            stop_time_ = impl_type::get_now();
            running_ = false;
        }
    }

    void mark_start() {
        start_time_ = impl_type::get_now();
        running_ = true;
        __COMPILER_BARRIER();
    }

    void mark_stop() {
        __COMPILER_BARRIER();
        stop_time_ = impl_type::get_now();
        running_ = false;
    }

    time_float_t getIntervalSecond() {
        __COMPILER_BARRIER();
        if (!running_) {
            elapsed_time_ = impl_type::get_interval_time(stop_time_, start_time_);
        }
        return elapsed_time_;
    }

    time_float_t getIntervalMillisec() {
        return getIntervalSecond() * static_cast<time_float_t>(1000.0);
    }

    void continues() {
        start();
    }

	void pause() {
        __COMPILER_BARRIER();
        stop();
        __COMPILER_BARRIER();
		double elapsed_time = getIntervalSecond();
        __COMPILER_BARRIER();
		total_elapsed_time_ += elapsed_time;
	}

    static time_stamp_t now() {
        __COMPILER_BARRIER();
        time_stamp_t _now = impl_type::now(impl_.get_now(), base_time_);
        __COMPILER_BARRIER();
        return _now;
    }

    time_float_t peekElapsedSecond() {
        __COMPILER_BARRIER();
        elapsed_time = impl_type::get_interval_time(impl_.get_now(), start_time_);
        __COMPILER_BARRIER();
        return elapsed_time;
    }

    time_float_t peekElapsedMillisec() {
        return peekElapsedSecond() * static_cast<time_float_t>(1000.0);
    }

    time_float_t getElapsedSecond() {
        __COMPILER_BARRIER();
        stop();
        __COMPILER_BARRIER();
        return getIntervalSecond();
    }

    time_float_t getElapsedMillisec() {
        return getElapsedMillisec() * static_cast<time_float_t>(1000.0);
    }

    time_float_t getTotalSecond() const {
        __COMPILER_BARRIER();
        return total_elapsed_time_;
    }

    time_float_t getTotalMillisec() const {
        __COMPILER_BARRIER();
        return total_elapsed_time_ * static_cast<time_float_t>(1000.0);
    }
};

template <typename TimeFloatType>
class StdStopWatchImpl {
public:
    typedef TimeFloatType                                   time_float_t;
    typedef double                                          time_stamp_t;
    typedef std::chrono::time_point<high_resolution_clock>  time_point_t;
    typedef std::chrono::duration<time_stamp_t>             duration_type;

public:
    StdStopWatchImpl() {}
    ~StdStopWatchImpl() {}

    static time_point_t get_now() {
        return std::chrono::high_resolution_clock::now();
    }

    static time_float_t get_interval_time(time_point_t now_time, time_point_t old_time) {
        duration_type interval_time = std::chrono::duration_cast<duration_type>(now_time - old_time);
        return interval_time.count();
    }

    static time_stamp_t now(time_point_t now_time, time_point_t base_time) {
        duration_type interval_time = std::chrono::duration_cast<duration_type>(now_time - base_time);
        return static_cast<time_stamp_t>(interval_time.count());
    }
};

typedef StopWatchBase<StdStopWatchImpl<double>> StdStopWatch;

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)

template <typename TimeFloatType>
class TickCountStopWatchImpl {
public:
    typedef TimeFloatType   time_float_t;
    typedef DWORD           time_stamp_t;
    typedef DWORD           time_point_t;
    typedef time_float_t    duration_type;

public:
    TickCountStopWatchImpl() {}
    ~TickCountStopWatchImpl() {}

    static time_point_t get_now() {
        return ::timeGetTime();
    }

    static time_float_t get_interval_time(time_point_t now_time, time_point_t old_time) {
        time_point_t interval_time = now_time - old_time;
        return (static_cast<time_float_t>(interval_time) / static_cast<time_float_t>(1000));
    }

    static time_stamp_t now(time_point_t now_time, time_point_t base_time) {
        return now_time;
    }
};

typedef StopWatchBase<TickCountStopWatchImpl<double>> TickCountStopWatch;

#endif // _WIN32

#undef __COMPILER_BARRIER
