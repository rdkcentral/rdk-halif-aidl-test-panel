#ifndef __VTS_PANEL_PARSE_HFP_H__
#define __VTS_PANEL_PARSE_HFP_H__

#include <string>
#include <vector>

#include <ut.h>
#include <ut_kvp_profile.h>

#include "com/rdk/hal/panel/PQParameter.h"
#include "com/rdk/hal/panel/PanelSource.h"
#include "com/rdk/hal/videodecoder/DynamicRange.h"

struct panelHFP
{
    std::string panelType;
    int32_t pixelWidth = 0;
    int32_t pixelHeight = 0;
    int32_t widthCm = 0;
    int32_t heightCm = 0;
    bool frameRateMatchingSupported = false;
    std::vector<com::rdk::hal::panel::PQParameter> supportedPQParameters;
    std::vector<double> supportedRefreshRatesHz;
    // HFP source instances are optional. L1 capability validation is by source
    // family because an implementation may expose a concrete instance only when
    // that input is physically present.
    std::vector<com::rdk::hal::panel::PanelSource> supportedAVSources;
    std::vector<com::rdk::hal::videodecoder::DynamicRange> supportedDynamicRanges;
    std::vector<std::string> supportedColourTemperatures;
};

extern panelHFP gPanel_hfp;

UT_status_t vts_panel_parse_hfp();

#endif
