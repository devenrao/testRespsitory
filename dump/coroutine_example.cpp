#include <sdbusplus/async.hpp>

#include <iostream>
#include <string>
#include <variant>
#include <vector>

auto startup(sdbusplus::async::context& ctx) -> sdbusplus::async::task<>
{
    // Create a proxy to the systemd manager object.
    constexpr auto systemd = sdbusplus::async::proxy()
                                 .service("org.freedesktop.systemd1")
                                 .path("/org/freedesktop/systemd1")
                                 .interface("org.freedesktop.systemd1.Manager");

    // Call ListUnitFiles method.
    using ret_type = std::vector<std::tuple<std::string, std::string>>;
    for (auto& [file, status] :
         co_await systemd.call<ret_type>(ctx, "ListUnitFiles"))
    {
        std::cout << file << " " << status << std::endl;
    }
    // We are all done, so shutdown the server.
    ctx.request_stop();

    co_return;
}

int main()
{
    sdbusplus::async::context ctx;
    ctx.spawn(startup(ctx));
    ctx.run();

    return 0;
}
