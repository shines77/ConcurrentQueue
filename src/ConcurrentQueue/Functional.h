#ifndef QT_FUNCTIONAL_H
#define QT_FUNCTIONAL_H

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <functional>
#include <tuple>
#include <array>
#include <utility>
#include <type_traits>

#if 0
namespace std {

template <class _Ret, class _Fx, class T, class ..._Types>
//class _Binder : public std::_Binder<_Ret, _Fx, std::tuple<std::decay<_Types>::type...> >
class _Binder : public std::_Binder<_Ret, _Fx, _Types...>
{
    _Binder() {}
    ~_Binder() {}
};

} // namespace std
#endif

namespace jimi {

// Convert array into a tuple
template <typename T, std::size_t N, std::size_t ...I>
decltype(auto) make_args_tuple_impl(std::array<T, N> & arr, std::index_sequence<I...>)
{
    //static constexpr std::size_t M = I + 1;
    const std::_Ph<1> * arg = nullptr;
    //arr[I]... = (T)(void *)arg;
    return std::make_tuple(arr[I]...);
}

template <typename T, std::size_t N, typename Indices = std::make_index_sequence<N>>
decltype(auto) make_args_tuple(std::array<T, N> & arr)
{
    return make_args_tuple_impl(arr, Indices());
}

/*
template <std::size_t ...I, std::size_t N, typename T, typename ...Args>
struct make_args_list_impl2 : make_args_list_impl2<(I - 1)..., N, Args...>
{
};

template <std::size_t ...I, std::size_t N, typename ...Args>
struct make_args_list_impl : make_args_list_impl2<(I - 1)..., N, Args...>
{
};
//*/

template <typename T, std::size_t N, std::size_t ...I>
struct make_args_list
{
    //typedef typename make_args_list_impl<I..., N, Args...>::type type;
    typedef typename std::_Ph<std::index_sequence<I...>::value>::type type;
};

/*
template <std::size_t N, typename T>
struct make_args_list_impl2<0, N, T>
{
    typedef const std::_Ph<1> & type;
};

template <std::size_t N, typename T>
struct make_args_list_impl2<1, N, T>
{
    typedef const std::_Ph<2> & type;
};

template <std::size_t N, typename T>
struct make_args_list_impl2<2, N, T>
{
    typedef const std::_Ph<3> & type;
};
//*/

template <typename Func, typename Ret, typename ...Args>
class binder {
    //
};

template <typename Func, typename Ret, typename ...Args>
class function: public binder<Func, Ret, Args...>
{
private:

};

class unforced;

template <typename Func, typename ...Args>
inline binder<Func, unforced, Args...> bind(Func && func, Args && ...args) {
    // Bind a callable object with an implicit return type.
    return (binder<Func, unforced, Args...>(
		std::forward<Func>(func), std::forward<Args>(args)...));
}

template <typename Func, typename Ret, typename ...Args>
inline binder<Func, Ret, Args...> bind(Func && func, Args && ...args) {
    // Bind a callable object with an explicit return type.
    return (binder<Func, Ret, Args...>(
		std::forward<Func>(func), std::forward<Args>(args)...));
}

//
// Comparing std::functions for equality?
// See: http://stackoverflow.com/questions/20833453/comparing-stdfunctions-for-equality
//
// C++ trying to get function address from a std::function
// See: http://stackoverflow.com/questions/18039723/c-trying-to-get-function-address-from-a-stdfunction
//
template <typename Ret, typename ...Args>
inline size_t getAddressOf(std::function<Ret(Args...)> func) _NOEXCEPT {
    typedef Ret(funcType)(Args...);
    printf("Ret = %s\n", typeid(Ret).name());
    printf("funcType = %s\n", typeid(funcType).name());
    funcType ** funcPointer = func.template target<funcType *>();
    if (funcPointer != nullptr)
        return reinterpret_cast<size_t>(*funcPointer);
    else
        return 0;
}

template <typename Ret, typename ...Args>
bool function_equal(std::function<Ret(Args...)> const & func1, std::function<Ret(Args...)> const & func2) _NOEXCEPT {
    typedef Ret(FuncType)(Args...);
    size_t sizeArgs = sizeof...(Args);
    Ret(* const * ptr1)(Args...) = func1.target<Ret(*)(Args...)>();
    Ret(* const * ptr2)(Args...) = func2.target<Ret(*)(Args...)>();
    bool is_equal = ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
    return is_equal;
}

template <typename T, typename ...Args>
bool function_equal(std::function<void(Args...)> const & func1, std::function<void(Args...)> const & func2) _NOEXCEPT {
    bool is_equal = false;
    static constexpr size_t sizeArgs = sizeof...(Args);
    //typedef void(cdecl T::*MemberFnType)(T *, Args...);
    //Ret(* const T::* ptr1)(T *, Args...) = func1.target<MemberFnType *>();
    //Ret(* const T::* ptr2)(T *, Args...) = func2.target<MemberFnType *>();
    {
        typedef void(cdecl T::*MemberFnType)(T *, Args...);
        printf("MemberFnType = %s\n", typeid(MemberFnType).name());
        MemberFnType const * ptr1 = func1.template target<MemberFnType>();
        MemberFnType const * ptr2 = func2.template target<MemberFnType>();
        is_equal = ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
    }
    if (is_equal)
        return is_equal;
    {
        std::array<void *, sizeArgs> arr = { 0 };
        auto tuple_list = make_args_tuple(arr);
        typedef void(cdecl T::*MemberFnType)(Args...);
        // decltype(template make_args_tuple<sizeArgs>())
        typedef std::_Binder<std::_Unforced, MemberFnType, T *, std::tuple<Args...>> BindType;
        printf("BindType = %s\n", typeid(BindType).name());
        BindType const * ptr1 = func1.template target<BindType>();
        BindType const * ptr2 = func2.template target<BindType>();
        is_equal = ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
    }
    return is_equal;
}

template <typename T, typename Ret, typename ...Args>
bool function_equal(std::function<Ret(Args...)> const & func1, std::function<Ret(Args...)> const & func2) _NOEXCEPT {
    bool is_equal = false;
    static constexpr size_t sizeArgs = sizeof...(Args);
    {
        typedef Ret(cdecl T::*MemberFnType)(T *, Args...);
        printf("MemberFnType = %s\n", typeid(MemberFnType).name());
        MemberFnType const * ptr1 = func1.template target<MemberFnType>();
        MemberFnType const * ptr2 = func2.template target<MemberFnType>();
        is_equal = ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
    }
    if (is_equal)
        return is_equal;
    {
        std::array<void *, sizeArgs> arr = { 0 };
        auto tuple_list = template make_args_tuple(arr);
        typedef Ret(cdecl T::*MemberFnType)(Args...);
        typedef std::_Binder<Ret, MemberFnType, T *, Args...> BindType;
        printf("BindType = %s\n", typeid(BindType).name());
        BindType const * ptr1 = func1.template target<BindType>();
        BindType const * ptr2 = func2.template target<BindType>();
        is_equal = ((ptr1 != nullptr) && (ptr2 != nullptr) && (ptr1 == ptr2));
    }
    return is_equal;
}

} // namespace jimi

#endif // !QT_FUNCTIONAL_H
