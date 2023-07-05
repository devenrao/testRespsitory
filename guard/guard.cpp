#include <libguard/guard_interface.hpp>
#include <iostream>
using ::openpower::guard::GuardRecords;

int main()
{
    openpower::guard::libguard_init();
    openpower::guard::GuardRecords records = openpower::guard::getAll(true);
    GuardRecords unresolvedRecords;
    // filter out all unused or resolved records
    for (const auto& elem : records)
    {
        if (elem.recordId != GUARD_RESOLVED)
        {
            unresolvedRecords.emplace_back(elem);
        }
    }

    for (const auto& elem : unresolvedRecords)
    {    
        auto physicalPath =
            openpower::guard::getPhysicalPath(elem.targetId);
        if (!physicalPath.has_value())
        {
            std::cout << "Failed to get physical path for record "  << elem.recordId << std::endl;
        }
        else
        {
            std::cout << "physical path value is " << *physicalPath << std::endl;
        }
    }
    return 0;
}

