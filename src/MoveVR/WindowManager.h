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
#ifndef SRC_MOVEVR_WINDOWMANAGER_H_
#define SRC_MOVEVR_WINDOWMANAGER_H_

#include <memory>
#include <vector>
#include <map>
#include <functional>
#include "src/windows/Window.h"
#include "MovedWindow.h"

class WindowManager {
public:
    using WindowIterator = std::function<void(std::shared_ptr<Window>)>;

    void update();
    void checkForClose();
    void forEachWindow(WindowIterator f);

    std::shared_ptr<MovedWindow> moveToVR(std::shared_ptr<Window> window);
    std::shared_ptr<MovedWindow> findMovedWindow(std::shared_ptr<Window> window);

    void closeVRWindows();

private:
    std::vector<std::shared_ptr<Window>> systemWindows;
    std::map<std::shared_ptr<Window>, std::shared_ptr<MovedWindow>> movedWindows;
};

#endif /* SRC_MOVEVR_WINDOWMANAGER_H_ */
