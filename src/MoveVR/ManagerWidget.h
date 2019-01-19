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
#ifndef SRC_MOVEVR_MANAGERWIDGET_H_
#define SRC_MOVEVR_MANAGERWIDGET_H_

#include <functional>
#include <vector>
#include <map>
#include <memory>
#include "WindowManager.h"
#include "ImgWindow.h"

class ManagerWidget: public ImgWindow {
public:
    struct WindowConfig {
        int quality = 0;
        int delay = 0;
        bool dragging = false;
        float brightness = 1.0f;
    };

    ManagerWidget(std::shared_ptr<WindowManager> mgr, int left, int top, int right, int bot);
protected:
    void buildInterface() override;
private:
    std::shared_ptr<WindowManager> manager;
    std::map<std::shared_ptr<Window>, WindowConfig> windowConfig;

    void buildXPlaneWindows();
    void buildXPlaneWindow(XPLMWindowID wnd);

    void buildSystemWindows();

    std::map<XPLMPluginID, std::vector<XPLMWindowID>> groupWindowsByPlugin();
};

#endif /* SRC_MOVEVR_MANAGERWIDGET_H_ */
