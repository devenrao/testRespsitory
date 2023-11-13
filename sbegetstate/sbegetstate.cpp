#include <iostream>
#include <vector>
#include <string.h>
#include <attributes_info.H>
#include <libphal.H> 
#include <unistd.h>

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
    const char* targetname =  pdbg_target_name(target);
    if(targetname)
    {
        std::cout << "pdbg target name is " << targetname << std::endl;
    }
    std::cout << "pdbg target index is " << pdbg_target_index(target) << std::endl;
    char tgtPhysDevPath[64];        
    if (pdbg_target_get_attribute(target, "ATTR_PHYS_DEV_PATH", 1, 64,
  				       tgtPhysDevPath)) 
    {
        std::cout << "target devpath " <<  tgtPhysDevPath << std::endl;
  	}
    uint8_t type;
    if (pdbg_target_get_attribute(target, "ATTR_TYPE", 1, 1, &type)) 
    {
        std::cout << "target type " <<  (short)type << std::endl;
    }
    const char *classname = pdbg_target_class_name(target);
    if(classname)
    {
        std::cout << "pdbg target class is " <<  classname << std::endl;
    }
    const char* targetPath = pdbg_target_path(target);
    if( targetPath)
    {
        std::cout << "pdbg target path is " << targetPath << std::endl;
    }
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
    
    //std::cout << "calling probe method " << std::endl;
    //pdbg_target_probe_all(pdbg_target_root());
    //std::cout << "sbefifo pdbg target index is " <<   << std::endl;
    return 0;
}

int main()
{
    constexpr auto devtree = "/var/lib/phosphor-software-manager/pnor/rw/DEVTREE";

	printf("devender came into main method \n");
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

	pdbg_set_backend(PDBG_BACKEND_KERNEL, NULL);

    //initialize the targeting system 
    if (!pdbg_targets_init(NULL))
    {   
        std::cerr << "pdbg_targets_init failed" << std::endl;
        return 0;
    }

    // set log level and callback function
    pdbg_set_loglevel(PDBG_DEBUG);
    pdbg_set_logfunc(pdbgLogCallback);

	static const uint16_t ODYSSEY_CHIP_ID = 0x60C0;    
	struct pdbg_target *target;
    pdbg_for_each_target("ocmb", NULL, target)
    {
        uint32_t chipId = 0;
        pdbg_target_get_attribute(target, "ATTR_CHIP_ID", 4, 1, &chipId);
    	uint32_t proc = pdbg_target_index(pdbg_target_parent("proc", target));
	    if(chipId == ODYSSEY_CHIP_ID)
	    { 
			if(pdbg_target_probe(target) != PDBG_TARGET_ENABLED)
			{
				std::cout << "ocmb target is not enabled " << std::endl;
				continue;
			}
			
			
			std::cout << "ocmb target is enabled " << std::endl;
			struct pdbg_target *fsi = get_ody_fsi_target(target);
			std::cout << "probe fsi target " << std::endl;
			/*
			if(pdbg_target_probe(fsi) != PDBG_TARGET_ENABLED)
			{
				std::cout << "failed to probe fsi target " << std::endl;
				continue;
			}
			*/
			std::cout << "fsi target is enabled " << std::endl;			
			enum sbe_state state;
			if(sbe_ody_get_state(fsi, &state)){
				std::cout << "failed to get ody sbe state " << std::endl;
			}
			std::cout << "get ody sbe state value is " << state << std::endl;
			if(sbe_ody_set_state(fsi, state)){
				std::cout << "failed to set ody sbe state " << std::endl;
			}
			std::cout << "successfully set ody sbe state value " << std::endl;
			
		}
	}
   	return 0;
}
