#include <iostream>
#include <sdbusplus/bus.hpp>
#include <vector>
#include <libpdbg.h>
#include <string.h>
#include <attributes_info.H>
#include <libphal.H> 

constexpr auto CpuCoreIface = "xyz.openbmc_project.Inventory.Item.CpuCore";
constexpr auto mapperBusNmae = "xyz.openbmc_project.ObjectMapper";
constexpr auto mapperObjectPath = "/xyz/openbmc_project/object_mapper";
constexpr auto mapperInterface = "xyz.openbmc_project.ObjectMapper";
constexpr auto CpuIface = "xyz.openbmc_project.Inventory.Item.Cpu";

// a core target is guarded
// get the parent proc of the core target and its location code
// loop through inventory to get the cpu object path matching the location code
// using the inventory cpu object path append the core path with target index
// now use the core object path and query for the functional property in inventory

void pdbgLogCallback(int, const char* fmt, va_list ap)
{
    va_list vap;
    va_copy(vap, ap);
    std::vector<char> logData(1 + std::vsnprintf(nullptr, 0, fmt, ap));
    std::vsnprintf(logData.data(), logData.size(), fmt, vap);
    va_end(vap);
    std::string logstr(logData.begin(), logData.end());
    std::cout << "PDBG:" << logstr << std::endl;
}
std::string getUnexpandedLocCode(const std::string& locCode)
{
    std::string unExpandedLocCode;
    // Find the position of the last dash
    size_t lastDashPos = locCode.rfind('-');

    if (lastDashPos == std::string::npos) 
    {
        return unExpandedLocCode;
    }
    // Find the position of the second-to-last dash before the last dash
    size_t secondLastDashPos = locCode.rfind('-', lastDashPos - 1);

    if (secondLastDashPos == std::string::npos) 
    {
        return unExpandedLocCode;
    }
    // Read the string from the second-to-last dash to the end
    std::string readValue = locCode.substr(secondLastDashPos + 1, locCode.length()-1);

  
    unExpandedLocCode += "Ufcs";
    unExpandedLocCode += "-";
    unExpandedLocCode += readValue;
    return unExpandedLocCode;
}

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
T readProperty(sdbusplus::bus::bus& bus, const std::string& service,
               const std::string& object, const std::string& intf,
               const std::string& prop)
{
    std::variant<T> retVal{};
    try
    {
        auto properties =
            bus.new_method_call(service.c_str(), object.c_str(),
                                "org.freedesktop.DBus.Properties", "Get");
        properties.append(intf);
        properties.append(prop);
        auto result = bus.call(properties);
        result.read(retVal);
    }
    catch (const std::exception& ex)
    {
       //do nothing property might not exist
    }
    return std::get<T>(retVal);
}

bool getCoreFunctionalProp(sdbusplus::bus::bus& bus, const sdbusplus::message::object_path& path)
{
    return readProperty<bool>(bus, "xyz.openbmc_project.PLDM",
                                   path,
                                   "xyz.openbmc_project.State.Decorator.OperationalStatus",
                                   "Functional");
}
std::string getCoreLocationCode(sdbusplus::bus::bus& bus, const sdbusplus::message::object_path& path)
{
    return readProperty<std::string>(bus, "xyz.openbmc_project.PLDM",
                                   path,
                                   "xyz.openbmc_project.Inventory.Decorator.LocationCode",
                                   "LocationCode");
}

std::string getProcLocationCode(sdbusplus::bus::bus& bus, const sdbusplus::message::object_path& path)
{
    return readProperty<std::string>(bus, "xyz.openbmc_project.Inventory.Manager",
                                   path,
                                   "xyz.openbmc_project.Inventory.Decorator.LocationCode",
                                   "LocationCode");
}

const std::string getProcObjectPath(sdbusplus::bus::bus& bus, const std::string& coreLocCode)
{
    try
    {
        std::vector<std::string> paths;
        auto method = bus.new_method_call(mapperBusNmae, mapperObjectPath,
                                            mapperInterface, "GetSubTreePaths");
        method.append("/xyz/openbmc_project/inventory");
        method.append(0); // Depth 0 to search all
        method.append(std::vector<std::string>({"xyz.openbmc_project.Inventory.Item.Cpu"}));
        auto reply = bus.call(method);
        reply.read(paths);
        std::cout << "cpu object paths found is " << paths.size() << std::endl;
        for(const auto& path: paths)
        {
            std::string locCode = getUnexpandedLocCode(getProcLocationCode(bus, path));
            std::cout << "coreloccode " << coreLocCode << " inventorylocCode " << locCode << std::endl;
            if(locCode == coreLocCode)
            {
                return path;
            }
        }
    }
    catch(const std::exception& ex)
    {
        std::cout << "exception raised " << ex.what() << std::endl;
    }
    return "";    
}

int main()
{
    try
    {
        constexpr auto devtree = "/var/lib/phosphor-software-manager/pnor/rw/DEVTREE";

        // PDBG_DTB environment variable set to CEC device tree path
        if (setenv("PDBG_DTB", devtree, 1))
        {
            std::cerr << "Failed to set PDBG_DTB: " << strerror(errno) << std::endl;
            return 0;
        }

        constexpr auto PDATA_INFODB_PATH = "/usr/share/pdata/attributes_info.db";
        // PDATA_INFODB environment variable set to attributes tool  infodb path
        if (setenv("PDATA_INFODB", PDATA_INFODB_PATH, 1))
        {
            std::cerr << "Failed to set PDATA_INFODB: ({})" << strerror(errno) << std::endl;
            return 0;
        }


        //initialize the targeting system 
        if (!pdbg_targets_init(NULL))
        {   
            std::cerr << "pdbg_targets_init failed" << std::endl;
            return 0;
        }

        auto bus = sdbusplus::bus::new_default();

        // set log level and callback function
        pdbg_set_loglevel(PDBG_DEBUG);
        pdbg_set_logfunc(pdbgLogCallback);

        struct pdbg_target* coreTarget;
        pdbg_for_each_class_target("core", coreTarget)
        {
            if(openpower::phal::pdbg::isTgtPresent(coreTarget) && openpower::phal::pdbg::isTgtFunctional(coreTarget))
            {
                ATTR_LOCATION_CODE_Type locationCode1 = {'\0'};
                ATTR_LOCATION_CODE_Type locationCode2 = {'\0'};
                // Get location code information
                openpower::phal::pdbg::getLocationCode(coreTarget, locationCode1);
                struct pdbg_target* procTarget = pdbg_target_parent("proc", coreTarget);
                openpower::phal::pdbg::getLocationCode(procTarget, locationCode2);
                std::cout << "Target Index " << std::to_string(pdbg_target_index(coreTarget)) << 
                    " Location code core " << locationCode1 << " proc " << locationCode2 << std::endl;
                std::string procObjPath = getProcObjectPath(bus, locationCode2);
                std::string coreObjPath = procObjPath;
                coreObjPath += "/";
                coreObjPath += "core";
                coreObjPath += std::to_string(pdbg_target_index(coreTarget));
                bool functional = getCoreFunctionalProp(bus, coreObjPath); 
                if(functional)
                {
                    std::cout << "core is functional " << coreObjPath << std::endl;
                }
                else
                {
                    std::cout << "core is not functional " << coreObjPath << std::endl;
                }
                break;
            }
        }

        ///xyz/openbmc_project/inventory/system/chassis/motherboard/dcm0/cpu0
 
        /*       
        std::vector<std::string> paths;
        //xyz.openbmc_project.PLDM", "/xyz/openbmc_project/pldm
        auto method = bus.new_method_call(mapperBusNmae, mapperObjectPath,
                                            mapperInterface, "GetSubTreePaths");
        method.append("/xyz/openbmc_project/inventory");
        method.append(0); // Depth 0 to search all
        method.append(std::vector<std::string>({"xyz.openbmc_project.Inventory.Item.CpuCore"}));
        auto reply = bus.call(method);
      
        reply.read(paths);
        std::cout << "CpuCores size found is " << paths.size() << std::endl;
        for(const auto& path: paths)
        {
            std::string locCode = getLocationCode(bus, path);
            std::cout << "object path " << path << " location code " << locCode <<  " unexpanded " << getUnexpandedLocCode(locCode) << std::endl;
        }
        */
        // you have core/fc ta\rget
    }
    catch(const std::exception& ex)
    {
        std::cout << "exception raised " << ex.what() << std::endl;
    }    
    return 0;
}
