//This application uses test data writes to a file and invokes
//D-Bus method CreatePELWithFFDCFiles
//This application is used to validate logging/sbe_ffdc_handler code

extern "C"
{
#include "libpdbg.h"
}
#include<iplfacade.H>
int main()
{
    constexpr auto devtree = "/tmp/DEVTREE";

    // PDBG_DTB environment variable set to CEC device tree path
    if (setenv("PDBG_DTB", devtree, 1))
    {
        std::cerr << "Failed to set PDBG_DTB: " << strerror(errno) << std::endl;
        return 0;
    }

    //initialize the targeting system 
    if (!pdbg_targets_init(NULL))
    {   
        std::cerr << "pdbg_targets_init failed" << std::endl;
        return 0;
    }
    pdbg_set_backend(PDBG_BACKEND_SBEFIFO, NULL);

    struct pdbg_target *hubchip;
    pdbg_for_each_target("hubchip", NULL, hubchip)
    {
        std::cout << "hubchip found " << std::endl;	
		pdbg_target_probe(hubchip);
		if (pdbg_target_status(hubchip) != PDBG_TARGET_ENABLED)
		{
			std::cout << "hubchip chip not enabled " << std::endl;	
			return -1;
		}
        int ret =  ps_sppe_config_update(hubchip);
        if (ret == 0)
        {
            std::cout << "succes in call to HWP ps_sppe_config_update " << std::endl;
        }
		break;
    }

    sleep(100);
    return 0;
}

