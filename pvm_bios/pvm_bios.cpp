#include <sdbusplus/bus.hpp>
#include <iostream>
#include <string>
#include <variant>
#include <vector>
#include <sdeventplus/source/event.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/utility/dedup_variant.hpp>
#include <format>
#include <sdbusplus/bus/match.hpp>
/**
 * @brief Read property value from the specified object and interface
 * @param[in] bus D-Bus handle
 * @param[in] service service which has implemented the interface
 * @param[in] object object having has implemented the interface
 * @param[in] intf interface having the property
 * @param[in] prop name of the property to read
 * @return property value
 */
template <typename T>
std::expected<T, std::string> readDBusProperty(sdbusplus::bus::bus& bus,
                                               const std::string& service,
                                               const std::string& object,
                                               const std::string& intf,
                                               const std::string& prop) 
{
    try 
    {
        T retVal{};
        auto properties = bus.new_method_call(service.c_str(), object.c_str(),
                                              "org.freedesktop.DBus.Properties", "Get");
        properties.append(intf, prop);
        auto result = bus.call(properties);
        result.read(retVal);
        return retVal; // Success case
    } 
    catch (const std::exception& ex) 
    {
        return std::unexpected(std::format("Failed to get the property ({}): {}", prop, ex.what()));
    }
}

using BaseBIOSTableItem =
        std::tuple<std::string, bool, std::string, std::string,
        std::string, std::variant<int64_t, std::string>,
         std::variant<int64_t, std::string>, 
         std::vector<std::tuple<std::string, std::variant<int64_t, std::string>, std::string>>>;

using BaseBIOSTableItemList = std::map<std::string, BaseBIOSTableItem>;
bool isSystemHMCManaged(sdbusplus::bus::bus& bus) 
{
    try
    {
        auto retVal = readDBusProperty<std::variant<BaseBIOSTableItemList>>(
            bus, "xyz.openbmc_project.BIOSConfigManager",
            "/xyz/openbmc_project/bios_config/manager",
            "xyz.openbmc_project.BIOSConfig.Manager", "BaseBIOSTable");

        if (!std::holds_alternative<BaseBIOSTableItemList>(retVal)) 
        {
            std::cout << "Base BIOS table is null or invalid.\n";
            return false;
        }

        const auto& baseBiosTable = std::get<BaseBIOSTableItemList>(retVal);

        std::cout << "*biosTable size is " << baseBiosTable.size() << '\n';

        // Use find instead of looping
        const auto it = baseBiosTable.find("pvm_hmc_managed");
        if (it != baseBiosTable.end()) 
        {
            const auto& value = it->second;
            const auto& attrValue = std::get<5>(value);
            if (std::holds_alternative<std::string>(attrValue)) 
            {
                return std::get<std::string>(attrValue) == "Enabled";
            }
        }
        else 
        {
            std::cout << "Attribute 'pvm_hmc_managed' not found.\n";
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Error: " << ex.what() << '\n';
    }

    std::cout << "System is not HMC managed.\n";
    return false;
}

void propertyChanged(sdbusplus::message::message& msg) 
{
    std::cout << "Property changed signal received.\n";

    if (msg.is_method_error()) 
    {
        std::cerr << "Error in reading BIOS attribute signal.\n";
        return;
    }

    using BiosBaseTableMap = std::map<std::string, std::variant<BaseBIOSTableItemList>>;
    std::string object;
    BiosBaseTableMap propMap;
    msg.read(object, propMap);

    std::cout << "Property map size: " << propMap.size() << '\n';

    if (auto it = propMap.find("BaseBIOSTable"); it != propMap.end()) 
    {
        const auto& baseBiosTableItemList = std::get<BaseBIOSTableItemList>(it->second);
        std::cout << "BaseBIOSTableItem size: " << baseBiosTableItemList.size() << '\n';

        const auto attrIt = baseBiosTableItemList.find("pvm_hmc_managed");
        if (attrIt != baseBiosTableItemList.end()) 
        {
            const auto& attrValue = std::get<5>(attrIt->second);
            if (std::holds_alternative<std::string>(attrValue)) 
            {
                const std::string& strValue = std::get<std::string>(attrValue);
                std::cout << (strValue == "Enabled" ? "HMC managed\n" : "Not HMC managed\n");
            }
        }
    }
}

int main()
{
    auto bus = sdbusplus::bus::new_default();
    if (isSystemHMCManaged(bus))
    {
        std::cout << "System is HMC managed" << std::endl; 
    }
    else
    {
        std::cout << "System is not HMC managed " << std::endl;
    }
    std::unique_ptr<sdbusplus::bus::match_t> _hmcStatePropWatch =
         std::make_unique<sdbusplus::bus::match_t>(
            bus,
         sdbusplus::bus::match::rules::propertiesChanged(
                "/xyz/openbmc_project/bios_config/manager",
                "xyz.openbmc_project.BIOSConfig.Manager"),
            [](auto& msg) { propertyChanged(msg); });

        auto event = sdeventplus::Event::get_default();
        bus.attach_event(event.get(), SD_EVENT_PRIORITY_NORMAL);
        return event.loop();      
}
