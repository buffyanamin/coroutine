/**
 * @author github.com/luncliff (luncliff@gmail.com)
 */

#undef NDEBUG
#include <cassert>

#include <coroutine/channel.hpp>
#include <coroutine/return.h>

using namespace std;
using namespace coro;

using channel_without_lock_t = channel<int>;

int main(int, char*[]) {
    auto ch = make_unique<channel_without_lock_t>();
    bool ok = true;

    auto coro_write = [&ok](auto& ch, auto value) -> frame_t {
        ok = co_await ch.write(value);
    };
    // coroutine will suspend and wait in the channel
    auto h = coro_write(*ch, int{});
    {
        auto truncator = move(ch); // if channel is destroyed ...
    }
    assert(ch.get() == nullptr);

    coro::coroutine_handle<void>& coro = h;
    assert(coro.done()); // coroutine is in done state
    coro.destroy();      // destroy to prevent leak

    assert(ok == false); // and channel returned false
    return EXIT_SUCCESS;
}
