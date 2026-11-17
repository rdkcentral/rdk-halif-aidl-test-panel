#include "vts_panel_init.h"

#include <binder/IServiceManager.h>
#include <ut_log.h>
#include <utils/String16.h>

#include "com/rdk/hal/panel/IPanelOutput.h"
#include "vts_panel_helper_functions.h"
#include "vts_panel_parse_hfp.h"

android::sp<IPanelOutput> gPanelOutput;

UT_status_t VTS_PANEL_init()
{
    gPanelOutput = android::interface_cast<IPanelOutput>(
        android::defaultServiceManager()->getService(android::String16(IPanelOutput::serviceName().c_str())));
    if (gPanelOutput == nullptr)
    {
        UT_LOG_ERROR("Panel Output service not available");
        return UT_STATUS_FAILURE;
    }
    if (vts_panel_parse_hfp() != UT_STATUS_OK)
    {
        UT_LOG_ERROR("Failed to process the Panel HFP file");
        return UT_STATUS_FAILURE;
    }
    return UT_STATUS_OK;
}

void VTS_PANEL_destroy()
{
    gPanelOutput.clear();
}
