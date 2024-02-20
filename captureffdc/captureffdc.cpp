//This application uses test data writes to a file and invokes
//D-Bus method CreatePELWithFFDCFiles
//This application is used to validate logging/sbe_ffdc_handler code
#include <iostream>
#include <vector>
#include <string.h>
#include <attributes_info.H>
#include <libphal.H> 
#include <cstring>
#include <sdbusplus/bus.hpp>
#include <xyz/openbmc_project/Logging/Create/server.hpp>
#include <xyz/openbmc_project/Logging/Entry/server.hpp>
#include <fcntl.h> 

extern "C"
{
#include "libpdbg.h"
}
#include <libphal.H>

constexpr uint64_t TARGET_TYPE_OCMB_CHIP = 0x28;
constexpr uint64_t TARGET_TYPE_PROC_CHIP = 0x2;
using namespace openpower::phal;
  
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
  std::cout << "came into getService service " << response[0].first << std::endl;    

  return response[0].first;
}

int getSbeFfdcFd(struct pdbg_target* target)
{
    sbeError_t sbeError;
    try
    {
        std::cout << "before call to  collectDump " << std::endl;
        // Capture FFDC information on ocmb target 
        sbeError = sbe::captureFFDC(target);
        std::cout << "after call to  collectDump " << std::endl;
    }
    catch (const std::exception& e)
    {
        // Failed to collect FFDC information
        std::cout << "captureFFDC: Exception{}" << e.what() << std::endl;
    }

    std::cout << "came into getSbeFfdcFd fd " << sbeError.getFd() << std::endl;    
    return sbeError.getFd();
}
#define SBEFIFO_CMD_CLASS_DUMP              0xAA00
#define   SBEFIFO_CMD_GET_DUMP              0x01 /* long running */
using FFDCData = std::vector<std::pair<std::string, std::string>>;

int createSbeErrorPEL(sdbusplus::bus_t& bus, struct pdbg_target* target)
{
    std::cout << "came into createSbeErrorPEL " << std::endl;
    std::string event = "org.open_power.OCMB.Error.SbeChipOpFailure";

    std::vector<std::tuple<
          sdbusplus::xyz::openbmc_project::Logging::server::Create::FFDCFormat,
          uint8_t, uint8_t, sdbusplus::message::unix_fd>>
          pelFFDCInfo;
    pelFFDCInfo.emplace_back(
            std::make_tuple(sdbusplus::xyz::openbmc_project::Logging::server::
                        Create::FFDCFormat::Custom,
                        static_cast<uint8_t>(0xCB),
                        static_cast<uint8_t>(0x01), getSbeFfdcFd(target)));
    FFDCData pelAdditionalData;
    uint32_t cmd = SBEFIFO_CMD_CLASS_DUMP | SBEFIFO_CMD_GET_DUMP;

    uint32_t chipPos;
    pdbg_target_get_attribute(target, "ATTR_FAPI_POS", 4, 1, &chipPos);
    std::cout << "OCMB fapi position is " << chipPos << std::endl;
    pelAdditionalData.emplace_back("SRC6",
                               std::to_string((chipPos << 16) | cmd));
    pelAdditionalData.emplace_back("CHIP_TYPE",
                               std::to_string(TARGET_TYPE_OCMB_CHIP));
    try
    {
        std::string service = getService(bus, "/xyz/openbmc_project/logging",
                                            "org.open_power.Logging.PEL");
        auto method = bus.new_method_call(service.c_str(), "/xyz/openbmc_project/logging",
                                       "org.open_power.Logging.PEL", "CreatePELWithFFDCFiles");
        method.append(event, "xyz.openbmc_project.Logging.Entry.Level.Error", pelAdditionalData, pelFFDCInfo);
        std::cout << "before calling d-bus method CreatePELWithFFDCFiles " << std::endl;
        auto response = bus.call(method);

        // reply will be tuple containing bmc log id, platform log id
        std::tuple<uint32_t, uint32_t> reply = {0, 0};

        std::cout << "before response.read(reply) " << std::endl;
        // parse dbus response into reply
        response.read(reply);
        int plid = std::get<1>(reply); // platform log id is tuple "second"
        std::cout << "after getting plid " << plid << std::endl;
    }
    catch (const sdbusplus::exception_t& ex)
    {
        std::cout << ex.what() << std::endl;
    }
    catch (const std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }
    return 0;
}

int main()
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

    constexpr uint16_t ODYSSEY_CHIP_ID = 0x60C0;        
    struct pdbg_target *ocmb;
    pdbg_for_each_target("ocmb", NULL, ocmb)
    {
        uint32_t chipId = 0;
        pdbg_target_get_attribute(ocmb, "ATTR_CHIP_ID", 4, 1, &chipId);
	    uint64_t val = 0;
	    std::cout << "found ocmb target with index " << pdbg_target_index(ocmb) << std::endl;
	    if(chipId == ODYSSEY_CHIP_ID)
	    {
			pdbg_target_probe(ocmb);
			if (pdbg_target_status(ocmb) != PDBG_TARGET_ENABLED)
			{
				std::cout << "ocmb chip not enabled " << std::endl;	
				return -1;
			}
            std::cout << "calling createSbeErrorPEL "  << pdbg_target_index(ocmb) << std::endl;
			createSbeErrorPEL(bus, ocmb);
			std::cout << "after calling createSbeErrorPEL " << pdbg_target_index(ocmb) << std::endl;
			break;
	    }
    }

    sleep(20);
    return 0;
}

