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
#include <cmath>
#include "Window.h"
#include "src/Logger.h"

// these don't exist in Windows 7, so do not import them by IAT
using GetDpiForWindow_t = UINT WINAPI (*)(HWND);
using GetDpiForSystem_t = UINT WINAPI (*)();
auto u32lib = LoadLibraryA("user32");
GetDpiForWindow_t GetDpiForWindow_ = (GetDpiForWindow_t) GetProcAddress(u32lib, "GetDpiForWindow");
GetDpiForSystem_t GetDpiForSystem_ = (GetDpiForSystem_t) GetProcAddress(u32lib, "GetDpiForSystem");

Window::Window(HWND hWnd):
    wnd(hWnd)
{
}

bool Window::isEqual(const Window& other) const {
    return wnd == other.wnd;
}

int Window::getWidth() const {
    RECT winRect;
    GetWindowRect(wnd, &winRect);
    return winRect.right - winRect.left;
}

int Window::getHeight() const {
    RECT winRect;
    GetWindowRect(wnd, &winRect);
    return winRect.bottom - winRect.top;
}

float Window::getAspectRatio() const {
    RECT winRect;
    GetWindowRect(wnd, &winRect);

    return (winRect.bottom - winRect.top) / (float) (winRect.right - winRect.left);
}

std::string Window::getTitle() const {
    wchar_t nameBuf[200];
    if (wnd == GetDesktopWindow()) {
        return "Desktop";
    } else if (GetWindowTextW(wnd, nameBuf, sizeof(nameBuf) / sizeof(nameBuf[0])) > 0) {
        char res[sizeof(nameBuf)];
        WideCharToMultiByte(CP_UTF8, 0, nameBuf, -1, res, sizeof(res), nullptr, nullptr);
        return std::string(res);
    } else {
        return "";
    }
}

void Window::onMouseDown(int x, int y) {
    if (wnd == GetDesktopWindow()) {
        validClick = false;
        if (GetKeyState(VK_LBUTTON) >= 0) {
            mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN,
                    x * 65535 / getWidth(), y * 65535 / getHeight(), 0, 0);
            validClick = true;
        }
    } else  {
        HWND dest;
        convertMouseCoords(x, y, dest);
        if (dest) {
            PostMessageA(dest, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
        }
    }
}

void Window::onMouseDrag(int x, int y) {
    if (wnd == GetDesktopWindow()) {
        if (validClick) {
            mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE,
                    x * 65535 / getWidth(), y * 65535 / getHeight(), 0, 0);
        }
    } else  {
        HWND dest;
        convertMouseCoords(x, y, dest);
        if (dest) {
            PostMessageA(dest, WM_MOUSEMOVE, MK_LBUTTON, MAKELPARAM(x,y));
        }
    }
}

void Window::onMouseUp(int x, int y) {
    if (wnd == GetDesktopWindow()) {
        if (validClick) {
            mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTUP,
                    x * 65535 / getWidth(), y * 65535 / getHeight(), 0, 0);
        }
    } else  {
        HWND dest;
        convertMouseCoords(x, y, dest);
        if (dest) {
            PostMessageA(dest, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
        }
    }
}

void Window::onWheel(int x, int y, int dir) {
    if (wnd == GetDesktopWindow()) {
            mouse_event(MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_WHEEL,
                    x * 65535 / getWidth(), y * 65535 / getHeight(), dir * 15, 0);
    } else  {
        HWND dest;
        convertMouseCoords(x, y, dest);
        if (dest) {
            PostMessageA(dest, WM_VSCROLL, MAKEWPARAM(dir < 0 ? SB_LINEDOWN : SB_LINEUP, 0), 0);
        }
    }
}

void Window::convertMouseCoords(int &x, int &y, HWND &out) {
    int winX = x, winY = y;

    // Convert window coordinates to client coordinates
    RECT winRect;
    GetWindowRect(wnd, &winRect);
    POINT screenPoint {winX + winRect.left, winY + winRect.top};

    // Find child window at client coordinates
    POINT winPoint {};
    HWND curParent {};
    out = wnd;
    do {
        curParent = out;
        winPoint = screenPoint;
        ScreenToClient(curParent, &winPoint);
        out = RealChildWindowFromPoint(curParent, winPoint);
    } while (out != nullptr && out != curParent);
    out = curParent;

    ScreenToClient(out, &screenPoint);
    x = winPoint.x;
    y = winPoint.y;
}

void Window::updateScreenshot(int width, int height, int quality) {
    HDC windowDC = GetWindowDC(wnd);

    RECT winRect;
    GetWindowRect(wnd, &winRect);
    int winWidth = winRect.right - winRect.left;
    int winHeight = winRect.bottom - winRect.top;

    if (GetDpiForSystem_ && GetDpiForWindow_) {
        int wndDpi = GetDpiForWindow_(wnd);
        int sysDpi = GetDpiForSystem_();

        winWidth = MulDiv(winWidth, wndDpi, sysDpi);
        winHeight = MulDiv(winHeight, wndDpi, sysDpi);
    }

    updateBitmapIfDimensionChanged(windowDC, width, height);

    HBITMAP origBitmap = (HBITMAP) SelectObject(compDC, bitmap);
    if (quality == 0) {
        SetStretchBltMode(compDC, STRETCH_ANDSCANS);
    } else if (quality == 1) {
        SetStretchBltMode(compDC, STRETCH_DELETESCANS);
    } else {
        SetStretchBltMode(compDC, STRETCH_HALFTONE);
    }
    StretchBlt(compDC, 0, 0, bitmapWidth, bitmapHeight, windowDC, 0, 0, winWidth, winHeight, SRCCOPY);
    SelectObject(compDC, origBitmap);

    Gdiplus::Bitmap b(bitmap, nullptr);
    Gdiplus::Rect srcRect(0, 0, b.GetWidth(), b.GetHeight());

    Gdiplus::BitmapData data {};
    data.Width = b.GetWidth();
    data.Height = b.GetHeight();
    data.PixelFormat = PixelFormat24bppRGB;
    data.Stride = (3 * data.Width + (4 - 1)) & ~(4 - 1);

    shot.width = data.Width;
    shot.height = data.Height;
    shot.stride = std::abs((long) data.Stride);
    shot.pixels.resize(shot.stride * shot.height);

    data.Scan0 = shot.pixels.data();

    b.LockBits(&srcRect, Gdiplus::ImageLockModeRead | Gdiplus::ImageLockModeUserInputBuf, data.PixelFormat, &data);
    b.UnlockBits(&data);

    ReleaseDC(wnd, windowDC);
}

void Window::updateBitmapIfDimensionChanged(HDC winDC, int width, int height) {
    if (width != bitmapWidth || height != bitmapHeight) {
        if (bitmap) {
            DeleteObject(bitmap);
            bitmap = {};
        }
        if (compDC) {
            DeleteDC(compDC);
            compDC = {};
        }
        compDC = CreateCompatibleDC(winDC);
        bitmapWidth = width;
        bitmapHeight = height;
        bitmap = CreateCompatibleBitmap(winDC, bitmapWidth, bitmapHeight);
    }
}

const Window::Screenshot& Window::getLastScreenshot() const {
    return shot;
}

Window::~Window() {
    if (bitmap) {
        DeleteObject(bitmap);
    }
    if (compDC) {
        DeleteDC(compDC);
    }
}

std::vector<std::shared_ptr<Window>> findWindows() {
    std::vector<std::shared_ptr<Window>> res;

    res.push_back(std::make_shared<Window>(GetDesktopWindow()));

    EnumWindows([] (HWND wnd, LPARAM lparam) -> BOOL {
        auto *vec = reinterpret_cast<std::vector<std::shared_ptr<Window>> *>(lparam);

        DWORD pid;
        GetWindowThreadProcessId(wnd, &pid);
        if (pid == GetCurrentProcessId()) {
            return true;
        }

        if (!IsWindowVisible(wnd)) {
            return true;
        }

        HWND hwndWalk = nullptr;
        HWND hwndTry = GetAncestor(wnd, GA_ROOTOWNER);
        while(hwndTry != hwndWalk)
        {
            hwndWalk = hwndTry;
            hwndTry = GetLastActivePopup(hwndWalk);
            if(IsWindowVisible(hwndTry)) {
                break;
            }
        }
        if(hwndWalk != wnd) {
            return true;
        }

        if(GetWindowLong(wnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW) {
            return true;
        }

        if (GetWindowTextLengthW(wnd) == 0) {
            return true;
        }

        TITLEBARINFO ti;
        ti.cbSize = sizeof(ti);
        GetTitleBarInfo(wnd, &ti);
        if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE) {
            char title[255];
            GetWindowTextA(wnd, title, sizeof(title));
            bool isEFASS = (strstr(title, "EFASS") == title);
            bool isReflector = (strstr(title, "Reflector") == title);
            bool isAirDroid = (strstr(title, "AirDroid Cast v") == title);
            if (!isEFASS && !isReflector && !isAirDroid) {
                return true;
            }
        }

        vec->push_back(std::make_shared<Window>(wnd));

        return true;
    }, reinterpret_cast<LPARAM>(&res));

    return res;
}
