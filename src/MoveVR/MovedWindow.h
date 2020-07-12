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
#ifndef SRC_MOVEVR_MOVEDWINDOW_H_
#define SRC_MOVEVR_MOVEDWINDOW_H_

#include <XPLM/XPLMGraphics.h>
#include <XPLM/XPLMDisplay.h>
#include <string>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>
#include "AsyncPBO.h"
#include "DataRef.h"
#include "src/windows/Window.h"

class MovedWindow {
public:
    MovedWindow(std::shared_ptr<Window> window);

    void setDelay(int dly);
    void setBrightness(float bright);
    void setDoDrag(bool drag);

    int getDelay();
    float getBrightness();
    bool getDoDrag();

    bool isShown();

    bool isInVR() const;

    ~MovedWindow();
private:
    std::shared_ptr<Window> wnd;
    DataRef<bool> isVrEnabled;
    XPLMWindowID window = nullptr;
    int textureId = -1;
    AsyncPBO pbo;
    std::atomic_int requestedWidth {0}, requestedHeight {0};
    std::atomic_bool doCapture;
    std::atomic_bool needRedraw;
    std::atomic_bool keepRunning { false };
    std::unique_ptr<std::thread> captureThread;

    std::atomic_bool doDrag { false };
    std::atomic_int drawDelay { 0 };
    std::atomic<float> brightness { 1.0f };
    int delayCount = 0;

    void initTexture();

    void captureLoop();

    void createWindow(const std::string &title);
    void onDraw();
    void correctRatio(int left, int top, int &right, int &bottom);
    bool onClick(int x, int y, XPLMMouseStatus status);
    bool onRightClick(int x, int y, XPLMMouseStatus status);
    void onKey(char key, XPLMKeyFlags flags, char virtualKey, bool losingFocus);
    XPLMCursorStatus getCursor(int x, int y);
    bool onMouseWheel(int x, int y, int wheel, int clicks);

    bool boxelToPixel(int bx, int by, int &px, int &py);
};

#endif /* SRC_MOVEVR_MOVEDWINDOW_H_ */
