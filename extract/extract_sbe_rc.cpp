#include <libpdbg.h>
#include <vector>
#include <iostream>
#include <string.h>

#include <attributes_info.H>
#include <ekb/chips/ocmb/odyssey/procedures/hwp/perv/ody_extract_sbe_rc.H>
#include <ekb/hwpf/fapi2/include/return_code_defs.H>

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

int main()
{
    constexpr auto devtree = "/var/lib/phosphor-software-manager/pnor/rw/DEVTREE";

    // PDBG_DTB environment variable set to CEC device tree path
    if (setenv("PDBG_DTB", devtree, 1))
    {
        std::cout << "Failed to set PDBG_DTB: " << strerror(errno) << std::endl;
        return 0;
    }

    constexpr auto PDATA_INFODB_PATH = "/usr/share/pdata/attributes_info.db";
    // PDATA_INFODB environment variable set to attributes tool  infodb path
    if (setenv("PDATA_INFODB", PDATA_INFODB_PATH, 1))
    {
        std::cout << "Failed to set PDATA_INFODB: ({})" << strerror(errno) << std::endl;
        return 0;
    }

    // initially set the pdbg backend to sbefifo
    std::cout << "***Start with SBEFIFO as backend ******" << std::endl;
    if (!pdbg_set_backend(PDBG_BACKEND_SBEFIFO, NULL)) 
    {
        std::cout << "Failed to set pdbg backend to fifo" << std::endl;
    }

    //initialize the targeting system 
    if (!pdbg_targets_init(NULL))
    {   
        std::cout << "pdbg_targets_init failed" << std::endl;
        return 0;
    }

    // set log level and callback function
    pdbg_set_loglevel(PDBG_DEBUG);
    pdbg_set_logfunc(pdbgLogCallback);

    // switch the pdbg backend to kernel
    std::cout << "***Start with KERNEL as backend ******" << std::endl;
    if (!pdbg_set_backend(PDBG_BACKEND_KERNEL, NULL)) 
    {
        std::cout << "Failed to set pdbg backend to kernel" << std::endl;
    }


	static const uint16_t ODYSSEY_CHIP_ID = 0x60C0;    
	struct pdbg_target *target;
    pdbg_for_each_target("ocmb", NULL, target)
    {
        uint32_t chipId = 0;
        pdbg_target_get_attribute(target, "ATTR_CHIP_ID", 4, 1, &chipId);
    	uint32_t proc = pdbg_target_index(pdbg_target_parent("proc", target));
	    if(chipId == ODYSSEY_CHIP_ID)
	    { 
			// Execute SBE extract rc to set up sdb bit for pibmem dump to work
			// TODO Add error handling or revisit procedure later
			fapi2::ReturnCode fapiRc;

			// p10_extract_sbe_rc is returning the error along with
			// recovery action, so not checking the fapirc.
			fapiRc = ody_extract_sbe_rc(target);
			printf("p10_extract_sbe_rc for ocmb=%s returned rc=0x%08X and SBE "
				"Recovery Action=0x%08X",
				pdbg_target_path(target), fapiRc);
			break;
		}
	}
    return 1;
}
