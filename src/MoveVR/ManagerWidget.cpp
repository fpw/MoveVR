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
#include <XPLM/XPLMDisplay.h>
#include <imgui.h>
#include "src/Logger.h"
#include "ManagerWidget.h"

ManagerWidget::ManagerWidget(std::shared_ptr<WindowManager> mgr, int left, int top, int right, int bot):
    ImgWindow(left, top, right, bot),
    manager(mgr)
{
    SetWindowTitle("MoveVR " MOVEVR_VERSION " by Folke Will");
    SetVisible(true);
}

void ManagerWidget::buildInterface() {
    if (ImGui::TreeNode("Usage & About")) {
        const char *text =
                "MoveVR " MOVEVR_VERSION ", copyright 2018 by Folke Will <folko@solhost.org>\n"
                "Uses the X-Plane imgui integration library, copyright 2018 by Christopher Collins\n"
                "\n"
                "Make sure your original windows are about the same size as you need them in VR.\n"
                "Increasing the delay slider improves X-Plane's frame rate by slower capturing.\n"
                "Only use a higher quality setting if you really need it as it is rather expensive (FPS).\n"
                "Only enable mouse dragging if you are using an application that needs dragging or panning.\n"
                "";
        ImGui::Text(text);

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Plugin Windows")) {
        buildXPlaneWindows();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("System Windows")) {
        buildSystemWindows();
        ImGui::TreePop();
    }
}

void ManagerWidget::buildXPlaneWindows() {
    auto pluginWindows = groupWindowsByPlugin();

    for (auto it = pluginWindows.begin(); it != pluginWindows.end(); ++it) {
        auto plugin = it->first;
        char name[512];
        XPLMGetPluginInfo(plugin, name, nullptr, nullptr, nullptr);

        ImGui::PushID(plugin);
        if (ImGui::TreeNode(name)) {
            auto &windows = it->second;

            for (auto wnd: windows) {
                int top, left, bottom, right;
                XPLMGetWindowGeometry(wnd, &left, &top, &right, &bottom);

                char buf[128];
                sprintf(buf, "Window of size %dx%d###%p", right - left, top - bottom, wnd);

                if (ImGui::TreeNode(buf)) {
                    buildXPlaneWindow(wnd);
                    ImGui::TreePop();
                }
            }

            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}

void ManagerWidget::buildXPlaneWindow(XPLMWindowID wnd) {
    bool isInVR = XPLMWindowIsInVR(wnd);

    bool isReceiver = manager->isTriggerReceiver(wnd);
    if (ImGui::Checkbox("Receive VR panel clicks", &isReceiver)) {
        if (isReceiver) {
            manager->addTriggerReceiver(wnd);
        } else {
            manager->removeTriggerReceiver(wnd);
        }
    }

    if (ImGui::Button("Enable VR for legacy windows")) {
        manager->getXPlaneWindows()->injectFunction(wnd, [] {
            XPLMEnableFeature("XPLM_USE_NATIVE_WIDGET_WINDOWS", 1);
        });
    }

    if (!isInVR) {
        if (ImGui::Button("Move to VR")) {
            XPLMSetWindowPositioningMode(wnd, xplm_WindowVR, -1);
        }
    } else {
        if (ImGui::Button("Move to 2D")) {
            XPLMSetWindowPositioningMode(wnd, xplm_WindowPositionFree, -1);
        }
    }
}

void ManagerWidget::buildSystemWindows() {
    manager->forEachWindow([this] (std::shared_ptr<Window> wnd) {
        ImGui::PushID(wnd.get());
        if (ImGui::TreeNode(wnd->getTitle().c_str())) {
            auto &config = windowConfig[wnd];
            auto moved = manager->findMovedWindow(wnd);

            if (moved) {
                config.brightness = moved->getBrightness();
                config.delay = moved->getDelay();
                config.quality = moved->getQuality();
                config.dragging = moved->getDoDrag();
            }

            if (ImGui::SliderInt("", &config.delay, 0, 20, "Delay: %.0f frames")) {
                if (moved) {
                    moved->setDelay(config.delay);
                }
            }

            if (ImGui::SliderFloat("Brightness", &config.brightness, 0, 1)) {
                if (moved) {
                    moved->setBrightness(config.brightness);
                }
            }

            if (ImGui::RadioButton("Fast Quality", &config.quality, 0)) {
                if (moved) {
                    moved->setQuality(config.quality);
                }
            }
            ImGui::SameLine();

            if (ImGui::RadioButton("Medium Quality", &config.quality, 1)) {
                if (moved) {
                    moved->setQuality(config.quality);
                }
            }
            ImGui::SameLine();

            if (ImGui::RadioButton("Best Quality", &config.quality, 2)) {
                if (moved) {
                    moved->setQuality(config.quality);
                }
            }

            if (ImGui::Checkbox("Support Mouse Dragging", &config.dragging)) {
                if (moved) {
                    moved->setDoDrag(config.dragging);
                }
            }

            if (!moved) {
                if (ImGui::Button("Move to VR")) {
                    moved = manager->moveToVR(wnd);
                    moved->setDelay(config.delay);
                    moved->setQuality(config.quality);
                    moved->setBrightness(config.brightness);
                    moved->setDoDrag(config.dragging);
                }
            } else {
                ImGui::Text("Window is in VR");
            }

            ImGui::TreePop();
        }
        ImGui::PopID();
    });
}

std::map<XPLMPluginID, std::vector<XPLMWindowID>> ManagerWidget::groupWindowsByPlugin() {
    std::map<XPLMPluginID, std::vector<XPLMWindowID>> res;

    auto windowList = manager->getXPlaneWindows();
    auto windows = windowList->findWindows();

    for (auto wnd: windows) {
        if (!XPLMGetWindowIsVisible(wnd)) {
            continue;
        }

        auto plugin = windowList->getPluginFromWindow(wnd);
        res[plugin].push_back(wnd);
    }

    return res;
}
