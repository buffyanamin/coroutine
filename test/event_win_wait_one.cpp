//
//  Author  : github.com/luncliff (luncliff@gmail.com)
//  License : CC BY 4.0
//
#include <coroutine/event.h>
#include <coroutine/return.h>
#include <coroutine/thread.h>

#include <atomic>

#include "test.h"
using namespace std;
using namespace coro;

// we can't use rvalue reference. this design is necessary because
// `set_or_cancel` uses INFINITE wait internally.
// so, with the reference, user must sure one of `SetEvent` or `cancel` will
// happen in the future
auto wait_an_event(set_or_cancel& token, atomic_flag& flag) -> forget_frame {
    // wait for set or cancel
    // `co_await` will forward `GetLastError` if canceled.
    if (DWORD ec = co_await token) {
        _fail_now_(system_category().message(ec).c_str(), __FILE__, __LINE__);
        co_return;
    }
    flag.test_and_set();
}

auto set_after_sleep(HANDLE ev, uint32_t ms) -> forget_frame {
    co_await ptp_work{}; // move to background thread ...

    SleepEx(ms, true);
    // if failed, print error message
    if (SetEvent(ev) == 0) {
        auto ec = GetLastError();
        _fail_now_(system_category().message(ec).c_str(), __FILE__, __LINE__);
    }
}

auto set_or_cancel_wait_one_test() {
    HANDLE e = CreateEventEx(nullptr, nullptr, //
                             CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS);
    if (e) // if statement because of C6387
        ResetEvent(e);
    _require_(e != NULL);

    auto on_return = gsl::finally([e]() { CloseHandle(e); });

    auto ms = rand() & 0b1111; // at most 16 ms
    set_after_sleep(e, ms);

    SleepEx(3, true);

    // issue:
    //	CI environment runs slowly, so too short timeout might fail ...
    //	wait for 300 ms
    auto ec = WaitForSingleObjectEx(e, 200, true);
    _require_(ec == WAIT_OBJECT_0);

    return EXIT_SUCCESS;
}

#if defined(CMAKE_TEST)
int main(int, char*[]) {
    return set_or_cancel_wait_one_test();
}

#elif __has_include(<CppUnitTest.h>)
#include <CppUnitTest.h>

template <typename T>
using TestClass = ::Microsoft::VisualStudio::CppUnitTestFramework::TestClass<T>;

class set_or_cancel_wait_one : public TestClass<set_or_cancel_wait_one> {
    TEST_METHOD(test_set_or_cancel_wait_one) {
        set_or_cancel_wait_one_test();
    }
};
#endif
