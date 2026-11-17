#include "PanelOutput.h"

#include <algorithm>

using android::binder::Status;
using com::rdk::hal::videodecoder::DynamicRange;

namespace com::rdk::hal::panel
{
namespace
{
PanelSource makeSource(const std::string& type)
{
    PanelSource source;
    source.type = type;
    return source;
}

bool hasRefreshRate(const Capabilities& capabilities, double refreshRateHz)
{
    return std::find(capabilities.supportedRefreshRatesHz.begin(),
        capabilities.supportedRefreshRatesHz.end(), refreshRateHz) !=
        capabilities.supportedRefreshRatesHz.end();
}
}

PanelOutput::PanelOutput()
{
    mVideoSourceOverride = makeSource("AUTO");
    const PanelSource ipSource = makeSource("IP");
    const PanelSource tunerSource = makeSource("TUNER");
    const PanelSource hdmiSource = makeSource("HDMI");
    const PanelSource compositeSource = makeSource("COMPOSITE");
    Capabilities::PictureModeCapabilities::DynamicRangeCapabilities dynamicRangeCapabilities;
    dynamicRangeCapabilities.dynamicRange = DynamicRange::SDR;
    dynamicRangeCapabilities.supportedAVSources = {ipSource, tunerSource, hdmiSource, compositeSource};

    Capabilities::PictureModeCapabilities pictureModeCapabilities;
    pictureModeCapabilities.pictureMode = "Entertainment";
    pictureModeCapabilities.dynamicRangeCapabilities = {dynamicRangeCapabilities};

    mCapabilities.panelType = "LCD";
    mCapabilities.pixelWidth = 3840;
    mCapabilities.pixelHeight = 2160;
    mCapabilities.widthCm = 124;
    mCapabilities.heightCm = 71;
    mCapabilities.frameRateMatchingSupported = true;
    mCapabilities.supportedPQParameters = {
        PQParameter::BRIGHTNESS, PQParameter::CONTRAST, PQParameter::SHARPNESS,
        PQParameter::SATURATION, PQParameter::HUE, PQParameter::MANUAL_BACKLIGHT,
        PQParameter::SDR_GAMMA, PQParameter::COLOR_TEMPERATURE, PQParameter::DIMMING_SETTINGS,
        PQParameter::LOW_LATENCY_STATE, PQParameter::MEMC, PQParameter::LOCAL_CONTRAST_LEVEL,
        PQParameter::MPEG_NOISE_REDUCTION, PQParameter::NOISE_REDUCTION, PQParameter::AI_PQ_ENGINE,
        PQParameter::AMBIENT_LIGHT_SENSOR_CONTROL, PQParameter::PRECISION_DETAIL,
        PQParameter::DV_CALIBRATION, PQParameter::TWO_POINT_WB, PQParameter::MULTI_POINT_WB,
        PQParameter::COLOUR_MANAGEMENT_SETTINGS, PQParameter::DYNAMIC_CONTRAST_LEVEL,
        PQParameter::TONE_MAPPING_SETTINGS};
    mCapabilities.supportedRefreshRatesHz = {
        48.0, 49.0, 50.0, 51.0, 52.0, 53.0, 54.0, 55.0, 56.0, 57.0, 58.0, 59.0, 60.0,
        75.0, 85.0, 100.0, 120.0, 144.0, 165.0, 240.0};
    mCapabilities.supportedColourTemperatures = {"WARM", "NORMAL", "COLD", "SUPER_COLD", "CUSTOM"};
    mCapabilities.supportedAVSources = {ipSource, tunerSource, hdmiSource, compositeSource};
    mCapabilities.supportedDynamicRanges = {
        DynamicRange::SDR, DynamicRange::HLG, DynamicRange::HDR10, DynamicRange::DOLBY_VISION};
    mCapabilities.pictureModeCapabilities = {pictureModeCapabilities};

    mPictureMode.pictureMode = "Entertainment";
    mPictureMode.dynamicRange = DynamicRange::SDR;
    mPictureMode.source = ipSource;
}

Status PanelOutput::getCapabilities(Capabilities* capabilities)
{
    if (capabilities == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mMutex);
    *capabilities = mCapabilities;
    return Status::ok();
}

Status PanelOutput::open(const android::sp<IPanelOutputControllerListener>& listener,
    android::sp<IPanelOutputController>* controller)
{
    if (listener == nullptr || controller == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mMutex);
    if (mController != nullptr)
    {
        return Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
    }
    mController = new PanelOutputController(*this);
    *controller = mController;
    return Status::ok();
}

Status PanelOutput::close(const android::sp<IPanelOutputController>& controller, bool* result)
{
    if (controller == nullptr || result == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mMutex);
    if (mState != State::STOPPED && mState != State::ERROR)
    {
        return Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
    }
    *result = controller == mController;
    if (*result)
    {
        mController.clear();
    }
    return Status::ok();
}

Status PanelOutput::registerListener(const android::sp<IPanelOutputEventListener>& listener, bool* result)
{
    if (listener == nullptr || result == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mMutex);
    const auto existing = std::find(mEventListeners.begin(), mEventListeners.end(), listener);
    *result = existing == mEventListeners.end();
    if (*result)
    {
        mEventListeners.push_back(listener);
    }
    return Status::ok();
}

Status PanelOutput::unregisterListener(const android::sp<IPanelOutputEventListener>& listener, bool* result)
{
    if (listener == nullptr || result == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mMutex);
    const auto existing = std::find(mEventListeners.begin(), mEventListeners.end(), listener);
    *result = existing != mEventListeners.end();
    if (*result)
    {
        mEventListeners.erase(existing);
    }
    return Status::ok();
}

PanelOutputController::PanelOutputController(PanelOutput& output) : mOutput(output) {}

Status PanelOutputController::requireStarted() const
{
    return mOutput.mState == State::STARTED ? Status::ok() :
        Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
}

Status PanelOutputController::start(bool* result)
{
    if (result == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    if (mOutput.mState != State::STOPPED)
    {
        return Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
    }
    mOutput.mState = State::STARTED;
    *result = true;
    return Status::ok();
}

Status PanelOutputController::getState(State* state)
{
    if (state == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    *state = mOutput.mState;
    return Status::ok();
}

Status PanelOutputController::stop(bool* result)
{
    if (result == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    if (mOutput.mState != State::STARTED)
    {
        return Status::fromExceptionCode(Status::EX_ILLEGAL_STATE);
    }
    mOutput.mState = State::STOPPED;
    *result = true;
    return Status::ok();
}

Status PanelOutputController::setEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (status.isOk())
    {
        mOutput.mEnabled = enabled;
    }
    return status;
}

Status PanelOutputController::getEnabled(bool* enabled)
{
    if (enabled == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (status.isOk())
    {
        *enabled = mOutput.mEnabled;
    }
    return status;
}

Status PanelOutputController::setPictureModes(
    const std::vector<PictureModeConfiguration>& configurations, bool* result)
{
    if (result == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (!status.isOk())
    {
        return status;
    }
    if (configurations.empty())
    {
        return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
    }
    mOutput.mPictureMode = configurations.front();
    *result = true;
    return Status::ok();
}

Status PanelOutputController::getPictureModes(const std::vector<PictureModeConfiguration>& requested,
    std::vector<PictureModeConfiguration>* returned, bool* result)
{
    if (result == nullptr || returned == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (!status.isOk())
    {
        return status;
    }
    if (requested.empty())
    {
        return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
    }
    *returned = requested;
    for (auto& configuration : *returned)
    {
        configuration.pictureMode = mOutput.mPictureMode.pictureMode;
    }
    *result = true;
    return Status::ok();
}

Status PanelOutputController::getDefaultPictureModes(const std::vector<PictureModeConfiguration>& requested,
    std::vector<PictureModeConfiguration>* defaults, bool* result)
{
    return getPictureModes(requested, defaults, result);
}

Status PanelOutputController::setPQParameters(
    const std::vector<PQParameterConfiguration>& configurations, ErrorCode* result)
{
    if (result == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (!status.isOk())
    {
        return status;
    }
    if (configurations.empty())
    {
        return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
    }
    mOutput.mPQParameters = configurations;
    *result = ErrorCode::SUCCESS;
    return Status::ok();
}

Status PanelOutputController::getPQParameters(const std::vector<PQParameterSelector>& requested,
    std::vector<PQParameterConfiguration>* returned, bool* result)
{
    if (result == nullptr || returned == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (!status.isOk())
    {
        return status;
    }
    if (requested.empty())
    {
        return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
    }
    returned->clear();
    for (const auto& selector : requested)
    {
        PQParameterConfiguration configuration;
        configuration.selector = selector;
        returned->push_back(configuration);
    }
    *result = true;
    return Status::ok();
}

Status PanelOutputController::getDefaultPQParameters(const std::vector<PQParameterSelector>& requested,
    std::vector<PQParameterConfiguration>* defaults, bool* result)
{
    return getPQParameters(requested, defaults, result);
}

Status PanelOutputController::getPQParameterCapabilities(
    PQParameter parameter, PQParameterCapabilities* capabilities)
{
    if (capabilities == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (!status.isOk())
    {
        return status;
    }
    capabilities->pqParameter = parameter;
    capabilities->isSupported = std::find(mOutput.mCapabilities.supportedPQParameters.begin(),
        mOutput.mCapabilities.supportedPQParameters.end(), parameter) !=
        mOutput.mCapabilities.supportedPQParameters.end();
    capabilities->isGlobal = false;
    return Status::ok();
}

Status PanelOutputController::setRefreshRate(double refreshRateHz, bool* result)
{
    if (result == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (!status.isOk())
    {
        return status;
    }
    *result = hasRefreshRate(mOutput.mCapabilities, refreshRateHz);
    if (*result)
    {
        mOutput.mRefreshRateHz = refreshRateHz;
    }
    return Status::ok();
}

Status PanelOutputController::getRefreshRate(double* refreshRateHz)
{
    if (refreshRateHz == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (status.isOk())
    {
        *refreshRateHz = mOutput.mRefreshRateHz;
    }
    return status;
}

Status PanelOutputController::setFrameRateMatching(bool enabled, bool* result)
{
    if (result == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (!status.isOk())
    {
        return status;
    }
    mOutput.mFrameRateMatching = enabled;
    *result = true;
    return Status::ok();
}

Status PanelOutputController::getFrameRateMatching(bool* enabled)
{
    if (enabled == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (status.isOk())
    {
        *enabled = mOutput.mFrameRateMatching;
    }
    return status;
}

Status PanelOutputController::setVideoSourceOverride(const PanelSource& source)
{
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (status.isOk())
    {
        mOutput.mVideoSourceOverride = source;
    }
    return status;
}

Status PanelOutputController::getVideoSourceOverride(PanelSource* source)
{
    if (source == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (status.isOk())
    {
        *source = mOutput.mVideoSourceOverride;
    }
    return status;
}

Status PanelOutputController::getVideoSource(PanelSource* source)
{
    return getVideoSourceOverride(source);
}

Status PanelOutputController::getDynamicRange(DynamicRange* dynamicRange)
{
    if (dynamicRange == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (status.isOk())
    {
        *dynamicRange = DynamicRange::SDR;
    }
    return status;
}

Status PanelOutputController::getVideoFrameRate(std::array<int32_t, 2>* frameRate)
{
    if (frameRate == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (status.isOk())
    {
        *frameRate = {0, 0};
    }
    return status;
}

Status PanelOutputController::getVideoResolution(std::array<int32_t, 2>* resolution)
{
    if (resolution == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (status.isOk())
    {
        *resolution = {0, 0};
    }
    return status;
}

Status PanelOutputController::enableCalibrationMode(bool enabled, bool* result)
{
    if (result == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (!status.isOk())
    {
        return status;
    }
    *result = mOutput.mCalibrationMode != enabled;
    if (*result)
    {
        mOutput.mCalibrationMode = enabled;
    }
    return Status::ok();
}

Status PanelOutputController::getCalibrationMode(bool* enabled)
{
    if (enabled == nullptr)
    {
        return Status::fromExceptionCode(Status::EX_NULL_POINTER);
    }
    std::lock_guard<std::mutex> lock(mOutput.mMutex);
    const Status status = requireStarted();
    if (status.isOk())
    {
        *enabled = mOutput.mCalibrationMode;
    }
    return status;
}
}
