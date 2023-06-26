#include "xyz/openbmc_project/Common/error.hpp"
#include <phosphor-logging/elog-errors.hpp>
#include <phosphor-logging/log.hpp>

using namespace phosphor::logging;

int main()
{
    using InternalFailure =
        sdbusplus::xyz::openbmc_project::Common::Error::InternalFailure;
    report<InternalFailure>(); 
    return 0;
}

