//
//  Author  : github.com/luncliff (luncliff@gmail.com)
//  License : CC BY 4.0
//
#include "test_network.h"
#include "test_shared.h"

using namespace coro;

auto net_getaddrinfo_tcp6_connect_test() {
    init_network_api();
    auto on_return = gsl::finally([]() { release_network_api(); });

    addrinfo hint{};
    hint.ai_family = AF_INET6;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_ALL | AI_NUMERICHOST;

    size_t count = 0u;
    for (auto ep : resolve(hint, "::1", "7")) {
        REQUIRE(ep.in6.sin6_family == AF_INET6);
        REQUIRE(ep.in6.sin6_port == htons(7));

        auto loopback = IN6_IS_ADDR_LOOPBACK(&ep.in6.sin6_addr);
        REQUIRE(loopback);
        ++count;
    }
    REQUIRE(count > 0);
    return EXIT_SUCCESS;
}

#if __has_include(<CppUnitTest.h>)
class net_getaddrinfo_tcp6_connect
    : public TestClass<net_getaddrinfo_tcp6_connect> {
    TEST_METHOD(test_net_getaddrinfo_tcp6_connect) {
        net_getaddrinfo_tcp6_connect_test();
    }
};
#else
int main(int, char* []) {
    return net_getaddrinfo_tcp6_connect_test();
}
#endif
