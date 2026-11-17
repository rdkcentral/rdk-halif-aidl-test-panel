#include "vts_panel.h"

#include <algorithm>
#include <array>

using com::rdk::hal::videodecoder::DynamicRange;

UT_ADD_TEST_TO_GROUP(VTS_L1_PANEL, UT_TESTS_L1)

class VTS_L1_PANEL : public UTCore
{
public:
    VTS_L1_PANEL() : UTCore() {}
    ~VTS_L1_PANEL() override = default;

    void SetUp() override
    {
        android::ProcessState::self()->startThreadPool();
        PANEL_GET_CAPABILITIES(mCapabilities);
    }

    void TearDown() override {}

protected:
    bool openStarted(android::sp<IPanelOutputController>& controller)
    {
        mControllerListener = new PanelOutputControllerListener();
        Status status = gPanelOutput->open(mControllerListener, &controller);
        if (!status.isOk() || controller == nullptr)
        {
            return false;
        }
        bool started = false;
        status = controller->start(&started);
        return status.isOk() && started;
    }

    void stopClose(const android::sp<IPanelOutputController>& controller)
    {
        bool stopped = false;
        PANEL_STOP_SUCCESS(controller, stopped);
        bool closed = false;
        PANEL_CLOSE_SUCCESS(controller, closed);
    }

    Capabilities mCapabilities;
    android::sp<PanelOutputControllerListener> mControllerListener;
};

/**
 * @brief Validates the Panel service capabilities against the HFP.
 *
 * This test ensures that the service reports the panel dimensions, feature
 * flags, PQ parameters, refresh rates, source families, dynamic ranges, and
 * colour temperatures declared by the platform HFP.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** Panel HAL is initialized and the Panel HFP is loaded.@n
 * **Dependencies:** gPanelOutput and gPanel_hfp must be valid.@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Step | Action | Input | Expected Result | Remarks |@n
 * |01|Call getCapabilities()|N/A|Capabilities are returned successfully|Mandatory step|@n
 * |02|Compare scalar properties|HFP panel properties|Returned values match the HFP|Panel identity|@n
 * |03|Validate advertised lists|HFP capability lists|All HFP values are supported|Capability coverage|@n
 */
UT_ADD_TEST(VTS_L1_PANEL, getCapabilities_pos)
{
    UT_ASSERT_TRUE(mCapabilities.panelType == gPanel_hfp.panelType);
    UT_ASSERT_EQUAL(mCapabilities.pixelWidth, gPanel_hfp.pixelWidth);
    UT_ASSERT_EQUAL(mCapabilities.pixelHeight, gPanel_hfp.pixelHeight);
    UT_ASSERT_EQUAL(mCapabilities.widthCm, gPanel_hfp.widthCm);
    UT_ASSERT_EQUAL(mCapabilities.heightCm, gPanel_hfp.heightCm);
    UT_ASSERT_EQUAL(mCapabilities.frameRateMatchingSupported, gPanel_hfp.frameRateMatchingSupported);
    for (const PQParameter parameter : gPanel_hfp.supportedPQParameters)
    {
        UT_ASSERT_TRUE(std::find(mCapabilities.supportedPQParameters.begin(),
            mCapabilities.supportedPQParameters.end(), parameter) != mCapabilities.supportedPQParameters.end());
    }
    for (const double refreshRateHz : gPanel_hfp.supportedRefreshRatesHz)
    {
        UT_ASSERT_TRUE(std::find(mCapabilities.supportedRefreshRatesHz.begin(),
            mCapabilities.supportedRefreshRatesHz.end(), refreshRateHz) !=
            mCapabilities.supportedRefreshRatesHz.end());
    }
    for (const auto& source : gPanel_hfp.supportedAVSources)
    {
        UT_ASSERT_TRUE(panelHasSourceType(mCapabilities.supportedAVSources, source.type));
    }
    for (const DynamicRange dynamicRange : gPanel_hfp.supportedDynamicRanges)
    {
        UT_ASSERT_TRUE(std::find(mCapabilities.supportedDynamicRanges.begin(),
            mCapabilities.supportedDynamicRanges.end(), dynamicRange) !=
            mCapabilities.supportedDynamicRanges.end());
    }
    for (const auto& colourTemperature : gPanel_hfp.supportedColourTemperatures)
    {
        UT_ASSERT_TRUE(std::find(mCapabilities.supportedColourTemperatures.begin(),
            mCapabilities.supportedColourTemperatures.end(), colourTemperature) !=
            mCapabilities.supportedColourTemperatures.end());
    }
}

/**
 * @brief Validates Panel event-listener registration lifecycle.
 *
 * This test ensures a listener is registered once, duplicate registration is
 * rejected by a false result, and the listener can be unregistered.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** Panel HAL is initialized.@n
 * **Dependencies:** gPanelOutput must be valid.@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Step | Action | Input | Expected Result | Remarks |@n
 * |01|Create an event listener|PanelOutputEventListener|Listener is valid|Mandatory step|@n
 * |02|Register listener|Listener|Registration succeeds|First registration|@n
 * |03|Register listener again|Same listener|Call succeeds and returns false|Duplicate handling|@n
 * |04|Unregister listener|Same listener|Unregistration succeeds|Cleanup|@n
 */
UT_ADD_TEST(VTS_L1_PANEL, registerAndUnregisterListener_pos)
{
    android::sp<PanelOutputEventListener> listener = new PanelOutputEventListener();
    bool registered = false;
    PANEL_REGISTER_EVENT_SUCCESS(listener, registered);

    Status status;
    registered = true;
    status = gPanelOutput->registerListener(listener, &registered);
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_FALSE(registered);

    bool unregistered = false;
    status = gPanelOutput->unregisterListener(listener, &unregistered);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_TRUE(unregistered);
}

/**
 * @brief Validates Panel controller lifecycle transitions.
 *
 * This test verifies that an opened controller begins in STOPPED, transitions
 * to STARTED after start(), and can be stopped and closed.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** Panel HAL is initialized and not exclusively open.@n
 * **Dependencies:** gPanelOutput must be valid.@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Step | Action | Input | Expected Result | Remarks |@n
 * |01|Open Panel output|Controller listener|Non-null controller returned|Exclusive control|@n
 * |02|Get initial state|Controller|State is STOPPED|Initial lifecycle state|@n
 * |03|Start controller|Controller|Start succeeds and state becomes STARTED|Lifecycle transition|@n
 * |04|Stop and close|Controller|Both operations succeed|Cleanup|@n
 */
UT_ADD_TEST(VTS_L1_PANEL, openStartGetStateStopClose_pos)
{
    mControllerListener = new PanelOutputControllerListener();
    android::sp<IPanelOutputController> controller;
    PANEL_OPEN_SUCCESS(mControllerListener, controller);

    State state;
    PANEL_GET_STATE(controller, state);
    UT_ASSERT_EQUAL_FATAL(state, State::STOPPED);

    bool started = false;
    PANEL_START_SUCCESS(controller, started);
    PANEL_GET_STATE(controller, state);
    UT_ASSERT_EQUAL(state, State::STARTED);
    stopClose(controller);
}

/**
 * @brief Validates invalid Panel controller lifecycle requests.
 *
 * This test verifies that stop() from STOPPED and start() from STARTED fail
 * with EX_ILLEGAL_STATE.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 004@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** Panel HAL is initialized and not exclusively open.@n
 * **Dependencies:** gPanelOutput must be valid.@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Step | Action | Input | Expected Result | Remarks |@n
 * |01|Open controller|Controller listener|Controller is returned in STOPPED state|Setup|@n
 * |02|Call stop()|STOPPED controller|EX_ILLEGAL_STATE|Invalid transition|@n
 * |03|Start controller and call start() again|STARTED controller|EX_ILLEGAL_STATE|Invalid transition|@n
 * |04|Stop and close|Controller|Cleanup succeeds|Cleanup|@n
 */
UT_ADD_TEST(VTS_L1_PANEL, startAndStopInvalidState_neg)
{
    mControllerListener = new PanelOutputControllerListener();
    android::sp<IPanelOutputController> controller;
    PANEL_OPEN_SUCCESS(mControllerListener, controller);

    bool stopped = true;
    PANEL_STOP_FAILURE(controller, stopped, Status::EX_ILLEGAL_STATE);

    bool started = false;
    PANEL_START_SUCCESS(controller, started);
    started = true;
    PANEL_START_FAILURE(controller, started, Status::EX_ILLEGAL_STATE);
    stopClose(controller);
}

/**
 * @brief Validates Panel enabled-state control.
 *
 * This test reads the enabled state, changes it, confirms the new value, and
 * restores the original state before closing the controller.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 005@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** Panel controller can be started.@n
 * **Dependencies:** Panel output must support controller lifecycle APIs.@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Step | Action | Input | Expected Result | Remarks |@n
 * |01|Open and start controller|N/A|Controller is STARTED|Setup|@n
 * |02|Get enabled state|Controller|Current state is returned|Baseline|@n
 * |03|Set inverse enabled state|Boolean inverse|Update succeeds|State change|@n
 * |04|Get and restore enabled state|Controller|Changed value is observed and original is restored|Cleanup|@n
 */
UT_ADD_TEST(VTS_L1_PANEL, enabledState_pos)
{
    android::sp<IPanelOutputController> controller;
    UT_ASSERT_TRUE_FATAL(openStarted(controller));
    bool enabled = false;
    Status status = controller->getEnabled(&enabled);
    UT_ASSERT_TRUE_FATAL(status.isOk());

    status = controller->setEnabled(!enabled);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    bool actual = enabled;
    status = controller->getEnabled(&actual);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_EQUAL(actual, !enabled);

    status = controller->setEnabled(enabled);
    UT_ASSERT_TRUE(status.isOk());
    stopClose(controller);
}

/**
 * @brief Validates picture-mode get, default, and set operations.
 *
 * This test selects a valid picture-mode context from the live service
 * capabilities, reads current and default modes, then applies the returned
 * current configuration.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 006@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** Panel controller can be started and exposes a picture-mode context.@n
 * **Dependencies:** getCapabilities(), IPanelOutputController lifecycle APIs.@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Step | Action | Input | Expected Result | Remarks |@n
 * |01|Open and start controller|N/A|Controller is STARTED|Setup|@n
 * |02|Get picture modes|Valid source and dynamic range|One current mode is returned|Live configuration|@n
 * |03|Get default picture modes|Same query|One default mode is returned|Default configuration|@n
 * |04|Set returned picture mode|Current configuration|Set succeeds|Valid update|@n
 * |05|Stop and close|Controller|Cleanup succeeds|Cleanup|@n
 */
UT_ADD_TEST(VTS_L1_PANEL, pictureModes_pos)
{
    android::sp<IPanelOutputController> controller;
    UT_ASSERT_TRUE_FATAL(openStarted(controller));
    const PictureModeConfiguration configuration = panelFirstPictureMode(mCapabilities);
    const std::vector<PictureModeConfiguration> requested {configuration};
    std::vector<PictureModeConfiguration> returned;
    bool result = false;                           

    Status status = controller->getPictureModes(requested, &returned, &result);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_TRUE_FATAL(result);
    UT_ASSERT_EQUAL_FATAL(returned.size(), requested.size());

    std::vector<PictureModeConfiguration> defaults;
    status = controller->getDefaultPictureModes(requested, &defaults, &result);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_TRUE_FATAL(result);
    UT_ASSERT_EQUAL_FATAL(defaults.size(), requested.size());

    status = controller->setPictureModes(returned, &result);
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_TRUE(result);
    stopClose(controller);
}

/**
 * @brief Validates PQ parameter reads, defaults, capabilities, and writes.
 *
 * This test obtains a valid selector from live capabilities, reads current and
 * default values, verifies every HFP-declared PQ capability, and writes the
 * returned current PQ configuration.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 007@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** Panel controller can be started and HFP is loaded.@n
 * **Dependencies:** Panel capabilities and supported PQ parameter declarations.@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Step | Action | Input | Expected Result | Remarks |@n
 * |01|Open and start controller|N/A|Controller is STARTED|Setup|@n
 * |02|Get current PQ values|Valid selector|Current configuration is returned|Read path|@n
 * |03|Get default PQ values|Valid selector|Default configuration is returned|Default path|@n
 * |04|Get PQ capabilities|Each HFP PQ parameter|Parameter is reported supported|HFP validation|@n
 * |05|Set current PQ values|Returned configuration|SUCCESS is returned|Write path|@n
 * |06|Stop and close|Controller|Cleanup succeeds|Cleanup|@n
 */
UT_ADD_TEST(VTS_L1_PANEL, pqParametersAndCapabilities_pos)
{
    android::sp<IPanelOutputController> controller;
    UT_ASSERT_TRUE_FATAL(openStarted(controller));
    const PictureModeConfiguration pictureMode = panelFirstPictureMode(mCapabilities);
    const PQParameterSelector selector = panelFirstPQSelector(mCapabilities, pictureMode);
    const std::vector<PQParameterSelector> selectors {selector};
    std::vector<PQParameterConfiguration> current;
    std::vector<PQParameterConfiguration> defaults;
    bool result = false;

    Status status = controller->getPQParameters(selectors, &current, &result);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_TRUE_FATAL(result);
    UT_ASSERT_EQUAL_FATAL(current.size(), selectors.size());

    status = controller->getDefaultPQParameters(selectors, &defaults, &result);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_TRUE_FATAL(result);
    UT_ASSERT_EQUAL_FATAL(defaults.size(), selectors.size());

    for (const PQParameter parameter : gPanel_hfp.supportedPQParameters)
    {
        PQParameterCapabilities capabilities;
        status = controller->getPQParameterCapabilities(parameter, &capabilities);
        UT_ASSERT_TRUE_FATAL(status.isOk());
        UT_ASSERT_EQUAL_FATAL(capabilities.pqParameter, parameter);
        UT_ASSERT_TRUE(capabilities.isSupported);
    }

    ErrorCode errorCode;
    status = controller->setPQParameters(current, &errorCode);
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_EQUAL(errorCode, ErrorCode::SUCCESS);
    stopClose(controller);
}

/**
 * @brief Validates refresh-rate and frame-rate-matching controls.
 *
 * This test reads the refresh rate, applies a service-supported rate, verifies
 * it, and toggles frame-rate matching when the capability is supported.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 008@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** Panel controller can be started.@n
 * **Dependencies:** Panel capabilities must provide refresh-rate information.@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Step | Action | Input | Expected Result | Remarks |@n
 * |01|Open and start controller|N/A|Controller is STARTED|Setup|@n
 * |02|Get refresh rate|Controller|Current rate is returned|Read path|@n
 * |03|Set a supported refresh rate|Live capability value|Set succeeds and value is observable|Rate control|@n
 * |04|Toggle frame-rate matching|Inverse current value|Toggle succeeds when supported|Conditional capability|@n
 * |05|Restore and close|Controller|Original setting restored and cleanup succeeds|Cleanup|@n
 */
UT_ADD_TEST(VTS_L1_PANEL, refreshRateAndFrameRateMatching_pos)
{
    android::sp<IPanelOutputController> controller;
    UT_ASSERT_TRUE_FATAL(openStarted(controller));
    double refreshRate = 0.0;
    Status status = controller->getRefreshRate(&refreshRate);
    UT_ASSERT_TRUE_FATAL(status.isOk());

    if (!mCapabilities.supportedRefreshRatesHz.empty())
    {
        bool result = false;
        const double supportedRate = mCapabilities.supportedRefreshRatesHz.front();
        status = controller->setRefreshRate(supportedRate, &result);
        UT_ASSERT_TRUE_FATAL(status.isOk());
        UT_ASSERT_TRUE_FATAL(result);
        status = controller->getRefreshRate(&refreshRate);
        UT_ASSERT_TRUE_FATAL(status.isOk());
        UT_ASSERT_EQUAL(refreshRate, supportedRate);
    }

    if (mCapabilities.frameRateMatchingSupported)
    {
        bool enabled = false;
        status = controller->getFrameRateMatching(&enabled);
        UT_ASSERT_TRUE_FATAL(status.isOk());
        bool result = false;
        status = controller->setFrameRateMatching(!enabled, &result);
        UT_ASSERT_TRUE_FATAL(status.isOk());
        UT_ASSERT_TRUE_FATAL(result);
        status = controller->setFrameRateMatching(enabled, &result);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(result);
    }
    stopClose(controller);
}

/**
 * @brief Validates video source override and video property queries.
 *
 * This test applies a source from a valid picture-mode context and verifies
 * source override, active source, dynamic range, frame-rate, and resolution
 * queries are accepted while the controller is STARTED.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 009@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** Panel controller can be started and exposes a valid picture-mode context.@n
 * **Dependencies:** Panel source and video property APIs.@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Step | Action | Input | Expected Result | Remarks |@n
 * |01|Open and start controller|N/A|Controller is STARTED|Setup|@n
 * |02|Set source override|Valid capability source|Set succeeds|Source control|@n
 * |03|Get video properties|Controller|All calls succeed; frame rate and resolution contain two values|Video state|@n
 * |04|Stop and close|Controller|Cleanup succeeds|Cleanup|@n
 */
UT_ADD_TEST(VTS_L1_PANEL, videoSourceAndVideoProperties_pos)
{
    android::sp<IPanelOutputController> controller;
    UT_ASSERT_TRUE_FATAL(openStarted(controller));
    const PictureModeConfiguration pictureMode = panelFirstPictureMode(mCapabilities);
    Status status = controller->setVideoSourceOverride(pictureMode.source);
    UT_ASSERT_TRUE_FATAL(status.isOk());

    PanelSource overrideSource;
    PanelSource videoSource;
    DynamicRange dynamicRange;
    std::array<int32_t, 2> frameRate;
    std::array<int32_t, 2> resolution;
    status = controller->getVideoSourceOverride(&overrideSource);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_EQUAL(overrideSource.type, pictureMode.source.type);
    status = controller->getVideoSource(&videoSource);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    status = controller->getDynamicRange(&dynamicRange);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    status = controller->getVideoFrameRate(&frameRate);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_EQUAL(frameRate.size(), 2);
    status = controller->getVideoResolution(&resolution);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_EQUAL(resolution.size(), 2);
    stopClose(controller);
}

/**
 * @brief Validates calibration-mode transitions.
 *
 * This test confirms calibration mode is disabled initially, enables it,
 * verifies the enabled state, then disables it before cleanup.
 *
 * **Test Group ID:** 01@n
 * **Test Case ID:** 010@n
 * **Priority:** High@n
 *
 * **Pre-Conditions:** Panel controller can be started.@n
 * **Dependencies:** Calibration-mode and controller lifecycle APIs.@n
 * **User Interaction:** None@n
 *
 * **Test Procedure:**@n
 * | Step | Action | Input | Expected Result | Remarks |@n
 * |01|Open and start controller|N/A|Controller is STARTED|Setup|@n
 * |02|Get calibration mode|Controller|Mode is disabled initially|Default state|@n
 * |03|Enable calibration mode|true|Operation succeeds and mode becomes enabled|Transition|@n
 * |04|Disable calibration mode|false|Operation succeeds|Restore state|@n
 * |05|Stop and close|Controller|Cleanup succeeds|Cleanup|@n
 */
UT_ADD_TEST(VTS_L1_PANEL, calibrationMode_pos)
{
    android::sp<IPanelOutputController> controller;
    UT_ASSERT_TRUE_FATAL(openStarted(controller));
    bool calibrationMode = true;
    Status status = controller->getCalibrationMode(&calibrationMode);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_FALSE_FATAL(calibrationMode);

    bool result = false;
    status = controller->enableCalibrationMode(true, &result);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_TRUE_FATAL(result);
    status = controller->getCalibrationMode(&calibrationMode);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_TRUE(calibrationMode);

    status = controller->enableCalibrationMode(false, &result);
    UT_ASSERT_TRUE_FATAL(status.isOk());
    UT_ASSERT_TRUE(result);
    stopClose(controller);
}
