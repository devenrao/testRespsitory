#include <sdbusplus/async.hpp>

#include <iostream>
#include <string>
#include <variant>
#include <vector>

auto startup(sdbusplus::async::context& ctx) -> sdbusplus::async::task<>
{
  try 
  {
    constexpr auto loggerObjectPath = "/xyz/openbmc_project/logging";
    constexpr auto loggerCreateInterface = "xyz.openbmc_project.Logging.Create";
    constexpr auto loggerService = "xyz.openbmc_project.Logging";
    constexpr auto dumpFileString = "File Name";
    constexpr auto dumpFileTypeString = "Dump Type";
    constexpr auto dumpIdString = "Dump ID";

    const std::string dumpFilePath {"/var/lib/phosphor/BMC/dumps/entry/TestDump"};
    const std::string dumpFileType {"BMC"};
    const int dumpId{99};
    const std::string pelSev {"xyz.openbmc_project.Logging.Entry.Level.Informational"};
    const std::string errIntf {"xyz.openbmc_project.Dump.Error.Invalidate"};

    const auto systemd = sdbusplus::async::proxy()
                             .service(loggerService)
                             .path(loggerObjectPath)
                             .interface(loggerCreateInterface)
                             .preserve();

    const std::unordered_map<std::string_view, std::string_view> userDataMap = {
        {dumpIdString, std::to_string(dumpId)},
        {dumpFileString, dumpFilePath},
        {dumpFileTypeString, dumpFileType}};
    
    std::cout << "calling create pel " << std::endl;
    co_await systemd.call<>(ctx, "Create", errIntf, pelSev, userDataMap);
    std::cout << "waiting for create call" << std::endl;
    // We are all done, so shutdown the server.
    ctx.request_stop();
    co_return;
  } 
  catch (const std::exception &e)
  {
    std::cerr << "Error in calling creating PEL. Standard exception caught : "
              << e.what() << std::endl;
  }

}


int main()
{
    sdbusplus::async::context ctx;
    ctx.spawn(startup(ctx));
    ctx.run();

    return 0;
}

