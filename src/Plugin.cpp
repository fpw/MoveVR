/*
 *   MoveVR - Move native windows into X-Plane's VR cockpit
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

#include <XPLM/XPLMDefs.h>
#include <XPLM/XPLMPlugin.h>
#include <XPLM/XPLMUtilities.h>
#include <memory>
#include "src/MoveVR/MoveVR.h"
#include "src/Logger.h"


namespace {
std::string getPluginPath() {
    XPLMPluginID ourId = XPLMGetMyID();
    char pathBuf[2048];
    XPLMGetPluginInfo(ourId, nullptr, pathBuf, nullptr, nullptr);
    char *pathPart = XPLMExtractFileAndPath(pathBuf);
    return std::string(pathBuf, 0, pathPart - pathBuf) + "/../";
}

std::unique_ptr<MoveVR> moveVR;
Gdiplus::GdiplusStartupInput gdiplusStartupInput {};
ULONG_PTR gdiplusToken {};
}

PLUGIN_API int XPluginStart(char *outName, char *outSignature, char *outDescription) {
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, 0);

    strncpy(outName, "MoveVR", 255);
    strncpy(outSignature, "org.solhost.folko.movevr", 255);

    try {
        logger::init(getPluginPath());
        logger::setStdOut(true);
        moveVR = std::make_unique<MoveVR>();
        strncpy(outDescription, "Move native windows into VR.", 255);
    } catch (const std::exception &e) {
        logger::error("Exception in XPluginStart: %s", e.what());
        strncpy(outDescription, e.what(), 255);
        return 0;
    }

    return 1;
}

PLUGIN_API int XPluginEnable(void) {
    try {
        if (moveVR) {
            moveVR->start();
        }
    } catch (const std::exception &e) {
        logger::error("Exception in XPluginEnable: %s", e.what());
        return 0;
    }

    return 1;
}

PLUGIN_API void XPluginReceiveMessage(XPLMPluginID src, int msg, void *inParam) {
    if (!moveVR) {
        return;
    }

    switch (msg) {
    case XPLM_MSG_PLANE_LOADED:
        if (inParam == 0) {
            moveVR->onPlaneReload();
        }
        break;
    case XPLM_MSG_ENTERED_VR:
        moveVR->onVRStateChanged(true);
        break;
    case XPLM_MSG_EXITING_VR:
        moveVR->onVRStateChanged(false);
        break;
    }
}

PLUGIN_API void XPluginDisable(void) {
    try {
        logger::verbose("Disabling plugin...");
        if (moveVR) {
            moveVR->stop();
        }
        logger::verbose("Disabled");
    } catch (const std::exception &e) {
        logger::error("Exception in XPluginDisable: %s", e.what());
    }
}

PLUGIN_API void XPluginStop(void) {
    try {
        logger::verbose("Stopping plugin...");
        moveVR.reset();
        if (gdiplusToken) {
            Gdiplus::GdiplusShutdown(gdiplusToken);
        }
        logger::verbose("Stopped");
    } catch (const std::exception &e) {
        logger::error("Exception in XPluginStop: %s", e.what());
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    return TRUE;
}
