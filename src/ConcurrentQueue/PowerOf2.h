
#ifndef UTILS_POWEROF2_H
#define UTILS_POWEROF2_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <stddef.h>

#include <type_traits>

namespace detail {

enum boolean_value_t {
    false_value = false,
    true_value = true
};

struct false_type {
    typedef false_type type;
    enum { value = false_value };
};

struct true_type {
    typedef true_type type;
    enum { value = true_value };
};

template<bool condition, class T, class U>
struct condition_if {
    typedef U type;
};

template <class T, class U>
struct condition_if<true, T, U> {
    typedef T type;
};

template<bool condition>
struct boolean_if {
    typedef typename condition_if<condition, true_type, false_type>::type type;
    enum { value = type::value };
};

} // namespace detail

namespace run_time {

template <typename SizeType>
inline bool is_pow2(SizeType n) {
    static_assert(std::is_integral<SizeType>::value,
        "Error: is_pow2(SizeType n) -- SizeType must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type UnsignedSizeType;
    UnsignedSizeType x = n;
    return ((x & (x - 1)) == 0);
}

template <typename SizeType>
inline SizeType verify_pow2(SizeType n) {
    static_assert(std::is_integral<SizeType>::value,
        "Error: verify_pow2(SizeType n) -- SizeType must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type UnsignedSizeType;
    UnsignedSizeType x = n;
    return (x & (x - 1));
}

template <typename SizeType>
inline SizeType round_to_pow2(SizeType n) {
    static_assert(std::is_integral<SizeType>::value,
        "Error: round_to_pow2(SizeType n) -- SizeType must be a integral type.");
    typedef typename std::make_unsigned<SizeType>::type UnsignedSizeType;
    UnsignedSizeType x;
    if (is_pow2(n)) {
        return n;
    }
    else {
        x = static_cast<UnsignedSizeType>(n - 1);
        x = x | (x >> 1);
        x = x | (x >> 2);
        x = x | (x >> 4);
        x = x | (x >> 8);
        x = x | (x >> 16);
        return static_cast<SizeType>(x + 1);
    }
}

template <>
inline int32_t round_to_pow2(int32_t n) {
    uint32_t x = static_cast<uint32_t>(n);
    if (is_pow2(x)) {
        return x;
    }
    else {
        if (n < 0) {
            x = static_cast<uint64_t>(-n);
        }
        x = x - 1;
        x = x | (x >> 1);
        x = x | (x >> 2);
        x = x | (x >> 4);
        x = x | (x >> 8);
        x = x | (x >> 16);
        if (n >= 0)
            return static_cast<int32_t>(x + 1);
        else
            return -static_cast<int32_t>(x + 1);
    }
}

template <>
inline uint64_t round_to_pow2(uint64_t n) {
    if (is_pow2(n)) {
        return n;
    }
    else {
        uint64_t x = n - 1;
        x = x | (x >> 1);
        x = x | (x >> 2);
        x = x | (x >> 4);
        x = x | (x >> 8);
        x = x | (x >> 16);
        x = x | (x >> 32);
        return (x + 1);
    }
}

template <>
inline int64_t round_to_pow2(int64_t n) {
    uint64_t x = static_cast<uint64_t>(n);
    if (is_pow2(x)) {
        return x;
    }
    else {
        if (n < 0) {
            x = static_cast<uint64_t>(-n);
        }
        x = x - 1;
        x = x | (x >> 1);
        x = x | (x >> 2);
        x = x | (x >> 4);
        x = x | (x >> 8);
        x = x | (x >> 16);
        x = x | (x >> 32);
        if (n >= 0)
            return static_cast<int64_t>(x + 1);
        else
            return -static_cast<int64_t>(x + 1);
    }
}

} // namespace run_time

namespace compile_time {

//
// is_pow_of_2 = (N && ((N & (N - 1)) == 0);  // here, N must is unsigned number
//
template <size_t N>
struct is_pow2 {
    enum { value = detail::boolean_if<((N & (N - 1)) == 0)>::value };
};

template <>
struct is_pow2<0> {
    enum { value = detail::true_value };
};

//
// round_to_pow2<N> and next_is_pow2<N>
//
template <size_t N>
struct next_is_pow2 {
    enum { value = detail::boolean_if<((N & (N + 1)) == 0)>::value };
};

template <bool IsPow2, size_t N>
struct round_to_pow2_impl {
    enum {
        isPow2 = is_pow2<N>::value,
        nextIsPow2 = next_is_pow2<N>::value
    };
    static const size_t value = (isPow2 == 1) ? N
        : round_to_pow2_impl<(nextIsPow2 == detail::true_value), N + 1>::value;
};

template <size_t N>
struct round_to_pow2_impl<true, N> {
    static const size_t value = N;
};

template <size_t N>
struct round_to_pow2 {
    static const size_t value = round_to_pow2_impl<(is_pow2<N>::value == detail::true_value), N>::value;
};

//
// round_down_to_pow2<N> and front_is_pow2<N>
//
template <size_t N>
struct front_is_pow2 {
    enum { value = detail::boolean_if<(((N - 1) & (N - 2)) == 0)>::value };
};

template <bool IsPow2, size_t N>
struct round_down_to_pow2_impl {
    enum {
        isPow2 = is_pow2<N>::value,
        frontIsPow2 = front_is_pow2<N>::value
    };
    static const size_t value = ((isPow2 == detail::true_value) ? N
        : round_down_to_pow2_impl<(frontIsPow2 == detail::true_value), N - 1>::value);
};

template <size_t N>
struct round_down_to_pow2_impl<true, N> {
    static const size_t value = N;
};

template <size_t N>
struct round_down_to_pow2 {
    static const size_t value = round_down_to_pow2_impl<(is_pow2<N>::value == detail::true_value), N>::value;
};

#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) || defined(_M_IA64) \
 || defined(__amd64__) || defined(__x86_64__)

template <>
struct round_to_pow2<18446744073709551615ULL> {
    enum { value = detail::false_value };
};

#else

template <>
struct round_to_pow2<4294967295UL> {
    enum { value = detail::false_value };
};

#endif // _WIN64 || _M_X64 || _M_AMD64

} // namespace compile_time

#endif  /* UTILS_POWEROF2_H */
