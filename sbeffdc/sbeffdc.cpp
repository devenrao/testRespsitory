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

struct pdbg_target *getPibTarget(struct pdbg_target *proc)
{
    char path[16];
    sprintf(path, "/proc%d/pib", pdbg_target_index(proc));
    struct pdbg_target *pib = pdbg_target_from_path(nullptr, path);
    if (pib == nullptr) {
        std::cout << "error to get pib target" << std::endl;
        return NULL;
    }
    if (pdbg_target_probe(pib) != PDBG_TARGET_ENABLED) {
        std::cout << "error to probe pib target" << std::endl;
    }
    return pib;
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

    struct pdbg_target *proc;
    pdbg_for_each_target("proc", NULL, proc)
    {
        if (pdbg_target_probe(proc) != PDBG_TARGET_ENABLED)
        {
            std::cout << "proc chip not enabled " << std::endl;
            return -1;
        }
        struct pdbg_target *pib = getPibTarget(proc);
        std::cout << "sbedump - before calling sbe_ffdc_get " << std::endl;
        DumpDataPtr ffdcPtr;
        uint32_t status = 0;
        uint32_t ffdc_len = 0;
        int ret = sbe_ffdc_get(pib, &status, ffdcPtr.getPtr(), &ffdc_len);
        if(ret != 0){
            std::cout << "failed to get ffdc data " << ret << std::endl;
        } else {
            std::cout << "ffdc data status is " << status << " ffdc_len " << ffdc_len << std::endl;
        }
        std::cout << "sbedump - after calling sbe_ffdc_get " << std::endl;
        break;

    }
    struct pdbg_target *ocmb;
    pdbg_for_each_target("ocmb", NULL, ocmb)
    {
	    if(is_ody_ocmb_chip(ocmb))
	    {
			if (pdbg_target_probe(ocmb) != PDBG_TARGET_ENABLED)
			{
				std::cout << "ocmb chip not enabled " << std::endl;
				return -1;
			}

            std::cout << "sbedump - before calling sbe_ffdc_get " << std::endl;
			DumpDataPtr ffdcPtr;
			uint32_t status = 0;
			uint32_t ffdc_len = 0;
			int ret = sbe_ffdc_get(ocmb, &status, ffdcPtr.getPtr(), &ffdc_len);
			if(ret != 0){
				std::cout << "failed to get ffdc data " << ret << std::endl;
			} else {
				std::cout << "ffdc data status is " << status << " ffdc_len " << ffdc_len << std::endl;
			}
            std::cout << "sbedump - after calling sbe_ffdc_get " << std::endl;
			break;
	    }
    }
    return 0;
}

