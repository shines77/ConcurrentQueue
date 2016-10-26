#ifndef QT_SIGNAL_H
#define QT_SIGNAL_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "QtSignalImpl.h"

namespace jimi {

template <typename ...Args>
class signal
{
public:
	static signal & get()
	{
		static signal instance;
		return instance;
	}

	template <typename T>
	connection connect(int key, T && slot)
	{
		return signal_.connect(key, std::forward<T>(slot));
	}

	void disconnect(int key)
	{
		signal_.disconnect(key);
	}

	void emit(int key, Args && ...args)
	{
		signal_(key, std::forward<Args>(args)...);
	}

private:
	signal() = default;
	signal(const signal &) = delete;
	signal(signal &&) = delete;

	safe_signal_impl<void(Args...)> signal_;
};

template <typename ...Args>
class unsafe_signal
{
public:
	static unsafe_signal & get()
	{
		static unsafe_signal instance;
		return instance;
	}

	template <typename T>
	connection connect(int key, T && slot)
	{
		return signal_.connect(key, std::forward<T>(slot));
	}

	void disconnect(int key)
	{
		signal_.disconnect(key);
	}

	void emit(int key, Args && ...args)
	{
		signal_(key, std::forward<Args>(args)...);
	}

private:
	unsafe_signal() = default;
	unsafe_signal(const unsafe_signal &) = delete;
	unsafe_signal(unsafe_signal &&) = delete;

	unsafe_signal_impl<void(Args...)> signal_;
};

} // namespace jimi

#endif // !QT_SIGNAL_H
