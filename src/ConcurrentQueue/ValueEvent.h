#pragma once

#include <stdint.h>

template <typename T>
class ValueEventBase
{
protected:
    T value;

public:
    ValueEventBase() : value(0) {}
    ValueEventBase(T const & value_) : value(value_) {}
    ~ValueEventBase() {}

    ////////////////////////////////////////////////////////////////////////////
    // normal operation
    ////////////////////////////////////////////////////////////////////////////

    // Read data from event
    void read(ValueEventBase & event) const {
        event.value = this->value;
    }

    // Copy data from src
    void copy(ValueEventBase const & src) {
        value = src.value;
    }

    // Update data from event
    void update(ValueEventBase const & event) {
        this->value = event.value;
    }

    // Move the data reference only
    void move(ValueEventBase & event) {
        // Do nothing!
    }

    ////////////////////////////////////////////////////////////////////////////
    // volatile operation
    ////////////////////////////////////////////////////////////////////////////

    // Read data from event
    void read(volatile ValueEventBase & event) {
        event.value = this->value;
    }

    // Copy data from src
    void copy(volatile ValueEventBase const & src) {
        value = src.value;
    }

    // Update data from event
    void update(volatile ValueEventBase const & event) {
        this->value = event.value;
    }

    // Move the data reference only
    void move(volatile ValueEventBase & event) {
        // Do nothing!
    }
};

template <typename T>
class ValueEvent : public ValueEventBase<T>
{
public:
    ValueEvent() : ValueEventBase<T>(0) {}
    ValueEvent(T const & value_) : ValueEventBase<T>(value_) {}

    // Copy constructor
    ValueEvent(ValueEvent const & src) : ValueEventBase<T>(src.value) {}

    // Copy assignment operator
    ValueEvent & operator = (ValueEvent const & rhs) {
        this->value = rhs.value;
        return *this;
    }

    ~ValueEvent() {}

    T get() const {
        return value;
    }

    void set(T const & newValue) {
        value = newValue;
    }
};

template <>
class ValueEvent<uint64_t> : public ValueEventBase<uint64_t>
{
public:
    ValueEvent() : ValueEventBase<uint64_t>(0) {}
    ValueEvent(uint64_t value_) : ValueEventBase<uint64_t>(value_) {}

    // Copy constructor
    ValueEvent(ValueEvent const & src) : ValueEventBase<uint64_t>(src.value) {}

    // Copy assignment operator
    ValueEvent & operator = (ValueEvent const & rhs) {
        this->value = rhs.value;
        return *this;
    }

    ~ValueEvent() {}

    uint64_t get() const {
        return value;
    }

    void set(uint64_t newValue) {
        value = newValue;
    }
};

template <>
class ValueEvent<int64_t> : public ValueEventBase<int64_t>
{
public:
    ValueEvent() : ValueEventBase<int64_t>(0) {}
    ValueEvent(int64_t value_) : ValueEventBase<int64_t>(value_) {}

    // Copy constructor
    ValueEvent(ValueEvent const & src) : ValueEventBase<int64_t>(src.value) {}

    // Copy assignment operator
    ValueEvent & operator = (ValueEvent const & rhs) {
        this->value = rhs.value;
        return *this;
    }

    ~ValueEvent() {}

    int64_t get() const {
        return value;
    }

    void set(int64_t newValue) {
        value = newValue;
    }
};

template <>
class ValueEvent<uint32_t> : public ValueEventBase<uint32_t>
{
public:
    ValueEvent() : ValueEventBase<uint32_t>(0) {}
    ValueEvent(uint32_t value_) : ValueEventBase<uint32_t>(value_) {}

    // Copy constructor
    ValueEvent(ValueEvent const & src) : ValueEventBase<uint32_t>(src.value) {}

    // Copy assignment operator
    ValueEvent & operator = (ValueEvent const & rhs) {
        this->value = rhs.value;
        return *this;
    }

    ~ValueEvent() {}

    uint32_t get() const {
        return value;
    }

    void set(uint32_t newValue) {
        value = newValue;
    }
};

template <>
class ValueEvent<int32_t> : public ValueEventBase<int32_t>
{
public:
    ValueEvent() : ValueEventBase<int32_t>(0) {}
    ValueEvent(int32_t value_) : ValueEventBase<int32_t>(value_) {}

    // Copy constructor
    ValueEvent(ValueEvent const & src) : ValueEventBase<int32_t>(src.value) {}

    // Copy assignment operator
    ValueEvent & operator = (ValueEvent const & rhs) {
        this->value = rhs.value;
        return *this;
    }

    ~ValueEvent() {}

    int32_t get() const {
        return value;
    }

    void set(int32_t newValue) {
        value = newValue;
    }
};
