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
#include <XPLM/XPLMPlugin.h>
#include "MoveVR.h"
#include "src/Logger.h"

MoveVR::MoveVR():
    windowManager(std::make_shared<WindowManager>())
{
}

void MoveVR::start() {
    createMenu("MoveVR");
    command = createCommand();

    flightLoopId = createFlightLoop();
    XPLMScheduleFlightLoop(flightLoopId, 0.25, true);

    auto vrRef = XPLMFindDataRef("sim/graphics/VR/enabled");
    bool inVr = XPLMGetDatai(vrRef);
    onVRStateChanged(inVr);
}

void MoveVR::onVRStateChanged(bool inVr) {
    windowManager->onVRStateChanged(inVr);
}

void MoveVR::onPlaneReload() {
    if (managerWidget) {
        managerWidget->SetVisible(false);
    }
    windowManager->closeVRWindows();
}

void MoveVR::stop() {
    if (flightLoopId) {
        XPLMDestroyFlightLoop(flightLoopId);
        flightLoopId = {};
    }
    destroyMenu();
    destroyCommand();
}

XPLMFlightLoopID MoveVR::createFlightLoop() {
    XPLMCreateFlightLoop_t loop;
    loop.structSize = sizeof(XPLMCreateFlightLoop_t);
    loop.phase = 0; // ignored according to docs
    loop.refcon = this;
    loop.callbackFunc = [] (float f1, float f2, int c, void *ref) -> float {
        if (!ref) {
            return 0;
        }
        auto *us = reinterpret_cast<MoveVR *>(ref);
        return us->onFlightLoop(f1, f2, c);
    };

    XPLMFlightLoopID id = XPLMCreateFlightLoop(&loop);
    if (!id) {
        throw std::runtime_error("Couldn't create flight loop");
    }
    return id;
}

void MoveVR::createMenu(const std::string& name) {
    XPLMMenuID pluginMenu = XPLMFindPluginsMenu();
    subMenuIdx = XPLMAppendMenuItem(pluginMenu, name.c_str(), nullptr, 0);

    if (subMenuIdx < 0) {
        throw std::runtime_error("Couldn't create our menu item");
    }

    subMenu = XPLMCreateMenu(name.c_str(), pluginMenu, subMenuIdx, [] (void *ctrl, void *win) {
        MoveVR *us = reinterpret_cast<MoveVR *>(ctrl);
        us->onMenu();
    }, this);

    if (!subMenu) {
        XPLMRemoveMenuItem(pluginMenu, subMenuIdx);
        throw std::runtime_error("Couldn't create our menu");
    }

    XPLMAppendMenuItem(subMenu, "Manage Windows", 0, 0);
}

XPLMCommandRef MoveVR::createCommand() {
    XPLMCommandRef cmd = XPLMCreateCommand("MoveVR/toggle_manager", "Toggle Window Manager");
    if (!cmd) {
        throw std::runtime_error("Couldn't create command");
    }

    XPLMRegisterCommandHandler(cmd, onCommand, true, this);
    return cmd;
}

int MoveVR::onCommand(XPLMCommandRef cmd, XPLMCommandPhase phase, void *ref) {
    MoveVR *us = reinterpret_cast<MoveVR *>(ref);
    if (!us) {
        return 1;
    }

    if (phase != xplm_CommandBegin) {
        return 1;
    }

    us->toggleWidget();

    return 1;
}

void MoveVR::destroyCommand() {
    XPLMUnregisterCommandHandler(command, onCommand, true, this);
}

void MoveVR::onMenu() {
    toggleWidget();
}

void MoveVR::destroyMenu() {
    if (subMenu) {
        XPLMDestroyMenu(subMenu);
        subMenu = nullptr;
        XPLMRemoveMenuItem(XPLMFindPluginsMenu(), subMenuIdx);
        subMenuIdx = -1;
    }
}

void MoveVR::toggleWidget() {
    windowManager->update();

    if (!managerWidget) {
        int left, top, right, bot;
        XPLMGetScreenBoundsGlobal(&left, &top, &right, &bot);

        int width = 800;
        int height = 600;
        int pad = 75;
        int x = left + pad;
        int y = top - pad;
        managerWidget = std::make_unique<ManagerWidget>(windowManager, x, y, x + width, y - height);
        return;
    }

    managerWidget->SetVisible(!managerWidget->GetVisible());
}

float MoveVR::onFlightLoop(float elapsedSinceLastCall, float elapseSinceLastLoop, int count) {
    windowManager->checkForClose();
    return 0.25;
}
