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
#include <stdexcept>
#include "XPlaneWindowList.h"
#include "src/Logger.h"

/*
 * This class can be used to enumerate all plugin windows, even windows from other plugins.
 *
 * How does this work? XPLMWindowIDs are actually pointers to nodes of a double-linked list.
 * That means we can iterate the whole window list if we have any element of the list.
 *
 * To get such an element, we simply create a window and keep it valid (but invisible).
 *
 * To identify the previous and next pointers inside the window struct, we create another window
 * right after creating our node window and then search for the pointers of the respective other window
 * in both windows.
 *
 * Note that destroying merely removes the window from the list, the node itselfs stays valid and is not
 * modified, i.e. the previous and next pointers can become invalid. That's why we must keep our node window
 * alive so that it is always updated when the window list changes.
 */

namespace {
    // We won't actually use these callbacks - they are used as unique IDs to identify the offsets in the opaque handle
    void onDraw(XPLMWindowID, void *) { }
    int onLeftClick(XPLMWindowID, int, int, XPLMMouseStatus, void *) { return 0; }
    void onKey(XPLMWindowID, char, XPLMKeyFlags, char, void *, int) { }
    XPLMCursorStatus onCursor(XPLMWindowID, int, int, void *) { return xplm_CursorDefault; }
    int onMouseWheel(XPLMWindowID, int, int, int, int, void *) { return 0; }
    int onRightClick(XPLMWindowID, int, int, XPLMMouseStatus, void *) { return 0; }
}

XPlaneWindowList::XPlaneWindowList() {
    createNodes();

    try {
        findOffsets();
    } catch (...) {
        if (otherNode) {
            XPLMDestroyWindow(otherNode);
        }

        throw;
    }
}

void XPlaneWindowList::createNodes() {
    // let's fill the struct with sane parameters so we can search for them and
    // uniquely identify them. Avoid using low numbers (e.g. window dimensions) so
    // that we can find our plugin id (which will likely be low)
    XPLMCreateWindow_t params{};
    params.structSize = sizeof(params);
    params.left = 0xBABE;
    params.right = 0xCAFE;
    params.top = 0xDEAD;
    params.bottom = 0xF00D;
    params.visible = 0;
    params.drawWindowFunc = onDraw;
    params.handleMouseClickFunc = onLeftClick;
    params.handleKeyFunc = onKey;
    params.handleCursorFunc = onCursor;
    params.handleMouseWheelFunc = onMouseWheel;
    params.refcon = this;
    params.decorateAsFloatingWindow = xplm_WindowDecorationSelfDecoratedResizable;
    params.layer = xplm_WindowLayerModal;
    params.handleRightClickFunc = onRightClick;

    node = XPLMCreateWindowEx(&params);
    otherNode = XPLMCreateWindowEx(&params);
}

void XPlaneWindowList::findOffsets() {
    logger::verbose("Searching way out of plugin sandbox...");

    // find offset from node -> otherNode
    offsetNext = findPointerOffset(node, OPAQUE_SIZE, otherNode);
    logger::verbose("offsetNext: %d", offsetNext);

    // find offset from otherNode -> node
    offsetPrev = findPointerOffset(otherNode, OPAQUE_SIZE, node);
    logger::verbose("offsetPrev: %d", offsetPrev);

    // identify callback pointers
    offsetClick = findPointerOffset(node, OPAQUE_SIZE, (void *) onLeftClick);
    logger::verbose("offsetClick: %d", offsetClick);

    XPLMPluginID ourId = XPLMGetMyID();
    if (ourId > 1) {
        offsetPluginId = findIntOffset(node, OPAQUE_SIZE, ourId);
    } else {
        logger::warn("Using default value for offsetPluginId (dangerous - install more plugins!)");
    }
    logger::verbose("offsetPluginId: %d", offsetPluginId);

    XPLMDestroyWindow(otherNode);
    otherNode = {};

    logger::verbose("Can now escape plugin sandbox!");
}

XPLMPluginID XPlaneWindowList::getPluginFromWindow(XPLMWindowID wnd) {
    if (!wnd) {
        return XPLMPluginID{};
    }

    uint8_t *ptr = (uint8_t *) wnd;
    uint8_t *plugin = ptr + offsetPluginId;
    return *((XPLMPluginID *) plugin);
}

XPLMWindowID XPlaneWindowList::getNextWindow(XPLMWindowID wnd) {
    if (!wnd) {
        return XPLMWindowID{};
    }

    uint8_t *ptr = (uint8_t *) wnd;
    uint8_t *nxt = ptr + offsetNext;
    return *((XPLMWindowID *) nxt);
}

XPLMWindowID XPlaneWindowList::getPreviousWindow(XPLMWindowID wnd) {
    if (!wnd) {
        return XPLMWindowID{};
    }

    uint8_t *ptr = (uint8_t *) wnd;
    uint8_t *prv = ptr + offsetPrev;
    return *((XPLMWindowID *) prv);
}

std::vector<XPLMWindowID> XPlaneWindowList::findWindows() {
    std::vector<XPLMWindowID> res;

    // find windows behind us (prev)
    for (auto cur = getPreviousWindow(node); cur != XPLMWindowID{}; cur = getPreviousWindow(cur)) {
        res.push_back(cur);
    }

    // find windows ahead of us (next)
    for (auto cur = getNextWindow(node); cur != XPLMWindowID{}; cur = getNextWindow(cur)) {
        res.push_back(cur);
    }

    return res;
}

XPLMCreateWindow_t globalParams;

void XPlaneWindowList::sendLeftClick(XPLMWindowID wnd, XPLMMouseStatus status, int x, int y) {
    if (!XPLMGetWindowIsVisible(wnd)) {
        return;
    }

    uint8_t *ptr = (uint8_t *) wnd;
    uint8_t *click = ptr + offsetClick;
    XPLMHandleMouseClick_f handler = *((XPLMHandleMouseClick_f *) click);

    if (handler) {
        auto ref = XPLMGetWindowRefCon(wnd);
        // the ABI is not compatible if the handler is not inside the struct due to some calling convention optimization
        globalParams.handleMouseClickFunc = handler;
        globalParams.handleMouseClickFunc(wnd, status, x, y, ref);
    }
}

size_t XPlaneWindowList::findPointerOffset(void *base, size_t size, void *ptr) {
    uintptr_t pattern = (uintptr_t) ptr;

    for (size_t i = 0; i <= size - sizeof(pattern); i += sizeof(pattern)) {
        if (memcmp(((uint8_t *) base) + i, &pattern, sizeof(pattern)) == 0) {
            return i;
        }
    }

    throw std::runtime_error("Magic not found");
}

size_t XPlaneWindowList::findIntOffset(void *base, size_t size, int needle) {
    for (size_t i = 0; i <= size - sizeof(needle); i += sizeof(needle)) {
        if (memcmp(((uint8_t *) base) + i, &needle, sizeof(needle)) == 0) {
            return i;
        }
    }

    throw std::runtime_error("Magic not found");
}

XPlaneWindowList::~XPlaneWindowList() {
    XPLMDestroyWindow(node);
    if (otherNode) {
        XPLMDestroyWindow(otherNode);
    }
}
