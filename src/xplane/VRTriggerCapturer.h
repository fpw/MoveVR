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
#ifndef SRC_XPLANE_VRTRIGGERCAPTURER_H_
#define SRC_XPLANE_VRTRIGGERCAPTURER_H_

#include <XPLM/XPLMDataAccess.h>
#include <XPLM/XPLMDisplay.h>
#include <XPLM/XPLMProcessing.h>
#include <vector>
#include <functional>

class VRTriggerCapturer {
public:
    // the coordinates are panel pixel coordinates - clicks outside of panels will be reported, but there is
    // no way to get the coordinates in that case. They are set to -1.
    using TriggerCallback = std::function<void(XPLMMouseStatus, float, float)>;

    VRTriggerCapturer();
    void setEnabled(bool enable);
    void setTriggerCallback(TriggerCallback cb);
    ~VRTriggerCapturer();
private:
    XPLMFlightLoopID flightLoop{};
    bool isEnabled = false;
    TriggerCallback triggerCallback{};
    XPLMDataRef refXpix{}, refYpix{}, refButtonValues{};
    XPLMWindowID captureWindow{};
    std::vector<int> vrTriggerIndices;
    bool triggerDown = false;

    bool findVRTriggers();
    void checkVRClicks();
    void onTrigger(XPLMMouseStatus status);
};

#endif /* SRC_XPLANE_VRTRIGGERCAPTURER_H_ */
