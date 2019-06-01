//
//  Author  : github.com/luncliff (luncliff@gmail.com)
//  License : CC BY 4.0
//
#include "test_shared.h"

using namespace coro;
using namespace concrt;

using value_type = int;
using channel_without_lock_t = channel<value_type>;

template <typename E, typename L>
auto write_to(channel<E, L>& ch, E value, bool ok = false) -> no_return {
    ok = co_await ch.write(value);
    if (ok == false)
        // !!!!!
        // seems like clang optimizer is removing `value`.
        // so using it in some pass makes
        // the symbol and its memory location alive
        // !!!!!
        value += 1;
    REQUIRE(ok);
}

template <typename E, typename L>
auto read_from(channel<E, L>& ch, E& value, bool ok = false) -> no_return {

    tie(value, ok) = co_await ch.read();
    REQUIRE(ok);
}

auto coro_channel_write_before_read_test() {
    const auto list = {1, 2, 3};
    channel_without_lock_t ch{};
    value_type storage = 0;

    for (auto i : list) {
        write_to(ch, i);       // Writer coroutine will suspend
        REQUIRE(storage != i); // so no write occurs
    }
    for (auto i : list) {
        read_from(ch, storage); // read to `storage`
        REQUIRE(storage == i);  // stored value is same with sent value
    }

    return EXIT_SUCCESS;
}

#if __has_include(<CppUnitTest.h>)
class coro_channel_write_before_read
    : public TestClass<coro_channel_write_before_read> {
    TEST_METHOD(test_coro_channel_write_before_read) {
        coro_channel_write_before_read_test();
    }
};
#else
int main(int, char*[]) {
    return coro_channel_write_before_read_test();
}
#endif
