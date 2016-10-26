#ifndef QT_SIGNAL_IMPL_H
#define QT_SIGNAL_IMPL_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <stddef.h>

#include <vector>       // std::vector
#include <functional>   // std::function
#include <mutex>        // std::mutex, std::lock_guard
#include <memory>       // std::shared_ptr, std::weak_ptr
#include <algorithm>    // std::find_if()
#include <cassert>      // assert()
#include <thread>       // std::this_thread::yield()
#include <type_traits>  // std::is_same
#include <iterator>     // std::back_inserter
#include <map>          // std::multimap<T>

namespace jimi {

// implementational details
namespace detail {

    /// Interface for type erasure when disconnecting slots.
    struct disconnector
    {
	    virtual void operator() (std::size_t index) const = 0;
    };

    /// Deleter that doesn't delete.
    static inline void dont_delete(disconnector * p) {
        // Do nothing!!
    };

} // namespace detail

template <typename ThreadPolicy, typename T>
//template <typename ThreadPolicy, typename R, typename ...Args>
class signal_type;

class connection
{
private:
	/// Slot index of the connected slot.
    std::size_t key_;
	/// Weak pointer to the current disconnector functor.
	std::weak_ptr<detail::disconnector> weak_disconnector_;

public:
	connection() : key_(0) {
    }
	connection(connection const & src) = delete;
	connection & operator =(connection const & src) = delete;

	connection(connection && other) : key_(other.key_),
        weak_disconnector_(std::move(other.weak_disconnector_)) {
    }

	connection & operator =(connection && other) {
		key_ = other.key_;
        weak_disconnector_ = std::move(other.weak_disconnector_);
		return (*this);
	}

	bool connected() const {
		return (!weak_disconnector_.expired());
	}

    void connect(int key, std::shared_ptr<detail::disconnector> const & shared_disconnector) {
        //
    }

    // Implementation of the disconnect operation of the connection class
	void disconnect() {
		auto ptr = weak_disconnector_.lock();
		if (ptr != nullptr) {
			(*ptr)(key_);
		}
		weak_disconnector_.reset();
    }

private:
	template <typename ThreadPolicy, typename T>
    friend class signal_type;

	connection(std::size_t index, std::size_t key,
               std::shared_ptr<detail::disconnector> const & shared_disconnector) :
        key_(key), weak_disconnector_(shared_disconnector) {
    }
};

class scoped_connection
{
private:
    /// Underlying connection object
    connection connection_;

public:
    scoped_connection() = default;
    scoped_connection(scoped_connection const &) = delete;
    scoped_connection & operator =(scoped_connection const &) = delete;
    scoped_connection(scoped_connection && other) :
        connection_(std::move(other.connection_)) {
    }

    scoped_connection(connection && conn) :
        connection_(std::forward<connection>(conn)) {
    }

    ~scoped_connection() {
        disconnect();
    }

    scoped_connection & operator =(connection && conn) {
        reset(std::forward<connection>(conn));
        return (*this);
    }

    void reset(connection && conn = {}) {
        disconnect();
        connection_ = std::move(conn);
    }

    connection release() {
        connection conn = std::move(connection_);
        connection_ = connection{};
        return conn;
    }

    bool connected() const {
        return connection_.connected();
    }

    void connect() {
        //connection_.connect();
    }

    void disconnect() {
        connection_.disconnect();
    }
};

struct singel_thread_policy
{
	/// Dummy mutex type that doesn't do anything
	struct mutex_type{};

	/// Dummy lock type, that doesn't do any locking.
	struct mutex_lock_type
	{
		/// A lock type must be constructible from a
		/// mutex type from the same thread policy.
		explicit mutex_lock_type(mutex_type const &) {
		}
	};

	/// Dummy implementation of thread yielding, that
	/// doesn't do any actual yielding.
	static void yield_thread() {
	}
};

struct multi_thread_policy
{
	using mutex_type = std::mutex;
	using mutex_lock_type = std::lock_guard<mutex_type>;

	/// Function that yields the current thread, allowing
	/// the OS to reschedule.
	static void yield_thread() {
		std::this_thread::yield();
	}
};

template <typename ThreadPolicy, typename R, typename ...Args>
class signal_type<ThreadPolicy, R(Args...)>
{
public:
    /// Type that will be used to store the slots for this signal type.
    using slot_type = std::function<R(Args...)>;
    /// Type that is used for counting the slots connected to this signal.
    using size_type = typename std::vector<slot_type>::size_type;
    //
    using this_type = signal_type<ThreadPolicy, R(Args...)>;

private:
    /// Thread policy currently in use
    using thread_policy = ThreadPolicy;
    /// Type of mutex, provided by threading policy
    using mutex_type = typename thread_policy::mutex_type;
    /// Type of mutex lock, provided by threading policy
    using mutex_lock_type = typename thread_policy::mutex_lock_type;

    // Forward declaration
    struct disconnector;

    /// Mutex to syncronize access to the slot vector
    mutable mutex_type mutex_;
    /// Vector of all connected slots
    std::multimap<std::size_t, slot_type> slots_;
    /// Number of connected slots
    size_type slot_count_;
    /// Disconnector operation, used for executing disconnection in a
    /// type erased manner.
    disconnector disconnector_;
    /// Shared pointer to the disconnector. All connection objects has a
    /// weak pointer to this pointer for performing disconnections.
    std::shared_ptr<detail::disconnector> shared_disconnector_;

public:
    /// signals are not copy constructible
    signal_type(signal_type const &) = delete;
    /// signals are not copy assignable
    signal_type & operator = (signal_type const &) = delete;

    /// signals are default constructible
    signal_type() : slot_count_(0) {}

    // Destruct the signal object.
    ~signal_type() {
        // If we are unlucky, some of the connected slots
        // might be in the process of disconnecting from other threads.
        // If this happens, we are risking to destruct the disconnector
        // object managed by our shared pointer before they are done
        // disconnecting. This would be bad. To solve this problem, we
        // discard the shared pointer (that is pointing to the disconnector
        // object within our own instance), but keep a weak pointer to that
        // instance. We then stall the destruction until all other weak
        // pointers have released their "lock" (indicated by the fact that
        // we will get a nullptr when locking our weak pointer).
        std::weak_ptr<detail::disconnector> weak_disconnector{ shared_disconnector_ };
        shared_disconnector_.reset();
        while (weak_disconnector.lock() != nullptr) {
            // we just yield here, allowing the OS to reschedule. We do
            // this until all threads has released the disconnector object.
            thread_policy::yield_thread();
        }
    }

    size_type slot_count() const {
        return slot_count_;
    }

    bool is_empty() const {
        return (slot_count() == 0);
    }

    void operator () (std::size_t key, Args const & ...args) const {
        for (auto const & it : copy_slots()) {
            if (it.second && it.first == key) {
                it.second(args...);
            }
        }
    }

    template <typename T>
    connection connect(std::size_t key, T && slot) {
        mutex_lock_type lock{ mutex_ };
        slots_.emplace(key, std::forward<T>(slot));
        std::size_t index = slots_.size() - 1;
        if (shared_disconnector_ == nullptr) {
            disconnector_ = disconnector{ this };
            shared_disconnector_ = std::shared_ptr<detail::disconnector>{ &disconnector_, detail::dont_delete };
        }
        ++slot_count_;
        return connection{ index, key, shared_disconnector_ };
    }

    void disconnect(std::size_t key) {
        mutex_lock_type lock(mutex_);

#if 1
        // Because when std::multimap earse a item, the iterator will be invalid.
        // So we record the delete iterators on the first run.
        std::vector<typename std::multimap<std::size_t, slot_type>::iterator> del_iters;
        for (auto iter = slots_.begin(); iter != slots_.end(); ++iter) {
            if (iter->first == key) {
                del_iters.push_back(iter);
            }
        }

        // And then we delete the iterators on the second run.
        assert(del_iters.size() <= slot_count_);
        for (unsigned i = 0; i < del_iters.size(); ++i) {
            slots_.erase(del_iters[i]);
            slot_count_--;
            if (slot_count_ < 0)
                break;
        }
#else
        bool done = false;
        while (!done) {
            done = true;
            for (auto iter = slots_.cbegin(); iter != slots_.cend(); ++iter) {
                if (iter->first == key) {
                    // Because when std::multimap earse a item, the iterator will be invalid,
                    // so we must rescan from begin.
                    slots_.erase(iter);
                    slot_count_--;
                    done = false;
                    break;
                }
            }
        }
#endif
    }

private:
    //template <typename T, typename P, typename Q, typename Args...>
    //friend class signal_accumulator;

    struct disconnector : detail::disconnector
    {
        disconnector() : ptr_(nullptr) {}
        disconnector(this_type * ptr) : ptr_(ptr) {}

        void operator () (std::size_t key) const override {
            if (ptr_) {
                ptr_->disconnect(key);
            }
        }

        /// Pointer to the current signal.
        this_type * ptr_;
    };

    /// Retrieve a copy of the current slots
    ///
    /// It's useful and necessary to copy the slots so we don't need
    /// to hold the lock while calling the slots. If we hold the lock
    /// we prevent the called slots from modifying the slots vector.
    /// This simple "double buffering" will allow slots to disconnect
    /// themself or other slots and connect new slots.
    std::multimap<std::size_t, slot_type> copy_slots() const
    {
        mutex_lock_type lock{ mutex_ };
        return slots_;
    }
};

template <typename T>
using safe_signal_impl = signal_type<multi_thread_policy, T>;

template <typename T>
using unsafe_signal_impl = signal_type<singel_thread_policy, T>;

} // namespace jimi

#endif // !QT_SIGNAL_IMPL_H
