#ifndef MOCK_PANEL_OUTPUT_H
#define MOCK_PANEL_OUTPUT_H

#include <binder/BinderService.h>
#include <array>
#include <mutex>
#include <vector>

#include "com/rdk/hal/panel/BnPanelOutput.h"
#include "com/rdk/hal/panel/BnPanelOutputController.h"

namespace com::rdk::hal::panel
{
class PanelOutput;

class PanelOutputController : public BnPanelOutputController
{
public:
    explicit PanelOutputController(PanelOutput& output);

    ::android::binder::Status start(bool* result) override;
    ::android::binder::Status getState(State* state) override;
    ::android::binder::Status stop(bool* result) override;
    ::android::binder::Status setEnabled(bool enabled) override;
    ::android::binder::Status getEnabled(bool* enabled) override;
    ::android::binder::Status setPictureModes(
        const std::vector<PictureModeConfiguration>& configurations, bool* result) override;
    ::android::binder::Status getPictureModes(const std::vector<PictureModeConfiguration>& requested,
        std::vector<PictureModeConfiguration>* returned, bool* result) override;
    ::android::binder::Status getDefaultPictureModes(const std::vector<PictureModeConfiguration>& requested,
        std::vector<PictureModeConfiguration>* defaults, bool* result) override;
    ::android::binder::Status setPQParameters(
        const std::vector<PQParameterConfiguration>& configurations, ErrorCode* result) override;
    ::android::binder::Status getPQParameters(const std::vector<PQParameterSelector>& requested,
        std::vector<PQParameterConfiguration>* returned, bool* result) override;
    ::android::binder::Status getDefaultPQParameters(const std::vector<PQParameterSelector>& requested,
        std::vector<PQParameterConfiguration>* defaults, bool* result) override;
    ::android::binder::Status getPQParameterCapabilities(
        PQParameter parameter, PQParameterCapabilities* capabilities) override;
    ::android::binder::Status setRefreshRate(double refreshRateHz, bool* result) override;
    ::android::binder::Status getRefreshRate(double* refreshRateHz) override;
    ::android::binder::Status setFrameRateMatching(bool enabled, bool* result) override;
    ::android::binder::Status getFrameRateMatching(bool* enabled) override;
    ::android::binder::Status setVideoSourceOverride(const PanelSource& source) override;
    ::android::binder::Status getVideoSourceOverride(PanelSource* source) override;
    ::android::binder::Status getVideoSource(PanelSource* source) override;
    ::android::binder::Status getDynamicRange(
        ::com::rdk::hal::videodecoder::DynamicRange* dynamicRange) override;
    ::android::binder::Status getVideoFrameRate(std::array<int32_t, 2>* frameRate) override;
    ::android::binder::Status getVideoResolution(std::array<int32_t, 2>* resolution) override;
    ::android::binder::Status enableCalibrationMode(bool enabled, bool* result) override;
    ::android::binder::Status getCalibrationMode(bool* enabled) override;

private:
    ::android::binder::Status requireStarted() const;
    PanelOutput& mOutput;
};

class PanelOutput : public ::android::BinderService<PanelOutput>, public BnPanelOutput
{
public:
    PanelOutput();
    static char const* getServiceName() { return IPanelOutput::serviceName().c_str(); }

    ::android::binder::Status getCapabilities(Capabilities* capabilities) override;
    ::android::binder::Status open(const ::android::sp<IPanelOutputControllerListener>& listener,
        ::android::sp<IPanelOutputController>* controller) override;
    ::android::binder::Status close(const ::android::sp<IPanelOutputController>& controller,
        bool* result) override;
    ::android::binder::Status registerListener(const ::android::sp<IPanelOutputEventListener>& listener,
        bool* result) override;
    ::android::binder::Status unregisterListener(const ::android::sp<IPanelOutputEventListener>& listener,
        bool* result) override;

    std::mutex mMutex;
    State mState = State::STOPPED;
    bool mEnabled = true;
    bool mFrameRateMatching = true;
    bool mCalibrationMode = false;
    double mRefreshRateHz = 60.0;
    PanelSource mVideoSourceOverride;
    PictureModeConfiguration mPictureMode;
    std::vector<PQParameterConfiguration> mPQParameters;

private:
    friend class PanelOutputController;
    Capabilities mCapabilities;
    ::android::sp<PanelOutputController> mController;
    std::vector<::android::sp<IPanelOutputEventListener>> mEventListeners;
};
}

#endif
