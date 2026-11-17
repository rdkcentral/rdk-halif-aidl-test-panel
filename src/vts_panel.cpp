#include "vts_panel_helper_functions.h"

#include <ut_log.h>

using android::binder::Status;
using com::rdk::hal::videodecoder::DynamicRange;

android::binder::Status PanelOutputControllerListener::onPictureModeChanged(const std::string& pictureMode)
{
    UT_LOG_INFO("PanelOutputControllerListener::onPictureModeChanged[%s]", pictureMode.c_str());
    return Status::ok();
}

android::binder::Status PanelOutputControllerListener::onPQParameterChanged(
    const std::string& pictureMode, const PanelSource&, DynamicRange, PQParameter parameter,
    const PQParameterValue&)
{
    UT_LOG_INFO("PanelOutputControllerListener::onPQParameterChanged pictureMode[%s] parameter[%d]",
        pictureMode.c_str(), static_cast<int>(parameter));
    return Status::ok();
}

android::binder::Status PanelOutputEventListener::onVideoSourceChanged(const PanelSource&)
{
    UT_LOG_INFO("PanelOutputEventListener::onVideoSourceChanged");
    return Status::ok();
}

android::binder::Status PanelOutputEventListener::onDynamicRangeChanged(DynamicRange)
{
    UT_LOG_INFO("PanelOutputEventListener::onDynamicRangeChanged");
    return Status::ok();
}

android::binder::Status PanelOutputEventListener::onVideoFrameRateChanged(int32_t numerator, int32_t denominator)
{
    UT_LOG_INFO("PanelOutputEventListener::onVideoFrameRateChanged[%d/%d]", numerator, denominator);
    return Status::ok();
}

android::binder::Status PanelOutputEventListener::onVideoResolutionChanged(int32_t width, int32_t height)
{
    UT_LOG_INFO("PanelOutputEventListener::onVideoResolutionChanged[%dx%d]", width, height);
    return Status::ok();
}

android::binder::Status PanelOutputEventListener::onRefreshRateChanged(double refreshRateHz)
{
    UT_LOG_INFO("PanelOutputEventListener::onRefreshRateChanged[%.2f]", refreshRateHz);
    return Status::ok();
}

android::binder::Status PanelOutputEventListener::onStateChanged(State oldState, State newState)
{
    UT_LOG_INFO("PanelOutputEventListener::onStateChanged[%d -> %d]",
        static_cast<int>(oldState), static_cast<int>(newState));
    return Status::ok();
}
