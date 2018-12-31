/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <XPLM/XPLMUtilities.h>
#include "src/Logger.h"
#include "VRTriggerCapturer.h"

VRTriggerCapturer::VRTriggerCapturer() {
    refXpix = XPLMFindDataRef("sim/graphics/view/click_3d_x_pixels");
    refYpix = XPLMFindDataRef("sim/graphics/view/click_3d_y_pixels");
    refButtonValues = XPLMFindDataRef("sim/joystick/joystick_button_values");

    XPLMCreateFlightLoop_t params{};
    params.structSize = sizeof(params);
    params.callbackFunc = [] (float elapsedSinceCall, float elapsedSinceLoop, int refCount, void *ref) -> float {
        ((VRTriggerCapturer *) (ref))->checkVRClicks();
        return -1;
    };
    params.phase = 0; // ignored
    params.refcon = this;
    flightLoop = XPLMCreateFlightLoop(&params);
}

void VRTriggerCapturer::setTriggerCallback(TriggerCallback cb) {
    triggerCallback = cb;
}

void VRTriggerCapturer::setEnabled(bool enable) {
    if (enable == isEnabled) {
        return;
    }

    isEnabled = enable;

    if (!isEnabled) {
        logger::verbose("Not capturing VR clicks");
        XPLMScheduleFlightLoop(flightLoop, 0, false);
        return;
    }

    bool foundController = findVRTriggers();
    if (foundController) {
        logger::verbose("Now capturing VR clicks");
        XPLMScheduleFlightLoop(flightLoop, -1, false);
    } else {
        logger::verbose("Not capturing VR clicks - no controllers detected");
    }
}

bool VRTriggerCapturer::findVRTriggers() {
    int triggerIndex = (uintptr_t) XPLMFindCommand("sim/VR/reserved/select");
    if (triggerIndex == 0) {
        logger::warn("Could not setup VR trigger check: command not found");
        return false;
    }

    auto assignmentsRef = XPLMFindDataRef("sim/joystick/joystick_button_assignments");
    if (!assignmentsRef) {
        logger::warn("Could not setup VR trigger check: assignments ref not found");
        return false;
    }

    vrTriggerIndices.clear();

    int assignments[3200];
    XPLMGetDatavi(assignmentsRef, assignments, 0, 3200);
    for (size_t i = 0; i < 3200; i++) {
        if (assignments[i] == triggerIndex) {
            vrTriggerIndices.push_back(i);
        }
    }

    return !vrTriggerIndices.empty();
}

void VRTriggerCapturer::checkVRClicks() {
    bool gotAnyTrigger = false;

    for (auto idx: vrTriggerIndices) {
        int triggerVal = 0;
        XPLMGetDatavi(refButtonValues, &triggerVal, idx, 1);
        if (triggerVal) {
            gotAnyTrigger = true;
            break;
        }
    }

    if (gotAnyTrigger) {
        // trigger is currently down
        if (!triggerDown) {
            // was not down before -> click
            onTrigger(xplm_MouseDown);
            triggerDown = true;
        } else {
            // was down before -> drag
            onTrigger(xplm_MouseDrag);
        }
    } else {
        // trigger is currently released
        if (triggerDown) {
            // was down before -> release
            onTrigger(xplm_MouseUp);
            triggerDown = false;
        }
    }
}

void VRTriggerCapturer::onTrigger(XPLMMouseStatus status) {
    if (triggerCallback) {
        float x = XPLMGetDataf(refXpix);
        float y = XPLMGetDataf(refYpix);
        triggerCallback(status, x, y);
    }
}

VRTriggerCapturer::~VRTriggerCapturer() {
    XPLMDestroyFlightLoop(flightLoop);
}
