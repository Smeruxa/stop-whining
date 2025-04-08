#include "imguisdraw.h"

#include <common.h>
#include <CWeather.h>
#include <CWorld.h>

#include <sampapi/CNetGame.h>
#include <RakHook/rakhook.hpp>
#include <RakNet/PacketEnumerations.h>
#include "sampver/sampver.h"

#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>

imguiDraw* gImguiDraw;

imguiDraw::imguiDraw() {
    ImGuiWidgets = new Widgets();
}

struct TextAnimator
{
    std::vector<std::string> texts;
    float transitionDuration;
    float holdDuration;
    int currentTextIndex;
    int nextTextIndex;
    float alpha;
    bool fadingOut;
    bool holding;
    std::chrono::steady_clock::time_point startTime;

    TextAnimator(const std::vector<std::string>& texts, float transitionDuration, float holdDuration)
        : texts(texts), transitionDuration(transitionDuration), holdDuration(holdDuration),
        currentTextIndex(0), nextTextIndex(0), alpha(1.0f), fadingOut(false), holding(false)
    {
        startTime = std::chrono::steady_clock::now();
    }

    void Update()
    {
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - startTime).count();

        if (holding)
        {
            if (elapsed >= holdDuration)
            {
                holding = false;
                fadingOut = true;
                startTime = now;
            }
        }
        else if (fadingOut)
        {
            alpha -= (1.0f / transitionDuration) * elapsed;
            if (alpha <= 0.0f)
            {
                alpha = 0.0f;
                fadingOut = false;
                nextTextIndex = (nextTextIndex + 1) % texts.size();
                currentTextIndex = nextTextIndex;
                alpha = 0.0f;
                startTime = now;
            }
        }
        else
        {
            if (elapsed >= transitionDuration)
            {
                alpha = 1.0f;
                holding = true;
                startTime = now;
            }
            else
                alpha = (elapsed / transitionDuration);
        }
    }

    void Render(const ImVec2& childSize)
    {
        ImVec2 textSize = ImGui::CalcTextSize(texts[currentTextIndex].c_str());
        ImVec2 textPos = ImVec2((childSize.x - textSize.x) * 0.5f, (childSize.y - textSize.y) * 0.5f);

        ImGui::SetCursorPos(textPos);
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, alpha), "%s", texts[currentTextIndex].c_str());
    }
};

void imguiDraw::Draw() {
	ImGui::SetNextWindowSize(ImVec2(630.f, 430.f), ImGuiCond_Always);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
	ImGui::Begin("Stop Whining", &variables.is_open_menu, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
    if (ImGui::BeginChild(1, ImVec2(160, 430), false)) {
        //ImGuiWidgets->RenderAnimatedSquares(ImGui::GetCursorScreenPos(), ImVec2(160, 430));

        ImGui::PushFont(variables.fonts[2]);
        
        ImVec4 textColor = ImVec4(0.400f, 0.988f, 0.945f, 1.0f); // Цвет текста
        ImVec4 glowColor = ImVec4(0.400f, 0.988f, 0.945f, 0.5f); // Цвет свечения

        ImGui::SetCursorPosY(20);
        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
        ImGuiWidgets->DrawGlowingText("Stop", ImVec2(cursorPos.x + (ImGui::GetWindowSize().x - ImGui::CalcTextSize("Stop").x) / 2, cursorPos.y), textColor, glowColor, 0.8f);

        ImGui::SetCursorPosY(55);
        cursorPos = ImGui::GetCursorScreenPos();

        ImGuiWidgets->DrawGlowingText("Whining", ImVec2(cursorPos.x + (ImGui::GetWindowSize().x - ImGui::CalcTextSize("Whining").x) / 2, cursorPos.y), textColor, glowColor, 0.8f);

        ImGui::Text("");

        ImGui::PopFont();

        static std::vector<std::pair<PLUGIN_STATES, std::vector<std::string>>> lists {
                { PLUGIN_STATES::CROSSHAIR, { ICON_FA_CROSSHAIRS, "Прицел" } },
                { PLUGIN_STATES::PLAYER, { ICON_FA_PERSON, "Игрок" } },
                { PLUGIN_STATES::VEHICLE, { ICON_FA_CAR, "Машина" } },
                { PLUGIN_STATES::VISUAL, { ICON_FA_WALLET, "Визуальное" } },
                { PLUGIN_STATES::ADDITIONAL, { ICON_FA_TENT, "Остальное" } },
                { PLUGIN_STATES::DESTROY, { ICON_FA_TRASH, "Отгрузка" } }
        };

        ImGui::Dummy(ImVec2(0, 33));
        RenderButtons(lists, CURRENT_STATE, 160.f);

        ImGui::PushFont(variables.fonts[3]);
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 60);
        ImGuiWidgets->ImGuiCenterTextByX("< Автор >");
        ImGuiWidgets->ImGuiCenterTextByX("Telegram - @smeruxa");
        ImGuiWidgets->ImGuiCenterTextByX("VK - @wfsmeruxa");
        ImGui::PopFont();

        ImGui::EndChild();
    }

    ImGui::SameLine();

    static TextAnimator textAnimator({
            "Добро пожаловать в наш чит!",
            "Сегодня отличный день для новых достижений!",
            "Мы рады видеть вас здесь!",
            "Не стесняйтесь обращаться за помощью.",
            "Вы на шаг ближе к вашим целям!",
            "Пусть этот день принесет много радости!",
            "Мы постоянно улучшаем наш чит для вас.",
            "Ваше мнение важно для нас!",
            "Не забывайте проверять обновления!",
            "Благодарим за выбор нашего сервиса!",
            "Удачи в ваших начинаниях!",
            "Ваша поддержка вдохновляет нас!",
            "Спасибо, что вы с нами!",
            "Надеемся, вам будет комфортно!",
            "Не пропустите важное!",
            "Мы готовы ответить на все ваши вопросы!",
            "Давайте вместе достигать результатов!",
            "Ваш успех — наша гордость!",
            "Следите за новостями и обновлениями!",
            "Каждый день — новая возможность!"
    }, 3.0f, 30.0f);

    static float secondMenuWidth = 467.f;
    if (ImGui::BeginChild(2, ImVec2(secondMenuWidth, 430), false)) {
        if (ImGui::BeginChild(3, ImVec2(secondMenuWidth, 35), false)) {

            textAnimator.Update();
            textAnimator.Render(ImVec2(secondMenuWidth, 35));

            ImGui::SetCursorPosX(ImGui::GetWindowSize().x - 30);
            ImGui::SetCursorPosY(5);

            ImGui::PushFont(variables.faFonts[0]);
            if (ImGuiWidgets->AnimButton(ICON_FA_XMARK, ImVec2(25, 25), { 1.0f, 0.7f }))
                variables.is_open_menu = false;
            ImGui::PopFont();

            ImGui::EndChild();
        }

        if (ImGui::BeginChild(4, ImVec2(secondMenuWidth, 395), false)) {

            ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 40, ImGui::GetCursorPosY() + 8));
            if (ImGui::BeginChild(5, ImVec2(secondMenuWidth - 40, 395 - 8), false)) {
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.15f, 0.5f));
                ImGui::PushFont(variables.fonts[0]);
                if (CURRENT_STATE == PLUGIN_STATES::CROSSHAIR) {

                    if (ImGui::BeginChild("##FOVSettings", ImVec2(secondMenuWidth - 80, 87), false)) {

                        ImGui::Dummy(ImVec2(0.f, 6.f));

                        ImGuiWidgets->ImGuiCenterTextByX("FOV настройка");

                        ImGui::Dummy(ImVec2(0.f, 6.f));

                        ImDrawList* drawList = ImGui::GetWindowDrawList();
                        ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                        cursorPos.y += 2.f * 0.5f;

                        drawList->AddLine(ImVec2(cursorPos.x, cursorPos.y), ImVec2(cursorPos.x + ImGui::GetContentRegionAvail().x, cursorPos.y), ImGui::GetColorU32(ImVec4(0.400f, 0.988f, 0.945f, 0.6f)), 2.5f);

                        ImGui::Dummy(ImVec2(0.f, 6.f));

                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY()));
                        if (ImGui::BeginTable("##FOVSettings", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            ImGui::Dummy(ImVec2(0.f, 7.f));

                            static bool showFOV = gConfig->data["crosshair"]["FOV"]["show"].get<bool>();
                            if (ImGui::Checkbox("Show FOV ##FOVSettings", &showFOV)) {
                                gConfig->data["crosshair"]["FOV"]["show"] = showFOV;
                                gConfig->save();
                            }

                            ImGui::SameLine();

                            ImGuiWidgets->ShowHelpMarker("FOV_TOOLTIP", "От точки, куда целится ваш персонаж, образуется круг,\nв радиусе которого он ищет персонажей,\nв которых в последующем будет целиться");

                            ImGui::TableSetColumnIndex(1);

                            static float FOV = gConfig->data["crosshair"]["FOV"]["radius"].get<float>();
                            std::pair<float, bool> resultFOV = ImGuiWidgets->CustomSliderFloat("FOV", FOV, 1.0f, 500.f, "%1.1f", 3, 130);
                            if (resultFOV.second) {
                                gConfig->data["crosshair"]["FOV"]["radius"] = resultFOV.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##silentAim", ImVec2(secondMenuWidth - 80, 103), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##silentAimTable", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool silentAim = gConfig->data["crosshair"]["silentAim"]["state"].get<bool>();
                            if (ImGui::Checkbox("Silent Aim ##SilentAim", &silentAim)) {
                                gConfig->data["crosshair"]["silentAim"]["state"] = silentAim;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 9.f));

                            static bool skipSameClist = gConfig->data["crosshair"]["silentAim"]["skipSameClist"].get<bool>();
                            if (ImGui::Checkbox("Skip Same Clist ##SilentAim", &skipSameClist)) {
                                gConfig->data["crosshair"]["silentAim"]["skipSameClist"] = skipSameClist;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static bool shootThroughWalls = gConfig->data["crosshair"]["silentAim"]["shootThroughWalls"].get<bool>();
                            if (ImGui::Checkbox("Through walls ##SilentAim", &shootThroughWalls)) {
                                gConfig->data["crosshair"]["silentAim"]["shootThroughWalls"] = shootThroughWalls;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static float hitChance = gConfig->data["crosshair"]["silentAim"]["hitChance"].get<float>();
                            std::pair<float, bool> resultChance = ImGuiWidgets->CustomSliderFloat("Hit Chance", hitChance, 0.f, 100.f, "%1.1f", 1, 130);
                            if (resultChance.second) {
                                gConfig->data["crosshair"]["silentAim"]["hitChance"] = resultChance.first;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 30.f));

                            static float maxDistance = gConfig->data["crosshair"]["silentAim"]["maxDistance"].get<float>();
                            std::pair<float, bool> resultDistance = ImGuiWidgets->CustomSliderFloat("Max Distance", maxDistance, 0.f, 300.f, "%1.1f", 2, 130);
                            if (resultDistance.second) {
                                gConfig->data["crosshair"]["silentAim"]["maxDistance"] = resultDistance.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##smoothAim", ImVec2(secondMenuWidth - 80, 127), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##smoothAim", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool smoothAim = gConfig->data["crosshair"]["smoothAim"]["state"].get<bool>();
                            if (ImGui::Checkbox("Smooth Aim ##SmoothAim", &smoothAim)) {
                                gConfig->data["crosshair"]["smoothAim"]["state"] = smoothAim;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static bool skipSameClist = gConfig->data["crosshair"]["smoothAim"]["skipSameClist"].get<bool>();
                            if (ImGui::Checkbox("Skip Same Clist ##SmoothAim", &skipSameClist)) {
                                gConfig->data["crosshair"]["smoothAim"]["skipSameClist"] = skipSameClist;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static bool skipDead = gConfig->data["crosshair"]["smoothAim"]["skipDead"].get<bool>();
                            if (ImGui::Checkbox("Skip Dead ##SmoothAim", &skipDead)) {
                                gConfig->data["crosshair"]["smoothAim"]["skipDead"] = skipDead;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static bool skipAFK = gConfig->data["crosshair"]["smoothAim"]["skipAFK"].get<bool>();
                            if (ImGui::Checkbox("Skip AFK ##SmoothAim", &skipAFK)) {
                                gConfig->data["crosshair"]["smoothAim"]["skipAFK"] = skipAFK;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            ImGui::Dummy(ImVec2(0.f, 10.f));

                            static float maxDistance = gConfig->data["crosshair"]["smoothAim"]["maxDistance"].get<float>();
                            std::pair<float, bool> resultDistance = ImGuiWidgets->CustomSliderFloat("Max Distance", maxDistance, 0.f, 300.f, "%1.1f", 4, 130);
                            if (resultDistance.second) {
                                gConfig->data["crosshair"]["smoothAim"]["maxDistance"] = resultDistance.first;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 30.f));

                            static float smooth = gConfig->data["crosshair"]["smoothAim"]["smooth"].get<float>();
                            std::pair<float, bool> resultSmooth = ImGuiWidgets->CustomSliderFloat("Smooth", smooth, 1.f, 30.f, "%1.1f", 5, 130);
                            if (resultSmooth.second) {
                                gConfig->data["crosshair"]["smoothAim"]["smooth"] = resultSmooth.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##triggerBot", ImVec2(secondMenuWidth - 80, 126), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##triggerBot", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool triggerBot = gConfig->data["crosshair"]["triggerBot"]["state"].get<bool>();
                            if (ImGui::Checkbox("Trigger Bot ##TriggerBot", &triggerBot)) {
                                gConfig->data["crosshair"]["triggerBot"]["state"] = triggerBot;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static bool skipSameClistTriggerBot = gConfig->data["crosshair"]["triggerBot"]["skipSameClist"].get<bool>();
                            if (ImGui::Checkbox("Skip Same Clist ##TriggerBot", &skipSameClistTriggerBot)) {
                                gConfig->data["crosshair"]["triggerBot"]["skipSameClist"] = skipSameClistTriggerBot;
                                gConfig->save();
                            }

                            static bool activateByKey = gConfig->data["crosshair"]["triggerBot"]["activateByKey"].get<bool>();

                            if (!activateByKey) {
                                ImGui::Dummy(ImVec2(0.f, 18.f));
                                if (ImGui::Checkbox("Activate By Key ##TriggerBot", &activateByKey)) {
                                    gConfig->data["crosshair"]["triggerBot"]["activateByKey"] = activateByKey;
                                    gConfig->save();
                                }
                            }
                            else {
                                ImGui::Dummy(ImVec2(0.f, 3.f));
                                if (ImGui::Checkbox("Activate By Key ##TriggerBot", &activateByKey)) {
                                    gConfig->data["crosshair"]["triggerBot"]["activateByKey"] = activateByKey;
                                    gConfig->save();
                                }

                                ImGui::Dummy(ImVec2(0.f, 3.f));

                                static ImGuiKey triggerKey = gConfig->data["crosshair"]["triggerBot"]["button"].get<ImGuiKey>();
                                if (gHotKey->Hotkey("TriggerBot HotKey", triggerKey, ImVec2(150, 25))) {
                                    gConfig->data["crosshair"]["triggerBot"]["button"] = ImGui::GetKeyIndex(triggerKey);
                                    gConfig->save();
                                }
                            }

                            ImGui::TableSetColumnIndex(1);

                            ImGui::Dummy(ImVec2(0.f, 10.f));

                            static float maxDistance = gConfig->data["crosshair"]["triggerBot"]["maxDistance"].get<float>();
                            std::pair<float, bool> resultDistance = ImGuiWidgets->CustomSliderFloat("Max Distance", maxDistance, 0.f, 300.f, "%1.1f", 7, 130);
                            if (resultDistance.second) {
                                gConfig->data["crosshair"]["triggerBot"]["maxDistance"] = resultDistance.first;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 30.f));

                            static float delay = gConfig->data["crosshair"]["triggerBot"]["delay"].get<float>();
                            std::pair<float, bool> resultDelay = ImGuiWidgets->CustomSliderFloat("Shoot Delay", delay, 0.f, 2.f, "%1.1f", 8, 130);
                            if (resultDelay.second) {
                                gConfig->data["crosshair"]["triggerBot"]["delay"] = resultDelay.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##NoSpread", ImVec2(secondMenuWidth - 80, 55), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##NoSpread", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            ImGui::SetCursorPosY(13.5f);

                            static bool noSpread = gConfig->data["crosshair"]["noSpread"]["state"].get<bool>();
                            if (ImGui::Checkbox("NoSpread ##NoSpread", &noSpread)) {
                                gConfig->data["crosshair"]["noSpread"]["state"] = noSpread;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static float spreadCount = gConfig->data["crosshair"]["noSpread"]["spread"].get<float>();
                            std::pair<float, bool> resultSpreadCount = ImGuiWidgets->CustomSliderFloat("Spread", spreadCount, 0.f, 100.f, "%1.1f", 13, 130);
                            if (resultSpreadCount.second) {
                                gConfig->data["crosshair"]["noSpread"]["spread"] = resultSpreadCount.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##NoCamRestore", ImVec2(secondMenuWidth - 80, 38), false)) {
                        ImGui::SetCursorPos(ImVec2(74, 6));

                        static bool noCamRestore = gConfig->data["crosshair"]["noCamRestore"]["state"].get<bool>();
                        if (ImGui::Checkbox("NoCamRestore [Extra WS] ##NoCamRestore", &noCamRestore)) {
                            gConfig->data["crosshair"]["noCamRestore"]["state"] = noCamRestore;
                            gConfig->save();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));
                }
                else if (CURRENT_STATE == PLUGIN_STATES::PLAYER) {
                    if (ImGui::BeginChild("##airbrake", ImVec2(secondMenuWidth - 80, 68), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##airbrake", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool airbrake = gConfig->data["player"]["airbrake"]["state"].get<bool>();
                            if (ImGui::Checkbox("AirBrake ##AirBrake", &airbrake)) {
                                gConfig->data["player"]["airbrake"]["state"] = airbrake;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static ImGuiKey airbrakeKey = gConfig->data["player"]["airbrake"]["button"].get<ImGuiKey>();
                            if (gHotKey->Hotkey("Airbrake HotKey ##AirBrake", airbrakeKey, ImVec2(150, 25))) {
                                gConfig->data["player"]["airbrake"]["button"] = ImGui::GetKeyIndex(airbrakeKey);
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static float airBrakeSpeed = gConfig->data["player"]["airbrake"]["speed"].get<float>();
                            std::pair<float, bool> resultSpeed = ImGuiWidgets->CustomSliderFloat("Speed", airBrakeSpeed, 0.f, 2.f, "%.2f", 9, 130);
                            if (resultSpeed.second) {
                                gConfig->data["player"]["airbrake"]["speed"] = resultSpeed.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##AntiStun", ImVec2(secondMenuWidth - 80, 97), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##AntiStun", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool antistun = gConfig->data["player"]["antistun"]["state"].get<bool>();
                            if (ImGui::Checkbox("AntiStun ##AntiStun", &antistun)) {
                                gConfig->data["player"]["antistun"]["state"] = antistun;
                                gConfig->save();
                            }

                            static bool antiStunByKey = gConfig->data["player"]["antistun"]["activateByKey"].get<bool>();

                            if (antiStunByKey) {
                                ImGui::Dummy(ImVec2(0.f, 3.f));

                                if (ImGui::Checkbox("Activate By Key ##AntiStun", &antiStunByKey)) {
                                    gConfig->data["player"]["antistun"]["activateByKey"] = antiStunByKey;
                                    gConfig->save();
                                }

                                ImGui::Dummy(ImVec2(0.f, 3.f));

                                static ImGuiKey antiStunKey = gConfig->data["player"]["antistun"]["button"].get<ImGuiKey>();
                                if (gHotKey->Hotkey("AntiStun HotKey", antiStunKey, ImVec2(150, 25))) {
                                    gConfig->data["player"]["antistun"]["button"] = ImGui::GetKeyIndex(antiStunKey);
                                    gConfig->save();
                                }
                            }
                            else {
                                ImGui::Dummy(ImVec2(0.f, 18.f));

                                if (ImGui::Checkbox("Activate By Key ##AntiStun", &antiStunByKey)) {
                                    gConfig->data["player"]["antistun"]["activateByKey"] = antiStunByKey;
                                    gConfig->save();
                                }
                            }

                            ImGui::TableSetColumnIndex(1);

                            ImGui::Dummy(ImVec2(0.f, 17.f));

                            static float antistunChance = gConfig->data["player"]["antistun"]["chance"].get<float>();
                            std::pair<float, bool> resultChance = ImGuiWidgets->CustomSliderFloat("Chance", antistunChance, 0.f, 100.f, "%1.1f", 10, 130);
                            if (resultChance.second) {
                                gConfig->data["player"]["antistun"]["chance"] = resultChance.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##FastRotation", ImVec2(secondMenuWidth - 80, 55), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##FastRotation", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            ImGui::SetCursorPosY(13.5f);

                            static bool fastRotation = gConfig->data["player"]["fastRotation"]["state"].get<bool>();
                            if (ImGui::Checkbox("Fast Rotation ##FastRotation", &fastRotation)) {
                                gConfig->data["player"]["fastRotation"]["state"] = fastRotation;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static float fastRotationSpeed = gConfig->data["player"]["fastRotation"]["speed"].get<float>();
                            std::pair<float, bool> resultRotationSpeed = ImGuiWidgets->CustomSliderFloat("Speed", fastRotationSpeed, 1.f, 30.f, "%1.1f", 11, 130);
                            if (resultRotationSpeed.second) {
                                gConfig->data["player"]["fastRotation"]["speed"] = resultRotationSpeed.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##Sbiv", ImVec2(secondMenuWidth - 80, 55), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##Sbiv", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            ImGui::SetCursorPosY(13.5f);

                            static bool sbiv = gConfig->data["player"]["sbiv"]["state"].get<bool>();
                            if (ImGui::Checkbox("Сбив анимации ##Sbiv", &sbiv)) {
                                gConfig->data["player"]["sbiv"]["state"] = sbiv;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            ImGui::SetCursorPosY(13.5f);

                            static ImGuiKey sbivKey = gConfig->data["player"]["sbiv"]["button"].get<ImGuiKey>();
                            if (gHotKey->Hotkey("Destroy Anim HotKey", sbivKey, ImVec2(150, 25))) {
                                gConfig->data["player"]["sbiv"]["button"] = ImGui::GetKeyIndex(sbivKey);
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##MegaJump", ImVec2(secondMenuWidth - 80, 55), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##MegaJump", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            ImGui::SetCursorPosY(13.5f);

                            static bool megaJump = gConfig->data["player"]["megaJump"]["state"].get<bool>();
                            if (ImGui::Checkbox("Mega Jump ##MegaJump", &megaJump)) {
                                gConfig->data["player"]["megaJump"]["state"] = megaJump;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static float megaJumpIncrease = gConfig->data["player"]["megaJump"]["increase"].get<float>();
                            std::pair<float, bool> resultJumpIncrease = ImGuiWidgets->CustomSliderFloat("Num increase", megaJumpIncrease, 1.f, 10.f, "%1.1f", 12, 130);
                            if (resultJumpIncrease.second) {
                                gConfig->data["player"]["megaJump"]["increase"] = resultJumpIncrease.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##Another Player", ImVec2(secondMenuWidth - 80, 96), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##Another Player", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool infinityRun = gConfig->data["player"]["infinityRun"].get<bool>();
                            if (ImGui::Checkbox("Infinity Run ##Another Player", &infinityRun)) {
                                gConfig->data["player"]["infinityRun"] = infinityRun;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static bool infinityOxygen = gConfig->data["player"]["infinityOxygen"].get<bool>();
                            if (ImGui::Checkbox("Infinity Oxygen ##Another Player", &infinityOxygen)) {
                                gConfig->data["player"]["infinityOxygen"] = infinityOxygen;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static bool antiBunnyHop = gConfig->data["player"]["antiBunnyHop"].get<bool>();
                            if (ImGui::Checkbox("AntiBunnyHop ##Another Player", &antiBunnyHop)) {
                                gConfig->data["player"]["antiBunnyHop"] = antiBunnyHop;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static bool noFall = gConfig->data["player"]["noFall"].get<bool>();
                            if (ImGui::Checkbox("No Fall ##Another Player", &noFall)) {
                                gConfig->data["player"]["noFall"] = noFall;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static bool fastRespawn = gConfig->data["player"]["fastRespawn"].get<bool>();
                            if (ImGui::Checkbox("Fast Respawn ##Another Player", &fastRespawn)) {
                                gConfig->data["player"]["fastRespawn"] = fastRespawn;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static bool antiFreeze = gConfig->data["player"]["antiFreeze"].get<bool>();
                            if (ImGui::Checkbox("Server Control NOP ##Another Player", &antiFreeze)) {
                                gConfig->data["player"]["antiFreeze"] = antiFreeze;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));
                }
                else if (CURRENT_STATE == PLUGIN_STATES::VEHICLE) {
                    if (ImGui::BeginChild("##Flip + Fast Exit + Drift", ImVec2(secondMenuWidth - 80, 105), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##Flip + Fast Exit + Drift", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool flip = gConfig->data["vehicle"]["flip"]["state"].get<bool>();
                            if (ImGui::Checkbox("Flip ##Flip", &flip)) {
                                gConfig->data["vehicle"]["flip"]["state"] = flip;
                                gConfig->save();
                            }

                            ImGui::SameLine();

                            ImGuiWidgets->ShowHelpMarker("FLIP_TOOLTIP", "Переворот машины по нажатию клавиши (set position)");

                            ImGui::Dummy(ImVec2(0.f, 6.f));

                            static bool fastExit = gConfig->data["vehicle"]["fastExit"]["state"].get<bool>();
                            if (ImGui::Checkbox("Fast Exit ##FastExit", &fastExit)) {
                                gConfig->data["vehicle"]["fastExit"]["state"] = fastExit;
                                gConfig->save();
                            }

                            ImGui::SameLine();

                            ImGuiWidgets->ShowHelpMarker("FASTEXIT_TOOLTIP", "Позволяет быстро покинуть машину,\nпредотвращая проигрывание анимации");

                            ImGui::Dummy(ImVec2(0.f, 6.f));

                            static bool drift = gConfig->data["vehicle"]["drift"]["state"].get<bool>();
                            if (ImGui::Checkbox("Дрифт ##Drift", &drift)) {
                                gConfig->data["vehicle"]["drift"]["state"] = drift;
                                gConfig->save();
                            }

                            ImGui::SameLine();

                            ImGuiWidgets->ShowHelpMarker("DRIFT_TOOLTIP", "Чтобы использовать, зажмите выбранную клавишу, и затем\nнажимайте клавиши \"A\" | \"D\" для поворота/ухода в дрифт");

                            ImGui::TableSetColumnIndex(1);

                            ImGui::Dummy(ImVec2(0.f, 2.f));

                            static ImGuiKey flipKey = gConfig->data["vehicle"]["flip"]["button"].get<ImGuiKey>();
                            if (gHotKey->Hotkey("Flip Hotkey", flipKey, ImVec2(170, 25))) {
                                gConfig->data["vehicle"]["flip"]["button"] = ImGui::GetKeyIndex(flipKey);
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 6.f));

                            static ImGuiKey exitKey = gConfig->data["vehicle"]["fastExit"]["button"].get<ImGuiKey>();
                            if (gHotKey->Hotkey("Fast Exit Hotkey", exitKey, ImVec2(170, 25))) {
                                gConfig->data["vehicle"]["fastExit"]["button"] = ImGui::GetKeyIndex(exitKey);
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 6.f));

                            static ImGuiKey driftKey = gConfig->data["vehicle"]["drift"]["button"].get<ImGuiKey>();
                            if (gHotKey->Hotkey("Drift Hotkey", driftKey, ImVec2(170, 25))) {
                                gConfig->data["vehicle"]["drift"]["button"] = ImGui::GetKeyIndex(driftKey);
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##Godmode", ImVec2(secondMenuWidth - 80, 39), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##Godmode", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool GM = gConfig->data["vehicle"]["godmode"]["state"].get<bool>();
                            if (ImGui::Checkbox("GodMode ##Godmode", &GM)) {
                                gConfig->data["vehicle"]["godmode"]["state"] = GM;
                                gConfig->save();
                            }

                            ImGui::SameLine();

                            ImGuiWidgets->ShowHelpMarker("GODMODECAR_TOOLTIP", "Держит ваш автомобиль в целом состоянии");

                            ImGui::TableSetColumnIndex(1);

                            static bool constHpGM = gConfig->data["vehicle"]["godmode"]["constHp"].get<bool>();
                            if (ImGui::Checkbox("Const HP ##Godmode", &constHpGM)) {
                                gConfig->data["vehicle"]["godmode"]["constHp"] = constHpGM;
                                gConfig->save();
                            }

                            ImGui::SameLine();

                            ImGuiWidgets->ShowHelpMarker("GMCAR_TOOLTIP", "Держит здоровье вашего транспорта на отметке 1000 [/dl]\nРаботает только в связке с GodMode");

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##NoBike", ImVec2(secondMenuWidth - 80, 39), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##NoBike", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool noBike = gConfig->data["vehicle"]["noBike"]["state"].get<bool>();
                            if (ImGui::Checkbox("NoBike ##NoBike", &noBike)) {
                                gConfig->data["vehicle"]["noBike"]["state"] = noBike;
                                gConfig->save();
                            }

                            ImGui::SameLine();

                            ImGuiWidgets->ShowHelpMarker("NOBIKE_TOOLTIP", "Не дает вам упасть с мотоцикла/мопеда/велосипеда");

                            ImGui::TableSetColumnIndex(1);

                            static bool turnOffInWater = gConfig->data["vehicle"]["noBike"]["turnOffInWater"].get<bool>();
                            if (ImGui::Checkbox("AntiWater ##NoBike", &turnOffInWater)) {
                                gConfig->data["vehicle"]["noBike"]["turnOffInWater"] = turnOffInWater;
                                gConfig->save();
                            }

                            ImGui::SameLine();

                            ImGuiWidgets->ShowHelpMarker("NOBIKEWATER_TOOLTIP", "Отключает NoBike, если вы окажетесь в воде\n[Используется против умных администраторов]");

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##SpeedHack", ImVec2(secondMenuWidth - 80, 105), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##SpeedHack", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            ImGui::SetCursorPosY(38.f);

                            static bool speedhack = gConfig->data["vehicle"]["speedhack"]["state"].get<bool>();
                            if (ImGui::Checkbox("SpeedHack [Alt] ##SpeedHack", &speedhack)) {
                                gConfig->data["vehicle"]["speedhack"]["state"] = speedhack;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static float speedhackAccuracy = gConfig->data["vehicle"]["speedhack"]["accuracy"].get<float>();
                            std::pair<float, bool> resultSpeedhackAccuracy = ImGuiWidgets->CustomSliderFloat("Accuracy", speedhackAccuracy, 0.1f, 1.f, "%1.1f", 17, 130);
                            if (resultSpeedhackAccuracy.second) {
                                gConfig->data["vehicle"]["speedhack"]["accuracy"] = resultSpeedhackAccuracy.first;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 30.f));

                            static float speedhackMaxSpeed = gConfig->data["vehicle"]["speedhack"]["maxSpeed"].get<float>();
                            std::pair<float, bool> resultSpeedhackMaxSpeed = ImGuiWidgets->CustomSliderFloat("Max Speed", speedhackMaxSpeed, 20.f, 500.f, "%.0f", 18, 130);
                            if (resultSpeedhackMaxSpeed.second) {
                                gConfig->data["vehicle"]["speedhack"]["maxSpeed"] = resultSpeedhackMaxSpeed.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##JumpCar", ImVec2(secondMenuWidth - 80, 88), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##JumpCar", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            ImGui::SetCursorPosY(29.f);

                            static bool jumpCar = gConfig->data["vehicle"]["jumpCar"]["state"].get<bool>();
                            if (ImGui::Checkbox("JumpCar ##JumpCar", &jumpCar)) {
                                gConfig->data["vehicle"]["jumpCar"]["state"] = jumpCar;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static ImGuiKey jumpCarButton = gConfig->data["vehicle"]["jumpCar"]["button"].get<ImGuiKey>();
                            if (gHotKey->Hotkey("JumpCar Hotkey", jumpCarButton, ImVec2(140, 25))) {
                                gConfig->data["vehicle"]["jumpCar"]["button"] = ImGui::GetKeyIndex(jumpCarButton);
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 8.f));

                            static float zPositionJumpCar = gConfig->data["vehicle"]["jumpCar"]["zPosition"].get<float>();
                            std::pair<float, bool> resultZPositionJumpCar = ImGuiWidgets->CustomSliderFloat("+Z Position", zPositionJumpCar, 0.2f, 2.f, "%.1f", 19, 130);
                            if (resultZPositionJumpCar.second) {
                                gConfig->data["vehicle"]["jumpCar"]["zPosition"] = resultZPositionJumpCar.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##AdditionalVehicleFunctions", ImVec2(secondMenuWidth - 80, 69), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##AdditionalVehicleFunctions", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool underwaterDrive = gConfig->data["vehicle"]["underwaterDrive"]["state"].get<bool>();
                            if (ImGui::Checkbox("Подводная езда ##UnderwaterDrive", &underwaterDrive)) {
                                gConfig->data["vehicle"]["underwaterDrive"]["state"] = underwaterDrive;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static bool bigJumpBMX = gConfig->data["vehicle"]["bigJumpBMX"]["state"].get<bool>();
                            if (ImGui::Checkbox("BigJump BMX ##BixJumpBMX", &bigJumpBMX)) {
                                gConfig->data["vehicle"]["bigJumpBMX"]["state"] = bigJumpBMX;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static bool overwaterDrive = gConfig->data["vehicle"]["overwaterDrive"]["state"].get<bool>();
                            if (ImGui::Checkbox("Езда по воде ##OverwaterDrive", &overwaterDrive)) {
                                gConfig->data["vehicle"]["overwaterDrive"]["state"] = overwaterDrive;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));
                }
                else if (CURRENT_STATE == PLUGIN_STATES::VISUAL) {
                    if (ImGui::BeginChild("##DistanceWallhack", ImVec2(secondMenuWidth - 80, 55), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##DistanceWallhack", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            ImGui::SetCursorPosX(28.f);
                            ImGui::Text("Максимальная");
                            ImGui::SetCursorPosX(46.f);
                            ImGui::Text("Дистанция");

                            ImGui::TableSetColumnIndex(1);

                            static float wallhackDistance = gConfig->data["visual"]["distance"].get<float>();
                            std::pair<float, bool> resultWallhackDistance = ImGuiWidgets->CustomSliderFloat("", wallhackDistance, 5.f, 500.f, "%.0f метров", 16, 130);
                            if (resultWallhackDistance.second) {
                                gConfig->data["visual"]["distance"] = resultWallhackDistance.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##Chams", ImVec2(secondMenuWidth - 80, 39), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##Chams", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool chams = gConfig->data["visual"]["chams"]["state"].get<bool>();
                            if (ImGui::Checkbox("Общая обводка ##Chams", &chams)) {
                                gConfig->data["visual"]["chams"]["state"] = chams;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static const char* chamsType[] = { "Невидимый", "Фиксированный" };
                            static int currentChamsType = gConfig->data["visual"]["chams"]["type"].get<int>();
                            if (ImGui::Combo("Тип ##Chams", &currentChamsType, chamsType, IM_ARRAYSIZE(chamsType))) {
                                gConfig->data["visual"]["chams"]["type"] = currentChamsType;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##Boxes", ImVec2(secondMenuWidth - 80, 39), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##Boxes", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool boxes = gConfig->data["visual"]["boxes"]["state"].get<bool>();
                            if (ImGui::Checkbox("Boxes ##Boxes", &boxes)) {
                                gConfig->data["visual"]["boxes"]["state"] = boxes;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static const char* boxesType[] = { "3D", "2D" };
                            static int currentBoxesType = gConfig->data["visual"]["boxes"]["type"].get<int>();
                            if (ImGui::Combo("Type ##Boxes", &currentBoxesType, boxesType, IM_ARRAYSIZE(boxesType))) {
                                gConfig->data["visual"]["boxes"]["type"] = currentBoxesType;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##Skeleton", ImVec2(secondMenuWidth - 80, 55), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##Skeleton", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            ImGui::SetCursorPosY(13.5f);

                            static bool skeleton = gConfig->data["visual"]["skeleton"]["state"].get<bool>();
                            if (ImGui::Checkbox("Скелетон ##Skeleton", &skeleton)) {
                                gConfig->data["visual"]["skeleton"]["state"] = skeleton;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static float skeletonWidth = gConfig->data["visual"]["skeleton"]["width"].get<float>();
                            std::pair<float, bool> resultSkeletonWidth = ImGuiWidgets->CustomSliderFloat("Длина линий", skeletonWidth, 0.5f, 3.f, "%1.1f", 14, 130);
                            if (resultSkeletonWidth.second) {
                                gConfig->data["visual"]["skeleton"]["width"] = resultSkeletonWidth.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##Lines", ImVec2(secondMenuWidth - 80, 68), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##Lines", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool lines = gConfig->data["visual"]["lines"]["state"].get<bool>();
                            if (ImGui::Checkbox("Линии ##Lines", &lines)) {
                                gConfig->data["visual"]["lines"]["state"] = lines;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            static bool point = gConfig->data["visual"]["lines"]["point"].get<bool>();
                            if (ImGui::Checkbox("Круг на игроке ##Lines", &point)) {
                                gConfig->data["visual"]["lines"]["point"] = point;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            ImGui::Dummy(ImVec2(0.f, 5.f));

                            static float linesWidth = gConfig->data["visual"]["lines"]["width"].get<float>();
                            std::pair<float, bool> resultLinesWidth = ImGuiWidgets->CustomSliderFloat("Длина линии", linesWidth, 0.5f, 3.f, "%1.1f", 15, 130);
                            if (resultLinesWidth.second) {
                                gConfig->data["visual"]["lines"]["width"] = resultLinesWidth.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##Nametags", ImVec2(secondMenuWidth - 80, 39), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##Nametags", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool nametags = gConfig->data["visual"]["nametags"]["state"].get<bool>();
                            if (ImGui::Checkbox("Nametags ##Nametags", &nametags)) {
                                gConfig->data["visual"]["nametags"]["state"] = nametags;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static bool defaultNametags = gConfig->data["visual"]["nametags"]["defaultNametags"].get<bool>();
                            if (ImGui::Checkbox("Default Nametags ##Nametags", &defaultNametags)) {
                                gConfig->data["visual"]["nametags"]["defaultNametags"] = defaultNametags;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));
                }
                else if (CURRENT_STATE == PLUGIN_STATES::ADDITIONAL) {
                    if (ImGui::BeginChild("##Crosshair", ImVec2(secondMenuWidth - 80, 39), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##Crosshair", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static bool customCrosshair = gConfig->data["additional"]["crosshair"]["state"].get<bool>();
                            if (ImGui::Checkbox("Custom Crosshair ##Crosshair", &customCrosshair)) {
                                gConfig->data["additional"]["crosshair"]["state"] = customCrosshair;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static const char* crosshairStyles[] = { "Always", "Only Down" };
                            static int currentCrosshairStyle = gConfig->data["additional"]["crosshair"]["style"].get<int>();
                            if (ImGui::Combo("Style ##Crosshair", &currentCrosshairStyle, crosshairStyles, IM_ARRAYSIZE(crosshairStyles))) {
                                gConfig->data["additional"]["crosshair"]["style"] = currentCrosshairStyle;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##SetWeather", ImVec2(secondMenuWidth - 80, 69), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##SetWeather", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            static int weather = gConfig->data["additional"]["server"]["weather"].get<int>();
                            if (ImGui::InputInt("Погода", &weather, 1)) {
                                weather = (weather > 44) ? 0 : (weather < 0) ? 44 : weather;

                                gConfig->data["additional"]["server"]["weather"] = weather;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            if (ImGui::Button("Применить", ImVec2(150, 0))) {
                                CWeather::ForceWeatherNow(static_cast<short>(gConfig->data["additional"]["server"]["weather"]));
                                RakNet::BitStream bs;
                                bs.Write(gConfig->data["additional"]["server"]["time"].get<int>());
                                rakhook::emul_rpc(29, bs);
                            }

                            ImGui::TableSetColumnIndex(1);

                            static int time = gConfig->data["additional"]["server"]["time"].get<int>();
                            if (ImGui::InputInt("Время", &time, 1)) {
                                time = (time > 23) ? 0 : (time < 0) ? 23 : time;

                                gConfig->data["additional"]["server"]["time"] = time;
                                gConfig->save();
                            }

                            ImGui::Dummy(ImVec2(0.f, 3.f));

                            if (ImGui::Button("Ресет", ImVec2(150, 0))) {
                                if (gSampVersion->getSampVersion() == samp_ver::v037r31) {
                                    CWeather::ForceWeatherNow(sampapi::v037r3::RefNetGame()->m_pSettings->m_nWeather);
                                    RakNet::BitStream bs;
                                    bs.Write((int)sampapi::v037r3::RefNetGame()->m_pSettings->m_nWorldTimeHour);
                                    rakhook::emul_rpc(29, bs);
                                }
                                else {
                                    CWeather::ForceWeatherNow(sampapi::v037r1::RefNetGame()->m_pSettings->m_nWeather);
                                    RakNet::BitStream bs;
                                    bs.Write((int)sampapi::v037r1::RefNetGame()->m_pSettings->m_nWorldTimeHour);
                                    rakhook::emul_rpc(29, bs);
                                }
                            }

                            ImGui::SameLine(155.f);
                            ImGuiWidgets->ShowHelpMarker("TIME_AND_WEATHER_TOOLTIP", "Возвращает настройки даты и времени к серверным значениям.");

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##NewFov", ImVec2(secondMenuWidth - 80, 55), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##NewFov", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            ImGui::SetCursorPosY(13.5f);

                            static bool newFOV = gConfig->data["additional"]["FOV"]["state"].get<bool>();
                            if (ImGui::Checkbox("FOV ##NewFov", &newFOV)) {
                                gConfig->data["additional"]["FOV"]["state"] = newFOV;
                                gConfig->save();
                            }

                            ImGui::TableSetColumnIndex(1);

                            static float fovNum = gConfig->data["additional"]["FOV"]["num"].get<float>();
                            std::pair<float, bool> resultFovNum = ImGuiWidgets->CustomSliderFloat("FOV num", fovNum, 0.f, 179.f, "%.0f", 20, 130);
                            if (resultFovNum.second) {
                                gConfig->data["additional"]["FOV"]["num"] = resultFovNum.first;
                                gConfig->save();
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));

                    if (ImGui::BeginChild("##Marker", ImVec2(secondMenuWidth - 80, 39), false)) {
                        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 6, ImGui::GetCursorPosY() + 4));

                        if (ImGui::BeginTable("##Marker", 2)) {
                            ImGui::TableNextRow();

                            ImGui::TableSetColumnIndex(0);

                            ImGui::SetCursorPosY(8);
                            ImGui::Text(("Маркер " + std::string((*(int*)0xBA6774 != 0) ? "установлен" : "не установлен")).c_str());

                            ImGui::TableSetColumnIndex(1);

                            if (ImGui::Button("Телепортироваться")) {
                                for (int i = 0; i < (0xAF * 0x28); i += 0x28)
                                    if (*(short*)(0xBA873D + i) == 4611)
                                    {
                                        float* pos = (float*)(0xBA86F8 + 0x28 + i);
                                        FindPlayerPed()->Teleport(CVector(*pos, *(pos + 1), CWorld::FindGroundZForCoord(pos[0], pos[1]) + 2.0f), false);
                                    }
                            }

                            ImGui::EndTable();
                        }
                    }
                    ImGui::EndChild();

                    ImGui::Dummy(ImVec2(0.f, 6.f));
                }
                else if (CURRENT_STATE == PLUGIN_STATES::DESTROY) {
                    ImGui::Text("В разработке..");
                }
                ImGui::PopFont();
                ImGui::PopStyleColor(1);
            }

            ImGui::EndChild();
        }
        ImGui::EndChild();
    }

    ImGui::End();
}

void imguiDraw::RenderButtons(std::vector<std::pair<PLUGIN_STATES, std::vector<std::string>>> list, PLUGIN_STATES& currentState, float width) {
    ImGui::BeginChild("ButtonChild", ImVec2(160, 220), false);

    ImGui::PushFont(variables.fonts[1]);
    for (auto& item : list) {
        if (currentState == item.first) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.48f, 0.48f, 0.48f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.50f, 0.50f, 0.50f, 0.3f));

            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.122f, 0.157f, 0.200f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.270f, 0.635f, 0.620f, 0.3f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.400f, 0.988f, 0.945f, 0.3f));

            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
        }

        if (ImGuiWidgets->AnimButton((item.second[1]).c_str(), ImVec2(width, 35.f), { 1.0f, 0.7f })) {
            currentState = item.first;
        }
        ImGui::PopStyleColor(3);
        ImGui::PopStyleVar(1);
    }
    ImGui::PopFont();

    ImGui::EndChild();
}