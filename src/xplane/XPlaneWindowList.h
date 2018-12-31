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
#ifndef MOVEVR_SRC_XPLANE_XPLANEWINDOWLIST_H_
#define MOVEVR_SRC_XPLANE_XPLANEWINDOWLIST_H_

#include <XPLM/XPLMDisplay.h>
#include <XPLM/XPLMPlugin.h>
#include <cstdint>
#include <vector>

class XPlaneWindowList {
public:
    XPlaneWindowList();
    std::vector<XPLMWindowID> findWindows();
    XPLMPluginID getPluginFromWindow(XPLMWindowID wnd);
    void sendCursorMove(XPLMWindowID wnd, int x, int y);
    void sendLeftClick(XPLMWindowID wnd, XPLMMouseStatus status, int x, int y);
    ~XPlaneWindowList();
private:
    static constexpr const size_t OPAQUE_SIZE = 128;

    XPLMWindowID node{}, otherNode{};
    size_t offsetPrev = 0;
    size_t offsetNext = 0;
    size_t offsetClick = 0;
    size_t offsetCursor = 0;
    size_t offsetPluginId = 0;

    void createNodes();
    void findOffsets();

    size_t findPointerOffset(void *base, size_t size, void *ptr);
    size_t findIntOffset(void *base, size_t size, int needle);

    XPLMWindowID getNextWindow(XPLMWindowID wnd);
    XPLMWindowID getPreviousWindow(XPLMWindowID wnd);
};

#endif /* MOVEVR_SRC_XPLANE_XPLANEWINDOWLIST_H_ */
