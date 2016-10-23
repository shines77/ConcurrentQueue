#pragma once

#include <stdint.h>

template <typename T>
class ValueMessageBase
{
public:
    typedef T value_type;

protected:
    T value;

public:
    ValueMessageBase() : value(0) {}
    ValueMessageBase(T const & value_) : value(value_) {}
    ~ValueMessageBase() {}

    ////////////////////////////////////////////////////////////////////////////
    // normal operation
    ////////////////////////////////////////////////////////////////////////////

    // Read data from event
    void read(ValueMessageBase & event) const {
        event.value = this->value;
    }

    // Copy data from src
    void copy(ValueMessageBase const & src) {
        value = src.value;
    }

    // Update data from event
    void update(ValueMessageBase const & event) {
        this->value = event.value;
    }

    // Move the data reference only
    void move(ValueMessageBase & event) {
        // Do nothing!
    }

    ////////////////////////////////////////////////////////////////////////////
    // volatile operation
    ////////////////////////////////////////////////////////////////////////////

    // Read data from event
    void read(volatile ValueMessageBase & event) {
        event.value = this->value;
    }

    // Copy data from src
    void copy(volatile ValueMessageBase const & src) {
        value = src.value;
    }

    // Update data from event
    void update(volatile ValueMessageBase const & event) {
        this->value = event.value;
    }

    // Move the data reference only
    void move(volatile ValueMessageBase & event) {
        // Do nothing!
    }
};

template <typename T>
class ValueMessage : public ValueMessageBase<T>
{
public:
    typedef typename ValueMessageBase<T>::value_type value_type;

public:
    ValueMessage() : ValueMessageBase<T>(0) {}
    ValueMessage(T const & value_) : ValueMessageBase<T>(value_) {}

    // Copy constructor
    ValueMessage(ValueMessage const & src) : ValueMessageBase<T>(src.value) {}

    // Copy assignment operator
    ValueMessage & operator = (ValueMessage const & rhs) {
        this->value = rhs.value;
        return *this;
    }

    ~ValueMessage() {}

    T get() const {
        return value;
    }

    void set(T const & newValue) {
        value = newValue;
    }
};

template <>
class ValueMessage<uint64_t> : public ValueMessageBase<uint64_t>
{
public:
    typedef uint64_t value_type;

public:
    ValueMessage() : ValueMessageBase<uint64_t>(0) {}
    ValueMessage(uint64_t value_) : ValueMessageBase<uint64_t>(value_) {}

    // Copy constructor
    ValueMessage(ValueMessage const & src) : ValueMessageBase<uint64_t>(src.value) {}

    // Copy assignment operator
    ValueMessage & operator = (ValueMessage const & rhs) {
        this->value = rhs.value;
        return *this;
    }

    ~ValueMessage() {}

    uint64_t get() const {
        return value;
    }

    void set(uint64_t newValue) {
        value = newValue;
    }
};

template <>
class ValueMessage<int64_t> : public ValueMessageBase<int64_t>
{
public:
    typedef int64_t value_type;

public:
    ValueMessage() : ValueMessageBase<int64_t>(0) {}
    ValueMessage(int64_t value_) : ValueMessageBase<int64_t>(value_) {}

    // Copy constructor
    ValueMessage(ValueMessage const & src) : ValueMessageBase<int64_t>(src.value) {}

    // Copy assignment operator
    ValueMessage & operator = (ValueMessage const & rhs) {
        this->value = rhs.value;
        return *this;
    }

    ~ValueMessage() {}

    int64_t get() const {
        return value;
    }

    void set(int64_t newValue) {
        value = newValue;
    }
};

template <>
class ValueMessage<uint32_t> : public ValueMessageBase<uint32_t>
{
public:
    typedef uint32_t value_type;

public:
    ValueMessage() : ValueMessageBase<uint32_t>(0) {}
    ValueMessage(uint32_t value_) : ValueMessageBase<uint32_t>(value_) {}

    // Copy constructor
    ValueMessage(ValueMessage const & src) : ValueMessageBase<uint32_t>(src.value) {}

    // Copy assignment operator
    ValueMessage & operator = (ValueMessage const & rhs) {
        this->value = rhs.value;
        return *this;
    }

    ~ValueMessage() {}

    uint32_t get() const {
        return value;
    }

    void set(uint32_t newValue) {
        value = newValue;
    }
};

template <>
class ValueMessage<int32_t> : public ValueMessageBase<int32_t>
{
public:
    typedef int32_t value_type;

public:
    ValueMessage() : ValueMessageBase<int32_t>(0) {}
    ValueMessage(int32_t value_) : ValueMessageBase<int32_t>(value_) {}

    // Copy constructor
    ValueMessage(ValueMessage const & src) : ValueMessageBase<int32_t>(src.value) {}

    // Copy assignment operator
    ValueMessage & operator = (ValueMessage const & rhs) {
        this->value = rhs.value;
        return *this;
    }

    ~ValueMessage() {}

    int32_t get() const {
        return value;
    }

    void set(int32_t newValue) {
        value = newValue;
    }
};
