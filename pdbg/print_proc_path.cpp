#include <libpdbg.h>
#include <vector>
#include <iostream>
#include <string.h>
#include <attributes_info.H>

void pdbgLogCallback(int, const char* fmt, va_list ap)
{
    va_list vap;
    va_copy(vap, ap);
    std::vector<char> logData(1 + std::vsnprintf(nullptr, 0, fmt, ap));
    std::vsnprintf(logData.data(), logData.size(), fmt, vap);
    va_end(vap);
    std::string logstr(logData.begin(), logData.end());
    std::cerr << "PDBG:" << logstr << std::endl;
}

static int compare(struct pdbg_target *target, void *priv)
{
    ATTR_PHYS_DEV_PATH_Type phyPath;
    const char* guardPath = (const char *)priv;
    if (!DT_GET_PROP(ATTR_PHYS_DEV_PATH, target, phyPath))
    {
        if(strcmp(phyPath, guardPath) == 0)
        {
            const char* path = pdbg_target_path(target);
            printf("%s\n", path);
            printf("devender guard tgt phsical path %s\n", guardPath);
            std::cout << "Type = " << pdbg_target_name(target) << std::endl;
            ATTR_HWAS_STATE_Type hwasState;
            if (!DT_GET_PROP(ATTR_HWAS_STATE, target, hwasState))
            {
                if( hwasState.functional )
                {
                    std::cout << "CONFIGURED" << std::endl;
                }
                else
                {
                    std::cout << "DECONFIGURED" << std::endl;
                } 
                std::cout << "EID" << hwasState.deconfiguredByEid << std::endl;
                if(hwasState.functionalOverride)
                {
                  std::cout << "FCO override " << std::endl;
                }
            }
            ATTR_LOCATION_CODE_Type attrLocCode;
            if (!DT_GET_PROP(ATTR_LOCATION_CODE, target, attrLocCode))
            {
                std::cerr << "ATTR_LOCATION_CODE " << attrLocCode << std::endl;
            }
        }
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

    // set log level and callback function
    pdbg_set_loglevel(PDBG_DEBUG);
    pdbg_set_logfunc(pdbgLogCallback);
    char guardTgt[] = "physical:sys-0/node-0/proc-1";
    pdbg_target_traverse(nullptr, compare, guardTgt);
    return 1;
}
