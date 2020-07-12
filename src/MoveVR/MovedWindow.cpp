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
#include <stdexcept>
#include <chrono>
#include <GL/gl.h>
#include <GL/glext.h>
#include "MovedWindow.h"
#include "src/Logger.h"

MovedWindow::MovedWindow(std::shared_ptr<Window> window):
    wnd(window),
    isVrEnabled("sim/graphics/VR/enabled", false)
{
    initTexture();
    createWindow(wnd->getTitle());

    doCapture = true;
    needRedraw = false;

    keepRunning = true;
    captureThread = std::make_unique<std::thread>(&MovedWindow::captureLoop, this);
}

void MovedWindow::setDelay(int dly) {
    drawDelay = dly;
}

void MovedWindow::setBrightness(float bright) {
    brightness = bright;
}

void MovedWindow::setDoDrag(bool drag) {
    doDrag = drag;
}

bool MovedWindow::getDoDrag() {
    return doDrag;
}

int MovedWindow::getDelay() {
    return drawDelay;
}

float MovedWindow::getBrightness() {
    return brightness;
}

bool MovedWindow::isShown() {
    return XPLMGetWindowIsVisible(window);
}

void MovedWindow::initTexture() {
    if (textureId < 0) {
        XPLMGenerateTextureNumbers(&textureId, 1);
        XPLMBindTexture2d(textureId, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

void MovedWindow::createWindow(const std::string &title) {
    int winLeft, winTop, winRight, winBot;
    XPLMGetScreenBoundsGlobal(&winLeft, &winTop, &winRight, &winBot);

    requestedWidth = 300;
    requestedHeight = requestedWidth * wnd->getAspectRatio();

    XPLMCreateWindow_t params;
    params.structSize = sizeof(params);
    params.left = winLeft + 100 ;
    params.right = winLeft + 100 + requestedWidth;
    params.top = winTop - 100 ;
    params.bottom = winTop - 100 - requestedHeight;
    params.visible = 1;
    params.refcon = this;
    params.drawWindowFunc = [] (XPLMWindowID id, void *ref) {
        reinterpret_cast<MovedWindow *>(ref)->onDraw();
    };
    params.handleMouseClickFunc = [] (XPLMWindowID id, int x, int y, XPLMMouseStatus status, void *ref) -> int {
        return reinterpret_cast<MovedWindow *>(ref)->onClick(x, y, status);
    };
    params.handleRightClickFunc = [] (XPLMWindowID id, int x, int y, XPLMMouseStatus status, void *ref) -> int {
        return reinterpret_cast<MovedWindow *>(ref)->onRightClick(x, y, status);
    };
    params.handleKeyFunc = [] (XPLMWindowID id, char key, XPLMKeyFlags flags, char vKey, void *ref, int losingFocus) {
        reinterpret_cast<MovedWindow *>(ref)->onKey(key, flags, vKey, losingFocus);
    };
    params.handleCursorFunc = [] (XPLMWindowID id, int x, int y, void *ref) -> XPLMCursorStatus {
        return reinterpret_cast<MovedWindow *>(ref)->getCursor(x, y);
    };
    params.handleMouseWheelFunc =  [] (XPLMWindowID id, int x, int y, int wheel, int clicks, void *ref) -> int {
        return reinterpret_cast<MovedWindow *>(ref)->onMouseWheel(x, y, wheel, clicks);
    };
    params.layer = xplm_WindowLayerFloatingWindows;
    params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;

    window = XPLMCreateWindowEx(&params);

    if (!window) {
        throw std::runtime_error("Couldn't create window");
    }

    if (isVrEnabled) {
        XPLMSetWindowPositioningMode(window, xplm_WindowVR, -1);
    } else {
        XPLMSetWindowPositioningMode(window, xplm_WindowPositionFree, -1);
        XPLMSetWindowGravity(window, 0, 1, 0, 0);
    }

    XPLMSetWindowTitle(window, title.c_str());

    XPLMBindTexture2d(textureId, 0);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    pbo.init(requestedWidth, requestedHeight, (3 * requestedWidth + (4 - 1)) & ~(4 - 1));
}

void MovedWindow::captureLoop() {
    while (keepRunning) {
        auto ptr = pbo.getBackBuffer();
        if (!ptr) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1 + drawDelay * 2));
            continue;
        }

        try {
            wnd->updateScreenshot(pbo.getBackbufferWidth(), pbo.getBackbufferHeight(), 2);
        } catch (const std::exception &e) {
            pbo.finishBackBuffer();
            logger::info("No screenshot: %s", e.what());
            break;
        }

        auto &screenshot = wnd->getLastScreenshot();
        memcpy(ptr, screenshot.pixels.data(), screenshot.pixels.size());
        pbo.finishBackBuffer();

        std::this_thread::sleep_for(std::chrono::milliseconds(1 + drawDelay * 2));
    }
}

void MovedWindow::onDraw() {
    int left, top, right, bottom;
    XPLMGetWindowGeometry(window, &left, &top, &right, &bottom);

    int xWinWidth = right - left;
    int xWinHeight = top - bottom;

    bool changedGeometry = false;
    if (xWinWidth != requestedWidth) {
        float ourRatio = wnd->getAspectRatio();
        xWinHeight = xWinWidth * ourRatio;
        bottom = top - xWinHeight;
        changedGeometry = true;
    } else if (xWinHeight != requestedHeight) {
        float ourRatio = wnd->getAspectRatio();
        xWinWidth = xWinHeight / ourRatio;
        right = left + xWinWidth;
        changedGeometry = true;
    }

    if (changedGeometry) {
        requestedHeight = top - bottom;
        requestedWidth = right - left;
        if (XPLMWindowIsInVR(window)) {
            XPLMSetWindowGeometryVR(window, requestedWidth, requestedHeight);
        } else {
            XPLMSetWindowGeometry(window, left, top, right, bottom);
        }
        pbo.setSize(requestedWidth, requestedHeight, (3 * requestedWidth + (4 - 1)) & ~(4 - 1));
    }

    XPLMBindTexture2d(textureId, 0);
    XPLMSetGraphicsState(0, 1, 0, 0, 1, 1, 0);

    pbo.drawFrontBuffer();

    glColor3f(brightness, brightness, brightness);

    glBegin(GL_QUADS);
        // map top left texture to bottom left vertex
        glTexCoord2i(0, 1);
        glVertex2i(left, bottom);

        // map bottom left texture to top left vertex
        glTexCoord2i(0, 0);
        glVertex2i(left, top);

        // map bottom right texture to top right vertex
        glTexCoord2i(1, 0);
        glVertex2i(right, top);

        // map top right texture to bottom right vertex
        glTexCoord2i(1, 1);
        glVertex2i(right, bottom);
    glEnd();
}

bool MovedWindow::onClick(int x, int y, XPLMMouseStatus status) {
    int px = 0, py = 0;
    if (!boxelToPixel(x, y, px, py)) {
        return true;
    }

    switch (status) {
    case xplm_MouseDown:
        wnd->onMouseDown(px, py);
        break;
    case xplm_MouseDrag:
        if (doDrag) {
            wnd->onMouseDrag(px, py);
        }
        break;
    case xplm_MouseUp:
        wnd->onMouseUp(px, py);
        break;
    }

    return true;
}

bool MovedWindow::onRightClick(int x, int y, XPLMMouseStatus status) {
    return true;
}

void MovedWindow::onKey(char key, XPLMKeyFlags flags, char virtualKey, bool losingFocus) {
}

XPLMCursorStatus MovedWindow::getCursor(int x, int y) {
    return xplm_CursorArrow;
}

bool MovedWindow::onMouseWheel(int x, int y, int wheel, int clicks) {
    int px = 0, py = 0;
    if (!boxelToPixel(x, y, px, py)) {
        return true;
    }
    wnd->onWheel(px, py, clicks);
    return true;
}

void MovedWindow::correctRatio(int left, int top, int& right, int& bottom) {
    int xWinWidth = right - left;
    int xWinHeight = top - bottom;

    float ourRatio = wnd->getAspectRatio();

    if (xWinWidth * ourRatio <= xWinHeight) {
        xWinHeight = xWinWidth * ourRatio;
    } else {
        xWinWidth = xWinHeight / ourRatio;
    }

    right = left + xWinWidth;
    bottom = top - xWinHeight;
}

bool MovedWindow::boxelToPixel(int bx, int by, int& px, int& py) {
    int bLeft, bTop, bRight, bBottom;
    XPLMGetWindowGeometry(window, &bLeft, &bTop, &bRight, &bBottom);

    correctRatio(bLeft, bTop, bRight, bBottom);

    if (bLeft == bRight || bTop == bBottom) {
        px = -1;
        py = -1;
        return false;
    }

    // calculate the center of the window in boxels
    int bCenterX = bLeft + (bRight - bLeft) / 2;
    int bCenterY = bBottom + (bTop - bBottom) / 2;

    // normalized vector from center to point
    float vecX = (bx - bCenterX) / float(bRight - bLeft);
    float vecY = (by - bCenterY) / float(bTop - bBottom);

    // GUI center in pixels
    int guiWidth = wnd->getWidth();
    int guiHeight = wnd->getHeight();
    int pCenterX = guiWidth / 2;
    int pCenterY = guiHeight / 2;

    // apply the vector to our center to get the coordinates in pixels
    px = pCenterX + vecX * guiWidth;
    py = pCenterY - vecY * guiHeight;

    // check if it's inside the window
    if (px >= 0 && px < guiWidth && py >= 0 && py < guiHeight) {
        return true;
    } else {
        return false;
    }
}

bool MovedWindow::isInVR() const {
    return XPLMWindowIsInVR(window);
}

MovedWindow::~MovedWindow() {
    keepRunning = false;

    if (captureThread) {
        captureThread->join();
    }

    if (window) {
        XPLMDestroyWindow(window);
    }
    logger::info("~MovedWindow");
}
