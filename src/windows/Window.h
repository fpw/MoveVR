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
#ifndef SRC_WINDOWS_WINDOW_H_
#define SRC_WINDOWS_WINDOW_H_

#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <memory>
#include <cstdint>

class Window {
public:
    struct Screenshot {
        int width, height, stride;
        std::vector<uint8_t> pixels;
    };

    Window(HWND hWnd);
    bool isEqual(const Window &other) const;

    std::string getTitle() const;
    int getWidth() const;
    int getHeight() const;

    void updateScreenshot(int width, int height, int quality);
    float getAspectRatio() const;
    const Screenshot &getLastScreenshot() const;

    void onMouseDown(int x, int y);
    void onMouseDrag(int x, int y);
    void onMouseUp(int x, int y);
    void onWheel(int x, int y, int dir);

    ~Window();
private:
    HWND wnd {};

    HDC compDC {};
    HBITMAP bitmap {};
    int bitmapWidth = 0, bitmapHeight = 0;
    bool validClick = false;

    Screenshot shot {};

    void updateBitmapIfDimensionChanged(HDC winDC, int width, int height);
    void convertMouseCoords(int &x, int &y, HWND &out);
};

std::vector<std::shared_ptr<Window>> findWindows();

#endif /* SRC_WINDOWS_WINDOW_H_ */
