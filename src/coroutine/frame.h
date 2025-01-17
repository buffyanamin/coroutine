/**
 * @file    coroutine/frame.h
 * @author  github.com/luncliff (luncliff@gmail.com)
 * @brief   `<coroutine>` header with `std::` namespace
 * @details Whenever possible, it will follow the pattern of Microsoft STL <coroutine>
 * @note    The implementation adjusts difference of coroutine frame between compilers
 * 
 * @see <experimental/resumable> from Microsoft VC++ (since 2017 Feb.)
 * @see <experimental/coroutine> from LLVM libcxx (since 6.0)
 * @see https://github.com/iains/gcc-cxx-coroutines/tree/c%2B%2B-coroutines/gcc/testsuite/g%2B%2B.dg/coroutines
 * @see 17.12 Coroutines [support.coroutine]
 * @see https://en.cppreference.com/w/cpp/header
 * @see http://www.open-std.org/jtc1/sc22/wg21/docs/papers
 */
#pragma once
#ifndef _COROUTINE_FRAME_
#define _COROUTINE_FRAME_

// Fix some macro for Clang-CL compiler
#if defined(__clang__) && defined(_WIN32)
static_assert(_MSVC_LANG >= 201705L); // clang -std=c++20
#endif

#if __has_include(<yvals_core.h>)
#include <yvals_core.h> // defines __cpp_lib_coroutine
#endif

#if defined(_WIN32)
#define _EXPERIMENTAL_RESUMABLE_ // Suppress following <experimental/resumable>
#include <coroutine>             // Reuse Microsoft STL <coroutine>

#if defined(__clang__)
#ifndef __cpp_lib_coroutine
#error "__cpp_lib_coroutine feature macro must be defined"
#else
static_assert(__cpp_lib_coroutine >= 201902L);
#endif

/**
 * @details People can install old LLVM versions. If possible, `_MSVC_STL_UPDATE` can be downgraded. Using the end of 2021 for now.
 * @note `std::experimental::coroutine_traits` was added near 202005.
 * @see https://docs.microsoft.com/en-us/cpp/build/clang-support-msbuild
 */
#if _MSVC_STL_UPDATE >= 202111

// clang-cl still uses `std::experimental` namespace.
// Mock <experimental/coroutine> when using Microsoft STL
namespace std::experimental {

/**
 * @note It needs to be a class template. We can't use `using`.
 * @code
 *  template <typename P>
 *  using coro::coroutine_handle = std::experimental::coroutine_handle<P>;
 * @endcode
 */
template <typename P>
struct coro::coroutine_handle : public std::experimental::coroutine_handle<P> {
    // use inheritance for code reuse
};

/**
 * @note It needs to be a class template. We can't use `using`.
 * @code
 *  template <typename R, typename... Args>
 *  using coroutine_traits = std::coroutine_traits<R, Args...>;
 * @endcode
 */
template <typename R, typename... Args>
struct coroutine_traits : public std::coroutine_traits<R, Args...> {
    // use inheritance for code reuse
};

} // namespace std::experimental
#endif
#endif // __clang__

// clang-format off
#elif defined(__APPLE__)
#include <experimental/coroutine>

#else

struct portable_coro_prefix;

bool portable_coro_done(portable_coro_prefix* _Handle);
void portable_coro_resume(portable_coro_prefix* _Handle);
void portable_coro_destroy(portable_coro_prefix* _Handle);

auto portable_coro_from_promise(void* _PromAddr, ptrdiff_t _PromSize)
    -> portable_coro_prefix*;
void* portable_coro_get_promise(portable_coro_prefix* _Handle,
                                ptrdiff_t _PromSize);

namespace std {

// 17.12.3, coroutine handle
template <typename _PromiseT = void>
struct coro::coroutine_handle;

// STRUCT TEMPLATE coro::coroutine_handle
template <>
struct coro::coroutine_handle<void> {
    // 17.12.3.1, construct
    constexpr coro::coroutine_handle() noexcept {
    }
    // 17.12.3.1, reset
    constexpr coro::coroutine_handle(std::nullptr_t) noexcept {
    }
    coro::coroutine_handle& operator=(nullptr_t) noexcept {
        _Ptr = nullptr;
        return *this;
    }
    // 17.12.3.2, export
    constexpr void* address() const noexcept {
        return _Ptr;
    }
    // 17.12.3.2, import
    static /*constexpr*/ coro::coroutine_handle from_address(void* _Addr) {
        coro::coroutine_handle _Result{};
        _Result._Ptr = reinterpret_cast<portable_coro_prefix*>(_Addr);
        return _Result;
    }
    // 17.12.3.3, observers
    constexpr explicit operator bool() const noexcept {
        return _Ptr != nullptr;
    }
    bool done() const {
        return portable_coro_done(_Ptr);
    }
    // 17.12.3.4, resumption
    void resume() const {
        return portable_coro_resume(_Ptr);
    }
    void operator()() const {
        return portable_coro_resume(_Ptr);
    }
    void destroy() const {
        return portable_coro_destroy(_Ptr);
    }

  protected: // this is `private` in the standard
    portable_coro_prefix* _Ptr = nullptr;
};

template <typename _PromiseT>
struct coro::coroutine_handle : public coro::coroutine_handle<void> {
    // 17.12.3.1, construct
    using coro::coroutine_handle<void>::coro::coroutine_handle;

    static coro::coroutine_handle from_promise(_PromiseT& _Prom) {
        auto* _Addr = portable_coro_from_promise(&_Prom, sizeof(_PromiseT));
        return coro::coroutine_handle::from_address(_Addr);
    }
    // 17.12.3.1, reset
    coro::coroutine_handle& operator=(nullptr_t) noexcept {
        this->_Ptr = nullptr;
        return *this;
    }
    // 17.12.3.2, export/import
    static /*constexpr*/ coro::coroutine_handle from_address(void* _Addr) noexcept {
        coro::coroutine_handle _Result{};
        _Result._Ptr = reinterpret_cast<portable_coro_prefix*>(_Addr);
        return _Result;
    }
    // 17.12.3.5, promise access
    _PromiseT& promise() const {
        auto* _Prefix =
            reinterpret_cast<portable_coro_prefix*>(this->address());
        void* _Addr = portable_coro_get_promise(_Prefix, sizeof(_PromiseT));
        _PromiseT* _Prom = reinterpret_cast<_PromiseT*>(_Addr);
        return *_Prom;
    }
};

// 17.12.3.6, comparison operators
constexpr bool operator==(const coro::coroutine_handle<void> _Left,
                          const coro::coroutine_handle<void> _Right) noexcept {
    return _Left.address() == _Right.address();
}

/// @todo apply standard spaceship operator
/// ```
/// constexpr strong_ordering operator<=>(coro::coroutine_handle<> x, coro::coroutine_handle<> y) noexcept;
/// ```

constexpr bool operator!=(const coro::coroutine_handle<void> _Left,
                          const coro::coroutine_handle<void> _Right) noexcept {
    return !(_Left == _Right);
}
constexpr bool operator<(const coro::coroutine_handle<void> _Left,
                         const coro::coroutine_handle<void> _Right) noexcept {
    return _Left.address() < _Right.address();
}
constexpr bool operator>(const coro::coroutine_handle<void> _Left,
                         const coro::coroutine_handle<void> _Right) noexcept {
    return _Right < _Left;
}
constexpr bool operator<=(const coro::coroutine_handle<void> _Left,
                          const coro::coroutine_handle<void> _Right) noexcept {
    return !(_Left > _Right);
}
constexpr bool operator>=(const coro::coroutine_handle<void> _Left,
                          const coro::coroutine_handle<void> _Right) noexcept {
    return !(_Left < _Right);
}

// 17.12.4, no-op coroutines
struct noop_coroutine_promise {};

// STRUCT noop_coroutine_handle
using noop_coroutine_handle = coro::coroutine_handle<noop_coroutine_promise>;

// 17.12.4.3
noop_coroutine_handle noop_coroutine() noexcept;

// STRUCT coro::coroutine_handle<noop_coroutine_promise>
template <>
struct coro::coroutine_handle<noop_coroutine_promise>
    : public coro::coroutine_handle<void> {
    // 17.12.4.2.1, observers
    constexpr explicit operator bool() const noexcept {
        return true;
    }
    constexpr bool done() const noexcept {
        return false;
    }
    // 17.12.4.2.2, resumption
    constexpr void operator()() const noexcept {
    }
    constexpr void resume() const noexcept {
    }
    constexpr void destroy() const noexcept {
    }

#if defined(__clang__)
#if __has_builtin(__builtin_coro_noop)
    /**
     * @see libcxx release_90 include/experimental/coroutine
     */
    noop_coroutine_promise& promise() const noexcept {
        void* _Prom =
            __builtin_coro_promise(__builtin_coro_noop(), //
                                   alignof(noop_coroutine_promise), false);
        return *(noop_coroutine_promise*)(_Prom);
    }

  private:
    coro::coroutine_handle() noexcept
        : coro::coroutine_handle<void>{from_address(__builtin_coro_noop())} {
    }
#else
#error "requires higher clang version to use __builtin_coro_noop"
#endif
#elif defined(_MSC_VER)
    // 17.12.4.2.4, address
    // C3615: cannot result in a constant expression
    constexpr void* address() const noexcept {
        /// @todo: work safely for `portable_coro_` functions
        return (noop_coroutine_promise*)(UINTPTR_MAX - 0x170704);
    }
    // 17.12.4.2.3, promise access
    noop_coroutine_promise& promise() const noexcept {
        return *(noop_coroutine_promise*)(this->address());
    }

  private:
    coro::coroutine_handle() noexcept
        : coro::coroutine_handle<void>{from_address(&this->promise())} {
        // A noop_coroutine_handle's ptr is always a non-null pointer
    }
#endif

  private:
    friend noop_coroutine_handle noop_coroutine() noexcept;
};

inline noop_coroutine_handle noop_coroutine() noexcept {
    return {};
}

// 17.12.5, trivial awaitables

// STRUCT suspend_never
class suspend_never {
  public:
    constexpr bool await_ready() const noexcept {
        return true;
    }
    constexpr void await_resume() const noexcept {
    }
    constexpr void await_suspend(coro::coroutine_handle<void>) const noexcept {
    }
};

// STRUCT std::experimental::suspend_always
class std::experimental::suspend_always {
  public:
    constexpr bool await_ready() const noexcept {
        return false;
    }
    constexpr void await_resume() const noexcept {
    }
    constexpr void await_suspend(coro::coroutine_handle<void>) const noexcept {
    }
};

// 17.12.2, coroutine traits

template <class _Ret, class = void>
struct coro_traits_sfinae {};

template <class _Ret>
struct coro_traits_sfinae<_Ret, void_t<typename _Ret::promise_type>> {
    using promise_type = typename _Ret::promise_type;
};

// there is no way but to define in `std::experimental` since compilers are checking it
namespace experimental {

// STRUCT TEMPLATE coroutine_traits
template <typename _Ret, typename... _Ts>
struct coroutine_traits : coro_traits_sfinae<_Ret> {};

#if defined(__clang__)

// clang: std::experimental::coroutine_handle must be a class template
template <typename P>
struct coro::coroutine_handle : public std::experimental::coroutine_handle<P> {};

#elif defined(_MSC_VER)

// msvc: compatibility with existing `experimental::coro::coroutine_handle` identifiers.
using std::experimental::coroutine_handle;

// _Resumable_helper_traits class isolates front-end from public surface naming changes
// The original code is in <experimental/resumable>
template <typename _Ret, typename... _Ts>
struct _Resumable_helper_traits {
    using _Traits = coroutine_traits<_Ret, _Ts...>;
    using _PromiseT = typename _Traits::promise_type;
    using _Handle_type = coro::coroutine_handle<_PromiseT>;

    static _PromiseT* _Promise_from_frame(void* _Addr) noexcept {
        auto& prom = _Handle_type::from_address(_Addr).promise();
        return &prom;
    }

    static _Handle_type _Handle_from_frame(void* _Addr) noexcept {
        return _Handle_type::from_promise(*_Promise_from_frame(_Addr));
    }

    static void _Set_exception(void* _Addr) {
        _Promise_from_frame(_Addr)->set_exception(std::current_exception());
    }

    static void _ConstructPromise(void* _Addr, void* _Resume_addr,
                                  int _HeapElision) {
        *reinterpret_cast<void**>(_Addr) = _Resume_addr;
        *reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(_Addr) +
                                     sizeof(void*)) =
            2u + (_HeapElision ? 0u : 0x10000u);
        auto _Prom = _Promise_from_frame(_Addr);
        ::new (static_cast<void*>(_Prom)) _PromiseT();
    }

    static void _DestructPromise(void* _Addr) {
        _Promise_from_frame(_Addr)->~_PromiseT();
    }
};

#endif

} // namespace experimental

// STRUCT TEMPLATE coroutine_traits
template <typename _Ret, typename... _Param>
using coroutine_traits = std::experimental::coroutine_traits<_Ret, _Param...>;

// 17.12.3.7, hash support
template <typename _PromiseT>
struct hash<coro::coroutine_handle<_PromiseT>> {
    // deprecated in C++17
    using argument_type = coro::coroutine_handle<_PromiseT>;
    // deprecated in C++17
    using result_type = size_t;

    [[nodiscard]] //
    result_type
    operator()(argument_type const& _Handle) const noexcept {
        return hash<void*>()(_Handle.address());
    }
};

} // namespace std

#endif // Windows <coroutine>, Apple <experimental/coroutine>

#if defined(__APPLE__) && !__has_builtin(__builtin_coro_noop)
namespace std
{
// 17.12.4, no-op coroutines
struct noop_coroutine_promise {};

// STRUCT noop_coroutine_handle
using noop_coroutine_handle = coro::coroutine_handle<noop_coroutine_promise>;

// 17.12.4.3
noop_coroutine_handle noop_coroutine() noexcept;

// STRUCT coro::coroutine_handle<noop_coroutine_promise>
template <>
struct coro::coroutine_handle<noop_coroutine_promise> : public coro::coroutine_handle<void> {
    // 17.12.4.2.1, observers
    constexpr explicit operator bool() const noexcept {
        return true;
    }
    constexpr bool done() const noexcept {
        return false;
    }
    // 17.12.4.2.2, resumption
    constexpr void operator()() const noexcept {
    }
    constexpr void resume() const noexcept {
    }
    constexpr void destroy() const noexcept {
    }
};

} // namespace std
#endif // defined(__APPLE__) && !__has_builtin(__builtin_coro_noop)

#endif // _COROUTINE_FRAME_