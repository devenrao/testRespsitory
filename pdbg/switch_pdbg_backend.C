#include <libpdbg.h>
#include <vector>
#include <iostream>
#include <string.h>

#include <attributes_info.H>
//#include <ekb/chips/p10/procedures/hwp/perv/p10_extract_sbe_rc.H>
//#include <ekb/hwpf/fapi2/include/return_code_defs.H>

void pdbgLogCallback(int, const char* fmt, va_list ap)
{
    va_list vap;
    va_copy(vap, ap);
    std::vector<char> logData(1 + std::vsnprintf(nullptr, 0, fmt, ap));
    std::vsnprintf(logData.data(), logData.size(), fmt, vap);
    va_end(vap);
    std::string logstr(logData.begin(), logData.end());
    std::cout << logstr << std::endl;
}
static bool flag = false;
void myGetScom(enum pdbg_backend backend)
{
    // initially set the pdbg backend to sbefifo
    if (!pdbg_set_backend(backend, "p10"))
    {
        std::cout << "Failed to set pdbg backend to " << backend << std::endl;
    }

    constexpr auto devtree = "/var/lib/phosphor-software-manager/pnor/rw/DEVTREE";
    // PDBG_DTB environment variable set to CEC device tree path
    if (setenv("PDBG_DTB", devtree, 1)) 
    {   
        std::cout << "Failed to set PDBG_DTB: " << strerror(errno) << std::endl;
    }   

    constexpr auto PDATA_INFODB_PATH = "/usr/share/pdata/attributes_info.db";
    // PDATA_INFODB environment variable set to attributes tool  infodb path
    if (setenv("PDATA_INFODB", PDATA_INFODB_PATH, 1)) 
    {   
        std::cout << "Failed to set PDATA_INFODB: ({})" << strerror(errno) << std::endl;
    }   

    //initialize the targeting system 
    if (!pdbg_targets_init(NULL))
    {   
        std::cout << "pdbg_targets_init failed" << std::endl;
    }
    if(!flag) 
    {
        pdbg_target_probe_all(pdbg_target_root());
        flag = true;
    }
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
    struct pdbg_target* procTarget;
    pdbg_for_each_class_target("proc", procTarget)
    {
        static const uint64_t addr = 0x10012ull;
        fapi2::buffer<uint64_t> l_data64;
        std::cout << " before devender0113 proc scom data received is " << std::hex <<  l_data64 << std::endl;
        int ret = getScom(static_cast<const fapi2::Target<fapi2::TARGET_TYPE_PROC_CHIP> >(procTarget), addr, l_data64);
        std::cout << ret << " after devender0113 proc scom data received is " << std::hex <<  l_data64 << std::endl;
        return;
    }
}

int main()
{
    // set log level and callback function
    pdbg_set_loglevel(PDBG_ERROR);
    pdbg_set_logfunc(pdbgLogCallback);

    // initially set the pdbg backend to sbefifo
    std::cout << "DEVENDER0114 *****before GetScom with KERNEL as backend ****"<< std::endl;
    myGetScom(PDBG_BACKEND_KERNEL);
    std::cout << "DEVENDER0114 *****after GetScom with KERNEL as backend ****"<< std::endl;

    //std::cout << "Reset the device tree root " << std::endl;
    pdbg_reset_dt_root();

    std::cout << "****DEVENDER0114 before GetScom with SBEFIFO as backend ***"<< std::endl;
    myGetScom(PDBG_BACKEND_SBEFIFO);
    std::cout << "****DEVENDER0114 before GetScom with SBEFIFO as backend ***"<< std::endl;
    //std::cout << "Reset the device tree root " << std::endl;
   // pdbg_reset_dt_root();

    //std::cout << "***GetScom with SBEFIFO as backend ****"<< std::endl;
    //myGetScom(PDBG_BACKEND_SBEFIFO);
    return 1;
}
