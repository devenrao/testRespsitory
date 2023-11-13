#include <iostream>
#include <vector>
#include <string.h>
#include <attributes_info.H>
#include <libphal.H> 
#include <cstring>

extern "C"
{
#include "libpdbg.h"
}

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

static int print_target_data(struct pdbg_target *target)
{
    std::cout << "--------------------------------------------" << std::endl;
    const char *classname = pdbg_target_class_name(target);
    if(classname)
    {
        std::cout << "pdbg target class is " <<  classname << std::endl;
    }

    const char* targetname =  pdbg_target_name(target);
    if(targetname)
    {
        std::cout << "pdbg target name is " << targetname << std::endl;
    }
    uint8_t type;
    if (pdbg_target_get_attribute(target, "ATTR_TYPE", 1, 1, &type)) 
    {
        std::cout << "target type " <<  (short)type << std::endl;
    }
    const char* targetPath = pdbg_target_path(target);
    if( targetPath)
    {
        std::cout << "pdbg target path is " << targetPath << std::endl;
    }
    std::cout << "pdbg target index is " << std::hex << "0x" << pdbg_target_index(target) << std::endl;
    size_t len;
    const char* compatible = (const char*)pdbg_target_property(target, "compatible", &len);
    if( compatible)
    {
        std::cout << "pdbg compatible is " << compatible << std::endl;
    }
    const char* system_path = (const char*)pdbg_target_property(target, "system-path", &len);
    if( system_path)
    {
        std::cout << "pdbg system-path is " << system_path << std::endl;
    }
    const char* device_path = (const char*)pdbg_target_property(target, "device-path", &len);
    if(device_path)
    {
        std::cout << "pdbg device-path is " << device_path << std::endl;
    }
    uint32_t port;
    /* Enable the port in the upstream control register */
    if (pdbg_target_u32_property(target, "port", &port)) {
	    printf("pdbg target port is 0x%x \n", port);
    }    
    return 0;
}

void captureFFDC(struct pdbg_target *ocmb)
{
	
}

/** @struct DumpPtr
* @brief a structure holding the data pointer
* @details This is a RAII container for the dump data
* returned by the SBE
*/
struct DumpDataPtr
{
public:
  /** @brief Destructor for the object, free the allocated memory.
   */
  ~DumpDataPtr()
  {
      // The memory is allocated using malloc
      free(dataPtr);
  }
  /** @brief Returns the pointer to the data
   */
  uint8_t** getPtr()
  {
      return &dataPtr;
  }
  /** @brief Returns the stored data
   */
  uint8_t* getData()
  {
      return dataPtr;
  }

private:
  /** The pointer to the data */
  uint8_t* dataPtr = nullptr;
};
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

    // set log level and callback function
    pdbg_set_loglevel(PDBG_DEBUG);
    pdbg_set_logfunc(pdbgLogCallback);

	/*
    struct pdbg_target *pib;
    pdbg_for_each_target("pib-ody", NULL, pib)
    {
        print_target_data(pib);
    }

    {
		std::cout << "******proc0 sbefifo pib target ***** " << std::endl;
		char path[16] = "/proc0/pib";
		struct pdbg_target *pib = pdbg_target_from_path(nullptr, path);
		print_target_data(pib);
    }
    */
    constexpr uint16_t ODYSSEY_CHIP_ID = 0x60C0;        
    struct pdbg_target *ocmb;
    pdbg_for_each_target("ocmb", NULL, ocmb)
    {
        uint32_t chipId = 0;
        pdbg_target_get_attribute(ocmb, "ATTR_CHIP_ID", 4, 1, &chipId);
    	uint32_t proc = pdbg_target_index(pdbg_target_parent("proc", ocmb));
	    uint64_t val = 0;
	    if(chipId == ODYSSEY_CHIP_ID)
	    {
			// Clock state requested
			// Collect the dump with clocks on
			constexpr int SBE_CLOCK_ON = 0x1;
			//constexpr auto SBE_CLOCK_OFF = 0x2;	    
			pdbg_target_probe(ocmb);

			if (pdbg_target_status(ocmb) != PDBG_TARGET_ENABLED)
			{
				std::cout << "ocmb chip not enabled " << std::endl;	
				return -1;
			}
			//printf("getting ody chipop target \n");
			struct pdbg_target *co = get_ody_chipop_target(ocmb);
			if (pdbg_target_probe(co) != PDBG_TARGET_ENABLED)
			{
				std::cout << "chipop target is not enabled " << std::endl; 
				return -1;
			}

			printf("getting ody fsi target \n");
			struct pdbg_target *fsi = get_ody_fsi_target(ocmb);
			if (pdbg_target_probe(fsi) != PDBG_TARGET_ENABLED)
			{
				std::cout << "fsi target is not enabled " << std::endl; 
				return -1;
			}

			std::cout << "*******ocmb sbefifo ody pib target****** " << std::endl;
			struct pdbg_target *pib = get_ody_pib_target(ocmb);
			if (pdbg_target_probe(pib) != PDBG_TARGET_ENABLED)
			{
				std::cout << "pib target is not enabled " << std::endl; 
				return -1;
			}
			DumpDataPtr dataPtr;
			uint32_t len = 0;
			constexpr int SBE_DUMP_TYPE_HOSTBOOT = 0x5;
			std::cout << "before calling get_ody_pib_target " << std::endl; 
			std::cout << "after calling get_ody_pib_target " << std::endl;			
			//if (sbe_dump(pib, SBE_DUMP_TYPE_HOSTBOOT, SBE_CLOCK_ON, 0, dataPtr.getPtr(), &len)) {
			//	std::cout << "failed to collect sbe dump " << std::endl;
			//	return 0;
			//}
			//else
			//{
			//	std::cout << "sucessfully collected dump data len is " << len << std::endl;
			//}
			uint8_t *ffdc = NULL;
			uint32_t status = 0;
			uint32_t ffdc_len = 0;
			int ret;
			ret = sbe_ffdc_get(pib, &status, &ffdc, &ffdc_len);
			if(ret != 0){
				std::cout << "failed to get ffdc data " << ret << std::endl;
			} else {
				std::cout << "ffdc data status is " << status << " ffdc_len " << ffdc_len << std::endl;
			}
			break;
	    }
    }
    return 0;
}
