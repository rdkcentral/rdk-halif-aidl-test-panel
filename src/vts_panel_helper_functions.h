#ifndef __VTS_PANEL_HELPER_FUNCTIONS_H__
#define __VTS_PANEL_HELPER_FUNCTIONS_H__

#include <binder/Status.h>
#include <ut.h>
#include <utils/StrongPointer.h>

#include "com/rdk/hal/panel/BnPanelOutputControllerListener.h"
#include "com/rdk/hal/panel/BnPanelOutputEventListener.h"
#include "com/rdk/hal/panel/IPanelOutput.h"
#include "com/rdk/hal/panel/IPanelOutputController.h"

using namespace com::rdk::hal::panel;

extern android::sp<IPanelOutput> gPanelOutput;

class PanelOutputControllerListener : public BnPanelOutputControllerListener
{
public:
    android::binder::Status onPictureModeChanged(const std::string& pictureMode) override;
    android::binder::Status onPQParameterChanged(const std::string& pictureMode, const PanelSource& source,
        com::rdk::hal::videodecoder::DynamicRange dynamicRange, PQParameter parameter,
        const PQParameterValue& value) override;
};

class PanelOutputEventListener : public BnPanelOutputEventListener
{
public:
    android::binder::Status onVideoSourceChanged(const PanelSource& source) override;
    android::binder::Status onDynamicRangeChanged(
        com::rdk::hal::videodecoder::DynamicRange dynamicRange) override;
    android::binder::Status onVideoFrameRateChanged(int32_t numerator, int32_t denominator) override;
    android::binder::Status onVideoResolutionChanged(int32_t width, int32_t height) override;
    android::binder::Status onRefreshRateChanged(double refreshRateHz) override;
    android::binder::Status onStateChanged(State oldState, State newState) override;
};

android::sp<IPanelOutputController> panelOpen(const android::sp<PanelOutputControllerListener>& listener);
void panelClose(const android::sp<IPanelOutputController>& controller);
void panelStart(const android::sp<IPanelOutputController>& controller);
void panelStop(const android::sp<IPanelOutputController>& controller);
PictureModeConfiguration panelFirstPictureMode(const Capabilities& capabilities);
PQParameterSelector panelFirstPQSelector(const Capabilities& capabilities,
    const PictureModeConfiguration& pictureMode);
bool panelHasSourceType(const std::vector<PanelSource>& sources, const std::string& type);

#endif
