#include <sdbusplus/bus.hpp>
#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <sdeventplus/source/event.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/utility/dedup_variant.hpp>

constexpr auto systemEntryObjPath = "/xyz/openbmc_project/dump/system/entry/";
constexpr auto bmcEntryIntf = "xyz.openbmc_project.Dump.Entry.BMC";
constexpr auto dumpObjPath = "/xyz/openbmc_project/dump";
constexpr auto bmcEntryObjPath = "/xyz/openbmc_project/dump/bmc/entry/";
constexpr auto systemEntryIntf = "xyz.openbmc_project.Dump.Entry.System";
constexpr auto hostbootEntryIntf = "com.ibm.Dump.Entry.Hostboot";
constexpr auto sbeEntryIntf = "com.ibm.Dump.Entry.SBE";
constexpr auto hardwareEntryIntf = "com.ibm.Dump.Entry.Hardware";

// clang-format off
using DbusVariantType = sdbusplus::utility::dedup_variant_t<
    std::string,
    int64_t,
    uint64_t,
    double,
    int32_t,
    uint32_t,
    int16_t,
    uint16_t,
    uint8_t,
    bool,
    size_t,
    ProgressStages
 >;

// clang-format on
using DBusInteracesList = std::vector<std::string>;
using DBusPropertiesMap = std::map<std::string, DbusVariantType>;
using DBusInteracesMap = std::map<std::string, DBusPropertiesMap>;
using ManagedObjectType =
    std::vector<std::pair<sdbusplus::message::object_path, DBusInteracesMap>>;

const std::vector<std::string>
    getDumpEntryObjPaths(sdbusplus::bus::bus& bus, std::vector<std::string>& intf)
{
    std::vector<std::string> liObjectPaths;
    try
    {
        auto mapperCall = bus.new_method_call(
            "xyz.openbmc_project.ObjectMapper",
            "/xyz/openbmc_project/object_mapper",
            "xyz.openbmc_project.ObjectMapper", "GetSubTreePaths");
        const int32_t depth = 0;
        mapperCall.append(dumpObjPath);
        mapperCall.append(depth);
        mapperCall.append(intf);
        auto response = bus.call(mapperCall);
        response.read(liObjectPaths);
        std::cout << "Util getDumpEntryObjPaths size " << liObjectPaths.size() << "dump object path" <<  dumpObjPath << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cout << "exception thrown" << std::endl;
    }
    return liObjectPaths;
}

int main()
{
    //bmc entry object paths are
    {
    std::cout << "BMC dump entry object paths " << std::endl;
    std::vector<std::string> liintf{bmcEntryIntf};
    auto bus = sdbusplus::bus::new_default();
    std::vector<std::string> liObjPath = getDumpEntryObjPaths(bus, liintf);
    for(auto& item : liObjPath)
    {
        std::cout << "object path is " << item << std::endl;
    }
    } 
    //system entry object paths are
    {
    std::cout << "system entry object paths " << std::endl;
    std::vector<std::string> liintf{hardwareEntryIntf};
    auto bus = sdbusplus::bus::new_default();
    std::vector<std::string> liObjPath = getDumpEntryObjPaths(bus, liintf);
    for(auto& item : liObjPath)
    {
        std::cout << "object path is " << item << std::endl;
    }
    } 
    auto event = sdeventplus::Event::get_default();
    bus.attach_event(event.get(), SD_EVENT_PRIORITY_NORMAL);

    std::unique_ptr<sdbusplus::bus::match_t> _intfAddWatch = 
        std::make_unique<sdbusplus::bus::match_t>(
            bus,
            sdbusplus::bus::match::rules::interfacesAdded() +
                sdbusplus::bus::match::rules::argNpath(0, bmcEntryObjPath) + sdbusplus::bus::match::rules::interface(bmcEntryIntf),
    [this](auto& msg) {
            sdbusplus::message::object_path objPath;
            DBusInteracesMap interfaces;
            msg.read(objPath, interfaces);
            std::cout << "Watch interfaceAdded path ({})" << objPath.str << std::endl;
    });
    return event.loop();
}

