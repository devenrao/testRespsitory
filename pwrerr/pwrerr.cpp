#include "xyz/openbmc_project/Common/error.hpp"
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>
#include <map>
#include <iostream>
#include <sdeventplus/source/event.hpp>
#include <sdbusplus/bus.hpp>
#include <xyz/openbmc_project/Logging/Create/server.hpp>
#include <xyz/openbmc_project/Logging/Entry/server.hpp>
#include <format>

using namespace phosphor::logging;
using Severity = sdbusplus::xyz::openbmc_project::Logging::server::Entry::Level;

int main()
{
    std::map<std::string, std::string> additionalData;
    auto bus = sdbusplus::bus::new_default();
	additionalData["STATUS_VOUT"] = std::format("{:#02x}", 0x30);    
	additionalData["CALLOUT_INVENTORY_PATH"] = "/xyz/openbmc_project/inventory/system/chassis/motherboard/powersupply1";

    try
    {
        auto method = bus.new_method_call("xyz.openbmc_project.Logging", 
                        "/xyz/openbmc_project/logging",
                        "xyz.openbmc_project.Logging.Create", "Create");
        auto level =
            sdbusplus::xyz::openbmc_project::Logging::server::convertForMessage(
                Severity::Error);
                        
        method.append("xyz.openbmc_project.Power.PowerSupply.Error.Fault", level, additionalData);
        auto resp = bus.call(method);
    }
    catch (const sdbusplus::exception_t& e)
    {
        std::cout << "sdbusplus exception " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << " exception " << e.what() << std::endl;
    }
    return 0;
}
