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
#ifndef SRC_MOVEVR_MOVEVR_H_
#define SRC_MOVEVR_MOVEVR_H_

#include <XPLM/XPLMMenus.h>
#include <XPLM/XPLMUtilities.h>
#include <XPLM/XPLMProcessing.h>
#include <memory>
#include "MovedWindow.h"
#include "ManagerWidget.h"
#include "WindowManager.h"

class MoveVR {
public:
    MoveVR();
    void start();
    void onVRStateChanged(bool inVr);
    void onPlaneReload();
    void stop();
private:
    int subMenuIdx = -1;
    XPLMMenuID subMenu = nullptr;
    XPLMFlightLoopID flightLoopId {};
    XPLMCommandRef command {};
    std::unique_ptr<ManagerWidget> managerWidget;
    std::shared_ptr<WindowManager> windowManager;

    void createMenu(const std::string &name);
    void onMenu();
    void destroyMenu();

    XPLMCommandRef createCommand();
    static int onCommand(XPLMCommandRef cmd, XPLMCommandPhase phase, void* ref);
    void destroyCommand();

    void toggleWidget();

    XPLMFlightLoopID createFlightLoop();
    float onFlightLoop(float elapsedSinceLastCall, float elapseSinceLastLoop, int count);
};

#endif /* SRC_MOVEVR_MOVEVR_H_ */
