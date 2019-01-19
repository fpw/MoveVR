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
    triggerRef = XPLMFindCommand("sim/VR/reserved/select");
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
        XPLMUnregisterCommandHandler(triggerRef, onRawTrigger, 1, this);
        return;
    }

    if (triggerRef) {
        logger::verbose("Now capturing VR clicks");
        XPLMRegisterCommandHandler(triggerRef, onRawTrigger, 1, this);
    } else {
        logger::verbose("Not capturing VR clicks - no controllers detected");
    }
}

int VRTriggerCapturer::onRawTrigger(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref) {
    ((VRTriggerCapturer *) ref)->onTrigger(phase);
    return 1;
}

void VRTriggerCapturer::onTrigger(XPLMCommandPhase status) {
    if (triggerCallback) {
        float x = XPLMGetDataf(refXpix);
        float y = XPLMGetDataf(refYpix);
        switch (status) {
            case xplm_CommandBegin: triggerCallback(xplm_MouseDown, x, y); break;
            case xplm_CommandContinue: triggerCallback(xplm_MouseDrag, x, y); break;
            case xplm_CommandEnd: triggerCallback(xplm_MouseUp, x, y); break;
        }
    }
}

VRTriggerCapturer::~VRTriggerCapturer() {
    XPLMUnregisterCommandHandler(triggerRef, onRawTrigger, 1, this);
}
