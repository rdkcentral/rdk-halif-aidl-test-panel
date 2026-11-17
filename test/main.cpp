#include <ut.h>
#include <ut_log.h>

#include "vts_panel_init.h"

int main(int argc, char** argv)
{
    UT_init(argc, argv);
    UT_LOG_INFO("BUILD_TIMESTAMP: %s", BUILD_TIMESTAMP);
    UT_LOG_INFO("AIDL HALIF Version: %s", RDK_HALIF_AIDL_VERSION);
    UT_LOG_INFO("VTS Panel HALIF Test Version: %s", PANEL_VERSION);

    if (VTS_PANEL_init() != UT_STATUS_OK)
    {
        return UT_STATUS_FAILURE;
    }
    const UT_status_t result = UT_run_tests();
    VTS_PANEL_destroy();
    return result;
}
