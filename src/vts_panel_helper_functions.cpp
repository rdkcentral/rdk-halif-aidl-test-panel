#include "vts_panel_helper_functions.h"

using android::binder::Status;
using com::rdk::hal::videodecoder::DynamicRange;

android::sp<IPanelOutputController> panelOpen(const android::sp<PanelOutputControllerListener>& listener)
{
    android::sp<IPanelOutputController> controller;
    const Status status = gPanelOutput->open(listener, &controller);
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_TRUE(controller != nullptr);
    return controller;
}

void panelClose(const android::sp<IPanelOutputController>& controller)
{
    bool closed = false;
    const Status status = gPanelOutput->close(controller, &closed);
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_TRUE(closed);
}

void panelStart(const android::sp<IPanelOutputController>& controller)
{
    bool started = false;
    const Status status = controller->start(&started);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_TRUE_FATAL(started);
}

void panelStop(const android::sp<IPanelOutputController>& controller)
{
    bool stopped = false;
    const Status status = controller->stop(&stopped);
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_TRUE(stopped);
}

PictureModeConfiguration panelFirstPictureMode(const Capabilities& capabilities)
{
    UT_ASSERT_FALSE(capabilities.pictureModeCapabilities.empty());
    if (capabilities.pictureModeCapabilities.empty())
    {
        return {};
    }
    const auto& pictureMode = capabilities.pictureModeCapabilities.front();
    UT_ASSERT_FALSE(pictureMode.dynamicRangeCapabilities.empty());
    if (pictureMode.dynamicRangeCapabilities.empty())
    {
        return {};
    }
    const auto& dynamicRange = pictureMode.dynamicRangeCapabilities.front();
    UT_ASSERT_FALSE(dynamicRange.supportedAVSources.empty());
    if (dynamicRange.supportedAVSources.empty())
    {
        return {};
    }
    PictureModeConfiguration configuration;
    configuration.pictureMode = pictureMode.pictureMode;
    configuration.dynamicRange = dynamicRange.dynamicRange;
    configuration.source = dynamicRange.supportedAVSources.front();
    return configuration;
}

PQParameterSelector panelFirstPQSelector(const Capabilities& capabilities,
    const PictureModeConfiguration& pictureMode)
{
    UT_ASSERT_FALSE(capabilities.supportedPQParameters.empty());
    if (capabilities.supportedPQParameters.empty())
    {
        return {};
    }
    PQParameterSelector selector;
    selector.pqParameter = capabilities.supportedPQParameters.front();
    selector.pictureMode = pictureMode.pictureMode;
    selector.source = pictureMode.source;
    selector.dynamicRange = pictureMode.dynamicRange;
    return selector;
}

bool panelHasSourceType(const std::vector<PanelSource>& sources, const std::string& type)
{
    return std::any_of(sources.begin(), sources.end(), [&type](const PanelSource& source) {
        return source.type == type;
    });
}
