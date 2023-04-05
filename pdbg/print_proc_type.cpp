#include <libpdbg.h>
#include <vector>
#include <iostream>
#include <string.h>

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

uint8_t getTrgtType(pdbg_target* i_trgt)
{
    uint8_t attr = 0;
    pdbg_target_get_attribute(i_trgt, "ATTR_TYPE", 1, 1, &attr);
    return attr;
}

pdbg_target* getPibTrgt(pdbg_target* i_procTrgt)
{
    // The input target must be a processor.
    std::cout << "TYPE_PROC " << "getTrgtType " 
        << static_cast<uint16_t>(getTrgtType(i_procTrgt)) << std::endl;

    char path[16];
    sprintf(path, "/proc%d/pib", pdbg_target_index(i_procTrgt));

    // Return the pib target.
    pdbg_target* pibTrgt = pdbg_target_from_path(nullptr, path);
    std::cout << "path is " << path << std::endl;

    return pibTrgt;
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


    //initialize the targeting system 
    if (!pdbg_targets_init(NULL))
    {   
        std::cout << "pdbg_targets_init failed" << std::endl;
        return 0;
    }

    // set log level and callback function
    pdbg_set_loglevel(PDBG_DEBUG);
    pdbg_set_logfunc(pdbgLogCallback);

    struct pdbg_target* procTarget;
    pdbg_for_each_class_target("proc", procTarget)
    {
        auto index = std::to_string(pdbg_target_index(procTarget));
        getPibTrgt(procTarget);
    }

    const char* myproc = "/proc0/pib";
    struct pdbg_target* pdbgTgt = pdbg_target_from_path(NULL, myproc);
    if(pdbgTgt == nullptr)
    {
        std::cerr << "Could not find pdbg target for path " << myproc << std::endl;
        return 0;
    }
    std::cout << "target name is " << pdbg_target_name(pdbgTgt) << std::endl;
    std::cout << "target path is " << pdbg_target_path(pdbgTgt) << std::endl;

    return 1;
}
