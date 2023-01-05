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

    struct pdbg_target* procTarget;
    ATTR_HWAS_STATE_Type hwasState;
    pdbg_for_each_class_target("proc", procTarget)
    {
        auto index = std::to_string(pdbg_target_index(procTarget));
        std::cout << "Index of the proc target " << 
            pdbg_target_path(procTarget) << " is " << index << std::endl;
    }
    // switch the pdbg backend to kernel
    std::cout << "***Start with KERNEL as backend ******" << std::endl;
    if (!pdbg_set_backend(PDBG_BACKEND_KERNEL, NULL)) 
    {
        std::cout << "Failed to set pdbg backend to kernel" << std::endl;
    }
    return 1;
}
