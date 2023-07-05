#include <iostream>
#include <sdbusplus/bus.hpp>
#include <xyz/openbmc_project/State/Boot/Progress/server.hpp>
#include <xyz/openbmc_project/State/Host/server.hpp>
#include <sdbusplus/bus.hpp>

using ProgressStages = sdbusplus::xyz::openbmc_project::State::Boot::server::
    Progress::ProgressStages;

std::string getService(sdbusplus::bus_t& bus, const std::string& path,
                     const std::string& interface)
{
  constexpr auto objectMapperName = "xyz.openbmc_project.ObjectMapper";
  constexpr auto objectMapperPath = "/xyz/openbmc_project/object_mapper";

  auto method = bus.new_method_call(objectMapperName, objectMapperPath,
                                    objectMapperName, "GetObject");

  method.append(path);
  method.append(std::vector<std::string>({interface}));

  std::vector<std::pair<std::string, std::vector<std::string>>> response;

  try
  {
      auto reply = bus.call(method);
      reply.read(response);
      if (response.empty())
      {
          std::cout << "Error in mapper response for getting path " <<  path << " interface " << interface << std::endl;
          return std::string{};
      }
  }
  catch (const sdbusplus::exception_t& e)
  {
      std::cout << "Error in mapper response for getting path " <<  path << " interface " << interface << std::endl;
      return std::string{};
  }
  return response[0].first;
}
    
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
        std::cout << "Failed to readProperty " << ex.what() << std::endl;
        throw;
    }
    return std::get<T>(retVal);
}

ProgressStages getBootProgress(sdbusplus::bus::bus& bus)
{
    try
    {
        auto service = getService(bus, "/xyz/openbmc_project/state/host0", "xyz.openbmc_project.State.Boot.Progress");
     
        using PropertiesVariant =
            sdbusplus::utility::dedup_variant_t<ProgressStages>;

        auto retVal = readProperty<PropertiesVariant>(
            bus, service,
            "/xyz/openbmc_project/state/host0",
            "xyz.openbmc_project.State.Boot.Progress", "BootProgress");
        const ProgressStages* progPtr = std::get_if<ProgressStages>(&retVal);
        if (progPtr != nullptr)
        {
            return *progPtr;
        }
    }
    catch (const sdbusplus::exception::SdBusError& ex)
    {
        std::cout << "Failed to read Boot Progress property " << ex.what() << std::endl;
    }
    return ProgressStages::Unspecified;
}

int main()
{
    auto bus = sdbusplus::bus::new_default();
    auto value = getBootProgress(bus);
    std::string bootProgress = sdbusplus::xyz::openbmc_project::State::
        Boot::server::Progress::convertProgressStagesToString(value);
    std::cout << "getBootProgress " << bootProgress
              << std::endl;
    return 0;
}

