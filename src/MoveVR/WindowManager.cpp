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
#include "WindowManager.h"

void WindowManager::update() {
    auto currentWindows = findWindows();

    // Remove non-existing
    for (auto it = systemWindows.begin(); it != systemWindows.end(); ) {
        bool exists = false;
        for (auto &curWin: currentWindows) {
            if ((*it)->isEqual(*curWin)) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            movedWindows.erase(*it);
            it = systemWindows.erase(it);
        } else {
            ++it;
        }
    }

    // Add new ones
    for (auto &win: currentWindows) {
        bool exists = false;
        for (auto &sysWin: systemWindows) {
            if (sysWin->isEqual(*win)) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            systemWindows.push_back(win);
        }
    }
}

void WindowManager::forEachWindow(WindowIterator f) {
    for (auto &win: systemWindows) {
        f(win);
    }
}

std::shared_ptr<MovedWindow> WindowManager::moveToVR(std::shared_ptr<Window> window) {
    auto movedWnd = std::make_shared<MovedWindow>(window);
    movedWindows.insert(std::make_pair(window, movedWnd));
    return movedWnd;
}

std::shared_ptr<MovedWindow> WindowManager::findMovedWindow(std::shared_ptr<Window> window) {
    auto it = movedWindows.find(window);
    if (it == movedWindows.end()) {
        return nullptr;
    }
    return it->second;
}

void WindowManager::checkForClose() {
    for (auto it = movedWindows.begin(); it != movedWindows.end(); ) {
        if (!it->second->isShown()) {
            it = movedWindows.erase(it);
        } else {
            ++it;
        }
    }
}

void WindowManager::closeVRWindows() {
    for (auto it = movedWindows.begin(); it != movedWindows.end(); ) {
        if (it->second->isInVR()) {
            it = movedWindows.erase(it);
        } else {
            ++it;
        }
    }
}
