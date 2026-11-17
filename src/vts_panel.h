#ifndef __VTS_PANEL_H__
#define __VTS_PANEL_H__

#include <chrono>
#include <string>
#include <vector>

#include <binder/ProcessState.h>
#include <binder/Status.h>
#include <ut.h>
#include <ut_control_plane.h>
#include <ut_kvp_profile.h>
#include <ut_log.h>

#include "com/rdk/hal/panel/IPanelOutput.h"
#include "com/rdk/hal/panel/IPanelOutputController.h"
#include "vts_panel_helper_functions.h"
#include "vts_panel_parse_hfp.h"

using android::binder::Status;
using android::sp;
using namespace com::rdk::hal::panel;

#define PANEL_GET_CAPABILITIES(caps) \
    do { \
        Status status = gPanelOutput->getCapabilities(&(caps)); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define PANEL_OPEN_SUCCESS(listener, controller) \
    do { \
        Status status = gPanelOutput->open((listener), &(controller)); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_TRUE_FATAL((controller) != nullptr); \
    } while (0)

#define PANEL_OPEN_FAILURE(listener, controller, exception) \
    do { \
        Status status = gPanelOutput->open((listener), &(controller)); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), (exception)); \
    } while (0)

#define PANEL_CLOSE_SUCCESS(controller, closed) \
    do { \
        Status status = gPanelOutput->close((controller), &(closed)); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_TRUE_FATAL(closed); \
    } while (0)

#define PANEL_REGISTER_EVENT_SUCCESS(listener, registered) \
    do { \
        Status status = gPanelOutput->registerListener((listener), &(registered)); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_TRUE_FATAL(registered); \
    } while (0)

#define PANEL_UNREGISTER_EVENT_SUCCESS(listener, unregistered) \
    do { \
        Status status = gPanelOutput->unregisterListener((listener), &(unregistered)); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_TRUE_FATAL(unregistered); \
    } while (0)

#define PANEL_START_SUCCESS(controller, started) \
    do { \
        Status status = (controller)->start(&(started)); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_TRUE_FATAL(started); \
    } while (0)

#define PANEL_START_FAILURE(controller, started, exception) \
    do { \
        Status status = (controller)->start(&(started)); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), (exception)); \
    } while (0)

#define PANEL_STOP_SUCCESS(controller, stopped) \
    do { \
        Status status = (controller)->stop(&(stopped)); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_TRUE_FATAL(stopped); \
    } while (0)

#define PANEL_STOP_FAILURE(controller, stopped, exception) \
    do { \
        Status status = (controller)->stop(&(stopped)); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), (exception)); \
    } while (0)

#define PANEL_GET_STATE(controller, state) \
    do { \
        Status status = (controller)->getState(&(state)); \
        UT_ASSERT_TRUE_FATAL(status.isOk()); \
        UT_ASSERT_EQUAL_FATAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#endif
