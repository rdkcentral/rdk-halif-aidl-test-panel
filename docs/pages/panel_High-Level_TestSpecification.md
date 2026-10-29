# Panel HAL High-Level Design and Test Specification

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Introduction](#introduction)
- [L2 Test Specification](#l2-test-specification)
- [L3 Test Specification](#l3-test-specification)

## Acronyms, Terms and Abbreviations

- `HAL` - Hardware Abstraction Layer
- `API` - Application Programming Interface
- `AIDL` - Android Interface Definition Language
- `L2` - Level 2 interface and functional testing
- `L3` - Level 3 platform integration and hardware-behaviour testing
- `AV` - Audio/Video
- `FRM` - Frame Rate Matching
- `PQ` - Picture Quality
- `HFP` - HAL Feature Profile
- `WB` - White Balance
- `CMS` - Colour Management System
- `DV` - Dolby Vision

## Introduction

The Panel HAL provides a stable Binder/AIDL interface for controlling and
observing a display panel on an RDK platform. It hides platform-specific
display-driver and picture-processing details behind a capability-driven
contract.

The `panel/current` module exposes one `PanelOutput` service that provides:

* Static panel capabilities, including panel type, pixel and physical
  dimensions, supported picture modes, dynamic ranges, AV sources, colour
  temperatures, PQ parameters, and refresh rates.
* An exclusive controller for lifecycle, enable-state, picture-mode, PQ,
  refresh-rate, video-source, and calibration operations.
* Controller callbacks for effective picture-mode and PQ changes.
* Passive event callbacks for video context, frame rate, resolution, refresh
  rate, and lifecycle-state changes.

Testing is capability-driven. A feature that is not declared in
`hfp-panel.yaml` or returned by `getCapabilities()` is not applicable to that
platform. Every declared feature must be tested against both the declaration
and the observed behaviour.

- **Panel Interface Definitions:** [Composite Input HAL](https://github.com/rdkcentral/rdk-halif-aidl/tree/0.23.0/panel/current/com/rdk/hal/panel/)
- **Panel HFP:** [HFP](https://github.com/rdkcentral/rdk-halif-aidl/blob/0.23.0/panel/current/hfp-panel.yaml)


## L2 Test Specification

| Test | Description | HAL APIs |
|------|-------------|----------|
| Validate lifecycle state transitions | <ul><li>Read the initial state with `getState()`.</li><li>From `STOPPED`, call `start()` and verify `STARTING` then `STARTED` state events.</li><li>From `STARTED`, call `stop()` and verify `STOPPING` then `STOPPED` state events.</li><li> Validate event listener notifications using `onStateChanged()` </li></ul> | <ul><li>IPanelOutputController - `start()`, `getState()`, `stop()`</li><li>IPanelOutputEventListener - `onStateChanged()`</li></ul> |
| Validate panel enable state | <ul><li>In `STARTED`, set enabled to `true` and `false`.</li><li>Read the result with `getEnabled()` after each change.</li></ul> | <ul><li>IPanelOutputController - `setEnabled()`, `getEnabled()`</li></ul> |
| Validate picture-mode configuration | <ul><li>For each supported picture mode, dynamic range, and AV-source combination, call `setPictureModes()`.</li><li>Use `getPictureModes()` to verify the selected mode and returned ordering.</li><li>Use `getDefaultPictureModes()` to verify default resolution.</li></ul> | <ul><li>IPanelOutputController - `setPictureModes()`, `getPictureModes()`, `getDefaultPictureModes()`</li></ul> |
| Validate PQ capabilities and values | <ul><li>For every supported PQ parameter, call `getPQParameterCapabilities()`.</li><li>Generate valid integer, enum, string, and structured values from the declared bounds and presets.</li><li>Set values with `setPQParameters()` and read them with `getPQParameters()`.</li></ul> | <ul><li>IPanelOutputController - `getPQParameterCapabilities()`, `setPQParameters()`, `getPQParameters()`, `getDefaultPQParameters()`</li></ul> |
| Validate refresh rate and FRM | <ul><li>Set every supported value in `supportedRefreshRatesHz` and verify `getRefreshRate()`.</li><li>Enable and disable FRM when supported and verify `getFrameRateMatching()`.</li></ul> | <ul><li>IPanelOutputController - `setRefreshRate()`, `getRefreshRate()`, `setFrameRateMatching()`, `getFrameRateMatching()`</li></ul> |
| Validate video context and source override | <ul><li>Verify the default source override is `AUTO`.</li><li>Set each supported source override and verify `getVideoSourceOverride()`.</li></ul> | <ul><li>IPanelOutputController - `setVideoSourceOverride()`, `getVideoSourceOverride()`</li></ul> |
| Validate calibration mode | <ul><li>Enable calibration mode and verify `getCalibrationMode()`.</li><li>Verify repeated enable or disable requests return the documented result.</li><li>Write supported WB and DV calibration values and read them back.</li></ul> | <ul><li>IPanelOutputController - `enableCalibrationMode()`, `getCalibrationMode()`, `setPQParameters()`, `getPQParameters()`</li></ul> |

## L3 Test Specification

| Test | Description | HAL APIs used |
|------|-------------|---------------|
| Verify video playback and settings application | <ul><li>Start video playback and verify normal content rendering in the `STARTED` state.</li><li>Apply panel output settings using `setEnabled()` and verify the expected visual behaviour is reflected during active playback.</li><li>Use `getEnabled()` to verify the reported state matches the applied configuration.</li><li>Revert the setting and verify video playback continues correctly with the original display output restored.</li><li>Repeat the operations and verify stable, consistent, and idempotent behaviour without video interruption or unintended PQ changes.</li></ul> | <ul><li>IPanelOutputController - `setEnabled()`, `getEnabled()`</li></ul> |
| Verify picture-mode application | <ul><li>Display controlled content and apply all supported picture modes, applicable contexts, and picture mode parameters listed in HFP_panel.yaml.</li><li>Verify the selected mode through `getPictureModes()` and `onPictureModeChanged()`.</li><li>Measure representative picture characteristics.</li><li>Change source and dynamic range and verify the correct scoped mode is restored.</li></ul> | <ul><li>IPanelOutputController - `setPictureModes()`, `getPictureModes()`, `getDefaultPictureModes()`</li><li>IPanelOutputControllerListener - `onPictureModeChanged()`</li><li>IPanelOutputEventListener - `onVideoSourceChanged()`, `onDynamicRangeChanged()`</li></ul> |
| Verify PQ parameter application | <ul><li>Retrieve supported PQ parameter capabilities and default values, and test all PQ parameters listed in HFP_panel.yaml within their declared bounds.</li><li>Apply supported PQ parameters, including colour temperature, two-point WB, multi-point WB, CMS, tone mapping, and DV calibration, where applicable.</li><li>Verify parameter values using getPQParameters() and confirm updates through onPQParameterChanged().</li><li>Play representative video content and verify visible picture quality changes for applicable PQ parameters, including brightness, contrast, colour reproduction, white balance, and tone mapping.</li><li>Verify parameter values remain scoped to picture mode, source, dynamic range, and colour temperature, where applicable, and are correctly restored across context changes.</li></ul> | <ul><li>IPanelOutputController - `setPQParameters()`, `getPQParameters()`, `getPQParameterCapabilities()`, `getDefaultPQParameters()`</li><li>IPanelOutputControllerListener - `onPQParameterChanged()`</li></ul> |
| Verify calibration mode operation | <ul><li>Enable calibration mode and verify the panel enters the platform-defined measurement state.</li><li>Apply WB and DV calibration values and verify their measured effect.</li><li>Apply non-calibration values and verify whether they are deferred as specified.</li><li>Disable calibration mode and verify stored and deferred values become active.</li></ul> | <ul><li>IPanelOutputController - `enableCalibrationMode()`, `getCalibrationMode()`, `setPQParameters()`</li><li>IPanelOutputControllerListener - `onPQParameterChanged()`</li></ul> |
| Verify refresh-rate matching during playback | <ul><li>Enable FRM and play content at each applicable frame rate.</li><li>Verify video frame-rate and panel refresh-rate callbacks.</li><li>Confirm the physical panel selects a supported refresh rate that matches or appropriately multiplies the video frame rate.</li><li>Disable FRM and verify normal refresh-rate behaviour returns.</li></ul> | <ul><li>IPanelOutputController - `setFrameRateMatching()`, `getFrameRateMatching()`</li><li>IPanelOutputEventListener - `onVideoFrameRateChanged()`, `onRefreshRateChanged()`</li></ul> |