#include "EncoreDebug.h"

#include "assets.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "raymath.h"
#include "easing/easing.h"
#include "gameplay/inputCallbacks.h"
#include "users/playerManager.h"
#include "misc/imgui_stdlib.h"
#include "settings/settings.h"
#include "util/frame-manager.h"
#include "gameplay/trackRenderer/Track.h"
#include "../menus/gameplay/GameplayMenu.h"
#include "menus/MenuManager.h"
#include "menus/gameplay/ReadyUpMenu.h"
#include "menus/gameplay/resultsMenu.h"
#include "menus/util/locale/Locale.h"
#include "song/audio.h"
#include "song/song.h"
#include "song/songlist.h"
#include "users/profiles/ProfileManager.h"

bool EncoreDebug::showDebug = false;
bool EncoreDebug::reloadQueued = false;
bool EncoreDebug::reloadFonts = false;
bool EncoreDebug::showGameplayHud = true;

bool EncoreDebug::showEngineWindow[MAX_PLAYERS] = { false, false, false, false };

bool showDemoWindow = false;
bool showAssets = false;
bool showPlayerManager = false;
bool showColorProfileManager = false;
bool showSongList = false;
bool showQuickSettings = false;
bool showPractice = false;
bool showEasings = false;
bool showJoystickTools = false;
bool showLocaleDebug = false;
bool showLog = false;


bool paused = false;
std::string pauseText = "Pause";

std::string debugVersionHash = "";

using namespace ImGui;

void ColorEdit(const char *label, Color *color, ImGuiColorEditFlags flags) {
    float floats[3] = { color->r / 255.0f, color->g / 255.0f, color->b / 255.0f };

    ColorEdit3(label, (float *)&floats, flags);

    color->r = floats[0] * 255;
    color->g = floats[1] * 255;
    color->b = floats[2] * 255;
}

ImVec4 ImGuiColor(Color color) {
    return { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f };
}

void DrawEasingsWindow() {
    if (Begin("Easing Functions")) {
        static easing_functions func = Linear;
        if (BeginCombo("Function", getEasingFunctionName(func), 0)) {
            for (unsigned int iter = Linear; iter <= EaseInOutBounce; iter++) {
                if (Selectable(
                    getEasingFunctionName((easing_functions)iter),
                    func == iter
                )) {
                    func = (easing_functions)iter;
                }
            }
            EndCombo();
        }

        const unsigned int count = 100;
        static float values[count] = { 0 };
        auto easingFunc = getEasingFunction(func);
        float min = 1000.f;
        float max = -1000.0f;
        for (unsigned int i = 0; i < count; i++) {
            double f = (double)i / (double)count;
            values[i] = easingFunc(f);
            if (values[i] > max) {
                max = values[i];
            }
            if (values[i] < min) {
                min = values[i];
            }
        }
        PlotLines(
            "Values",
            values,
            count,
            0,
            0,
            min,
            max,
            { 0.0f, GetContentRegionAvail().y }
        );
    }
    End();
}

void EncoreDebug::DrawDebug() {
    ZoneScoped;
    if (debugVersionHash.empty()) {
        debugVersionHash = TextFormat(
            "Encore %s-%s:%s",
            ENCORE_VERSION,
            GIT_COMMIT_HASH,
            GIT_BRANCH
        );
    }
    MenuBar();
    if (showAssets) {
        DrawAssetViewer();
    }
    if (showDemoWindow) {
        ShowDemoWindow(&showDemoWindow);
    }
    if (showPlayerManager) {
        DrawPlayerManager();
    }
    if (showSongList && !dynamic_cast<GameplayMenu *>(TheMenuManager.ActiveMenu.get())) {
        DrawSongList();
    }
    if (showQuickSettings) {
        DrawQuickSettings();
    }
    if (showColorProfileManager) {
        DrawColorProfileSettings();
    }
    if (showPractice) {
        DrawPracticeSectionSelector();
    }
    DrawSongScrubber();
    if (showEasings) {
        DrawEasingsWindow();
    }
    if (showJoystickTools) {
        DrawJoystickTools();
    }
    if (showLocaleDebug) {
        DrawLocaleDebug();
    }
    if (showLog) {
        DrawLog();
    }
}

void EncoreDebug::MenuBar() {
    ZoneScoped;
    auto gameplayMenu = TheMenuManager.GetActiveMenu<GameplayMenu>();
    BeginMainMenuBar();

    if (BeginMenu("Config")) {
        MenuItem("Quick Settings", 0, &showQuickSettings);
        MenuItem("Player Manager", 0, &showPlayerManager);
        MenuItem("Color Profile Manager", 0, &showColorProfileManager);
        EndMenu();
    }

    if (BeginMenu("Tools")) {
        MenuItem("Joystick Tools", 0, &showJoystickTools);
        MenuItem("ImGui Demo Window", 0, &showDemoWindow);
        MenuItem("Log", 0, &showLog);
        EndMenu();
    }

    if (BeginMenu("Dev")) {
        MenuItem("Assets", 0, &showAssets);
        MenuItem("Song List", 0, &showSongList, !gameplayMenu);
        MenuItem("Easings Debug", 0, &showEasings);
        if (Encore::Locale::unlocalizedTokens.empty()) {
            MenuItem("Locale Debug", 0, &showLocaleDebug);
        } else {
            std::size_t localeErrors = Encore::Locale::unlocalizedTokens.size();
            MenuItem(std::vformat("Locale Debug (!!! {} unlocalized tokens)",
                                  std::make_format_args(localeErrors)).c_str(),
                     0,
                     &showLocaleDebug);
        }
        EndMenu();
    }

    if (gameplayMenu) {
        if (BeginMenu("Gameplay")) {
            if (MenuItem(pauseText.c_str())) {
                paused = !paused;
                for (auto player : ThePlayerManager.ActivePlayers) {
                    if (!player)
                        continue;
                    player->engine->stats->Paused = paused;
                }
                if (paused) {
                    pauseText = "Resume";
                    TheAudioManager.pauseStreams();
                } else {
                    pauseText = "Pause";
                    TheAudioManager.unpauseStreams();
                }
            }
            if (MenuItem("End Song")) {
                TheAudioManager.unloadStreams();
                TheSongTime.FullReset();
                TheMenuManager.CreateAndSwitchMenu<resultsMenu>(gameplayMenu->curSong);
            }
            MenuItem("Practice", 0, &showPractice);
            MenuItem("Show Hud", 0, &showGameplayHud);
            if (BeginMenu("Active Stats/Engines")) {
                for (int p = 0; p < ThePlayerManager.PlayersActive; p++) {
                    MenuItem(ThePlayerManager.GetActivePlayer(p).Name.c_str(),
                             0,
                             &showEngineWindow[p]);
                }
                EndMenu();
            }
            EndMenu();
        }
    }
    auto avail = GetWindowWidth();
    auto size = CalcTextSize(debugVersionHash.c_str()).x;
    auto fpsSize = CalcTextSize("00000 FPS").x;
    auto realFpsSize = CalcTextSize(TextFormat("%i FPS", GetFPS())).x;

    SetCursorPosX(avail - realFpsSize - GetStyle().FramePadding.x);
    ImGui::Text("%i FPS", GetFPS());
    SetCursorPosX(avail - size - fpsSize - GetStyle().FramePadding.x);
    ImGui::Text("%s", debugVersionHash.c_str());

    EndMainMenuBar();
}

void EncoreDebug::DrawColorProfileSettings() {
    ZoneScoped;
    if (Begin("Color Profiles", &showColorProfileManager, 0)) {
        if (Button("Save All")) {
            TheProfileManager.SaveColorProfiles();
        }

        if (BeginTabBar("Profiles")) {
            for (auto &profile : TheProfileManager.ColorProfiles) {
                if (BeginTabItem(
                    (profile.second.Name + TextFormat("###%x", &profile.second.Name)).
                    c_str())) {
                    bool disabled = false;
                    if (profile.second.builtin) {
                        ImGui::Text("%s", "Cannot edit default color profile.");
                        BeginDisabled();
                        disabled = true;
                    }
                    InputText("Profile Name", &profile.second.Name);
                    ColorEdit("Overdrive",
                              &profile.second.colors[Encore::SLOT_OVERDRIVE],
                              0);
                    ColorEdit("Note Frame",
                              &profile.second.colors[Encore::SLOT_FRAME],
                              0);
                    ColorEdit("Overdrive Note Frame",
                              &profile.second.colors[Encore::SLOT_FRAME_OVERDRIVE],
                              0);
                    SeparatorText(" ");
                    ColorEdit("Green",
                              &profile.second.colors[Encore::SLOT_GREEN],
                              0);
                    ColorEdit("Red",
                              &profile.second.colors[Encore::SLOT_RED],
                              0);
                    ColorEdit("Yellow",
                              &profile.second.colors[Encore::SLOT_YELLOW],
                              0);
                    ColorEdit("Blue",
                              &profile.second.colors[Encore::SLOT_BLUE],
                              0);
                    ColorEdit("Orange",
                              &profile.second.colors[Encore::SLOT_ORANGE],
                              0);
                    ColorEdit("Open",
                              &profile.second.colors[Encore::SLOT_OPEN],
                              0);
                    SeparatorText("Drums Colors");
                    ColorEdit("Kick",
                              &profile.second.colors[Encore::SLOT_KICK],
                              0);
                    ColorEdit("Yellow Cymbal",
                              &profile.second.colors[Encore::SLOT_HIHAT],
                              0);
                    ColorEdit("Blue Cymbal",
                              &profile.second.colors[Encore::SLOT_RIDE],
                              0);
                    ColorEdit("Green Cymbal",
                              &profile.second.colors[Encore::SLOT_CRASH],
                              0);

                    if (disabled) {
                        EndDisabled();
                    }
                    EndTabItem();
                    // if (Button("Delete Player")) {
                    //    ThePlayerManager.SavePlayerList();
                    // }
                }
            }
            if (TabItemButton("New", ImGuiTabItemFlags_Trailing)) {
                TheProfileManager.CreateColorProfile();
            }
        }
        EndTabBar();
    }
    End();
}

void EncoreDebug::DrawQuickSettings() {
    ZoneScoped;
    if (Begin("Quick Settings", &showQuickSettings)) {
        SliderFloat("Song Speed", &TheAudioManager.songSpeed, 0, 3);
        SliderFloat("Debug Song Speed", &TheAudioManager.debugSpeed, 0, 3);
        Checkbox("Uncap Framerate", &TheFrameManager.removeFPSLimit);
        Checkbox("VSync", &TheGameSettings.VerticalSync);
        SliderInt("Menu FPS", &TheFrameManager.menuFPS, 1, 300);
        SliderInt("Gameplay FPS", &TheGameSettings.Framerate, 1, 1500);
        SliderInt("Controller Poll Rate", &controllerPollRate, 10, 1000, "%dhz");
        if (DragInt("Audio Calibration", &TheGameSettings.AudioOffset, 1, 0, 0, "%dms")) {
            TheSongTime.SetOffset(TheGameSettings.AudioOffset / 1000.0);
        }
        DragInt("Video Calibration", &TheGameSettings.VideoOffset, 1, 0, 0, "%dms");

        ImGui::Text("Audio Settings");
        SliderFloat("Main Volume", &TheGameSettings.avMainVolume, 0.0, 1.0);
        if (CollapsingHeader("Advanced")) {
            SliderFloat("Active Inst Volume",
                        &TheGameSettings.avActiveInstrumentVolume,
                        0.0,
                        1.0);
            SliderFloat("Track Mute Volume", &TheGameSettings.avMuteVolume, 0.0, 1.0);
            SliderFloat("Inactive Inst Volume",
                        &TheGameSettings.avInactiveInstrumentVolume,
                        0.0,
                        1.0);
            SliderFloat("Inactive Vocals Volume",
                        &TheGameSettings.avInactiveVocalsVolume,
                        0.0,
                        1.0);
            SliderFloat("Crowd Volume", &TheGameSettings.avCrowdVolume, 0.0, 1.0);
            SliderFloat("SFX Volume", &TheGameSettings.avSoundEffectVolume, 0.0, 1.0);
            SliderFloat("Menu Music Volume",
                        &TheGameSettings.avMenuMusicVolume,
                        0.0,
                        1.0);
        }
        if (Button("Save Settings")) {
            TheGameSettings.SaveToFile(
                (TheGameSettings.directory / "settings.json").string());
        }
    }
    End();
}

void DebugSeek(float time, float audioTime) {
    TheAudioManager.seekStreams(audioTime);
    for (auto player : ThePlayerManager.ActivePlayers) {
        if (!player)
            continue;
        auto engine = player->engine.get();
        engine->chart->MissedNotePointers.clear();
        for (size_t i = 0; i < engine->chart->CurrentNoteIterators.size(); i++) {
            if (i >= engine->chart->Lanes.size()) {
                break;
            }
            for (auto iter = engine->chart->Lanes[i].begin(); iter < engine->chart->Lanes[
                     i].end(); ++iter) {
                if (iter->start.sec > time) {
                    engine->chart->CurrentNoteIterators[i] = iter;
                    break;
                }
            }
        }
    }
    TheSongTime.CurrentBeatline = 0;
    TheSongTime.UpdateBeatlines();
}


void EncoreDebug::DrawPracticeSectionSelector() {
    ZoneScoped;
    bool isGameplay = dynamic_cast<GameplayMenu *>(TheMenuManager.ActiveMenu.get()) !=
        nullptr;
    if (isGameplay) {
        if (Begin("Practice Section Selector")) {
            for (size_t sectionInt = 0; sectionInt < TheSongTime.Sections.size();
                 sectionInt++) {
                ImGui::Text("%s", TheSongTime.Sections.at(sectionInt).name.c_str());
                SameLine();
                float buttWidth = CalcTextSize(" whole").x;
                SetCursorPosX(GetWindowWidth() - (buttWidth * 3));
                PushID(sectionInt);
                if (Button("whole")) {
                    double startTime;
                    for (auto player : ThePlayerManager.ActivePlayers) {
                        if (!player)
                            continue;
                        double endTime = 0.0;
                        startTime = TheSongTime.Sections.at(sectionInt).start;
                        if (sectionInt == TheSongTime.Sections.size() - 1)
                            endTime = TheSongTime.GetSongLength();
                        else
                            endTime = TheSongTime.Sections.at(sectionInt + 1).start;
                        player->engine->pStartTime = startTime - 0.1;
                        player->engine->pStopTime = endTime;
                        player->engine->practice = true;
                    }
                    DebugSeek(startTime, startTime - 2);
                }
                SameLine();
                if (Button("start")) {
                    double startTime;
                    for (auto player : ThePlayerManager.ActivePlayers) {
                        if (!player)
                            continue;
                        startTime = TheSongTime.Sections.at(sectionInt).start;
                        player->engine->pStartTime = startTime - 0.1;
                        player->engine->pStopTime = TheSongTime.GetSongLength();
                        player->engine->practice = true;
                    }
                    DebugSeek(startTime, startTime - 2);
                }
                SameLine();
                if (Button("end")) {
                    for (auto player : ThePlayerManager.ActivePlayers) {
                        if (!player)
                            continue;
                        double endTime = 0.0;
                        if (sectionInt == TheSongTime.Sections.size() - 1)
                            endTime = TheSongTime.GetSongLength();
                        else
                            endTime = TheSongTime.Sections.at(sectionInt + 1).start;
                        player->engine->pStopTime = endTime;
                        player->engine->practice = true;
                    }
                }
                PopID();
            }
        }
        End();
    }
}

struct TimelineTextSpacer {
    float startPos;
    float endPos;
    int layer;
};

void EncoreDebug::DrawSongScrubber() {
    ZoneScoped;
    bool isGameplay = dynamic_cast<GameplayMenu *>(TheMenuManager.ActiveMenu.get()) !=
        nullptr;
    if (isGameplay) {
        SetNextWindowPos({ 0, GetFrameHeight() + 4 }, ImGuiCond_Always);
        SetNextWindowSize({ ImGui::GetIO().DisplaySize.x, 0 }, ImGuiCond_Always);
        if (Begin("Song Scrubber",
                  0,
                  ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize |
                  ImGuiWindowFlags_NoMove)) {
            PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            float time = TheSongTime.GetElapsedTime();
            auto size = GetContentRegionAvail();
            size.y = 24;
            auto pos = GetCursorScreenPos();
            InvisibleButton("Song Scrubber Button", size);
            auto GetTimeAtPos = [&](float x) {
                return (x / size.x) * TheSongTime.GetSongLength();
            };
            auto TimeToPos = [&](double time) {
                return (time / TheSongTime.GetSongLength()) * size.x;
            };
            auto GetMouseLocalPos = [&]() {
                return GetMousePos().x - pos.x;
            };
            if (IsItemHovered() && IsMouseButtonDown(ImGuiMouseButton_Left)) {
                DebugSeek(GetTimeAtPos(GetMouseLocalPos()),
                          GetTimeAtPos(GetMouseLocalPos()));
            }
            if (IsItemHovered() && IsMouseButtonDown(ImGuiMouseButton_Right)) {
                TheAudioManager.seekStreams(GetTimeAtPos(GetMouseLocalPos()));
            }
            auto drawlist = GetWindowDrawList();
            drawlist->AddRectFilled(pos,
                                    pos + size,
                                    ColorConvertFloat4ToU32(
                                        GetStyle().Colors[
                                            IsItemHovered() && !IsItemActive()
                                            ? ImGuiCol_FrameBgHovered
                                            : ImGuiCol_FrameBg]));
            drawlist->AddLine(pos + ImVec2(TimeToPos(time), 0),
                              pos + ImVec2(TimeToPos(time), size.y),
                              ColorConvertFloat4ToU32({ 1, 0, 1, 1 }),
                              2);

            static std::vector<TimelineTextSpacer> texts = {};
            texts.clear();
            auto occupied = [&](float x, int layer) {
                if (layer < 0) {
                    return true;
                }
                for (auto &spacer : texts) {
                    if (x > spacer.startPos && x < spacer.endPos && spacer.layer ==
                        layer) {
                        return true;
                    }
                }
                return false;
            };
            size_t layer = 0;
            size_t maxlayer = 0;
            for (size_t i = 0; i < TheSongTime.Sections.size(); i++) {
                auto &section = TheSongTime.Sections[i];
                float endTime = TheSongTime.GetSongLength();
                if (i < TheSongTime.Sections.size() - 1) {
                    endTime = TheSongTime.Sections[i + 1].start;
                }
                float rectPos = TimeToPos(section.start);
                float startPos = TimeToPos(section.start);
                float textWide = MeasureText(section.name.c_str(), GetFontSize());
                if (startPos + textWide > size.x) {
                    startPos = size.x - textWide;
                }
                drawlist->AddRectFilled(pos + ImVec2(rectPos, 0),
                                        pos + ImVec2(TimeToPos(endTime), size.y),
                                        ColorConvertFloat4ToU32(i % 2 == 0
                                            ? ImVec4{ 1, 1, 1, 0.2 }
                                            : ImVec4{ 1, 1, 1, 0.1 }));
                drawlist->AddLine(pos + ImVec2(rectPos, 0),
                                  pos + ImVec2(rectPos, size.y),
                                  ColorConvertFloat4ToU32({ 1, 1, 1, 0.9 }));
                layer = 0;
                while (occupied(startPos, layer)) {
                    layer++;
                }
                float textY = pos.y + size.y + layer * GetFontSize();
                drawlist->AddText(ImVec2(startPos + pos.x, textY),
                                  0xffffffff,
                                  section.name.c_str());
                texts.push_back(
                    { startPos, startPos + textWide, static_cast<int>(layer) });
                if (layer > maxlayer) {
                    maxlayer = layer;
                }
            }

            Dummy(ImVec2{ 0, GetFontSize() * (maxlayer + 1) });
            PopStyleVar();
        }
        End();
    }
}

void DrawPlayer(std::shared_ptr<Player> &player) {
    if (BeginTabItem(
        (player->Name + "###" + TextFormat("%x", &player)).c_str())) {
        InputText("Username", &player->Name);
        SeparatorText("Color Profile");
        // for some reason, when creating a new profile, player.GetColorProfile() eats shit and dies
        // it doesnt get set to a nullptr??? but it gets set to some fucking random memory address and eugh
        // close this when making a new color profile
        if (BeginCombo("Plastic Color Profile",
                       player->GetColorProfile(Encore::ProfileManager::PLASTIC)->Name.
                               c_str())) {
            for (auto i : TheProfileManager.ColorProfiles) {
                if (Selectable(i.second.Name.c_str())) {
                    player->SetColorProfile(i.second.Name,
                                            Encore::ProfileManager::PLASTIC);
                }
            }
            EndCombo();
        }
        if (BeginCombo("Pad Color Profile",
                       player->GetColorProfile(
                           Encore::ProfileManager::PAD)->Name.c_str())) {
            for (auto i : TheProfileManager.ColorProfiles) {
                if (Selectable(i.second.Name.c_str())) {
                    player->SetColorProfile(i.second.Name, Encore::ProfileManager::PAD);
                }
            }
            EndCombo();
        }
        if (BeginCombo("Drums Color Profile",
                       player->GetColorProfile(Encore::ProfileManager::DRUMS)->Name.
                               c_str())) {
            for (auto i : TheProfileManager.ColorProfiles) {
                if (Selectable(i.second.Name.c_str())) {
                    player->SetColorProfile(i.second.Name, Encore::ProfileManager::DRUMS);
                }
            }
            EndCombo();
        }

        SeparatorText(std::string("Player: " + player->Name).c_str());
        SliderFloat("Note Speed", &player->NoteSpeed, 0, 3);
        SliderFloat("Track Length", &player->HighwayLength, 0, 5);
        int inputOffset = player->InputCalibration * 1000;
        DragInt("Input Calibration",
                &inputOffset,
                1,
                -1000,
                1000,
                "%dms");
        player->InputCalibration = inputOffset / 1000.0;
        ColorEdit("Accent Color", &player->AccentColor, 0);
        Checkbox("Bot", &player->Bot);
        Checkbox("Lefty Flip", &player->LeftyFlip);
        Checkbox("Brutal Mode", &player->BrutalMode);
        EndTabItem();

        if (Button("Delete Player")) {
            ThePlayerManager.DeletePlayer(*player);
            ThePlayerManager.SavePlayerList();
        }
    }
}

bool showActive = true;

void EncoreDebug::DrawPlayerManager() {
    ZoneScoped;
    if (Begin("Player Manager", &showPlayerManager, 0)) {
        if (Button("Save All")) {
            ThePlayerManager.SavePlayerList();
        }
        Checkbox("Show Only Active Players", &showActive);

        if (BeginTabBar("Players")) {
            if (showActive) {
                for (int i = 0; i < MAX_PLAYERS; i++) {
                    if (ThePlayerManager.ActivePlayers[i] == nullptr) {
                        if (i == 0)
                            Text("%s",
                                 "No players are available. Please consider having players join, or disable \"Show Only Active Players\"");
                        break;
                    }
                    DrawPlayer(ThePlayerManager.ActivePlayers[i]);
                }
            } else {
                for (auto &player : ThePlayerManager.PlayerList) {
                    if (player == nullptr) {
                        Text("%s",
                             "No players are available. Please add players using the New button.");
                        break;
                    }
                    DrawPlayer(player);
                }
            }
            if (TabItemButton("New", ImGuiTabItemFlags_Trailing)) {
                ThePlayerManager.CreatePlayer("New Player");
            }
        }
        EndTabBar();
    }
    End();
}

std::string tolowerStr(std::string &in) {
    std::string out;
    for (auto c : in) {
        // Quirk: ignore some characters, makes searching better
        if (c == '\'') {
            continue;
        }
        if (c == ' ') {
            continue;
        }
        if (c == '(') {
            continue;
        }
        if (c == ')') {
            continue;
        }
        if (c == ',') {
            continue;
        }
        out += std::tolower(c);
    }
    return out;
}

void EncoreDebug::DrawJoystickTools() {
    if (Begin("Joystick Tools", &showJoystickTools)) {
        Text("Audio Sync Time: %f", syncAudioTime);
        Text("SDL Tick Time: %f", (float)syncSDLTicks * 0.000000001);
        Text("Last Translated Time: %f", lastTranslatedTime);
        const ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;
        if (BeginTable("Joysticks", 6, flags, GetContentRegionAvail())) {
            TableSetupScrollFreeze(0, 1);
            TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            TableSetupColumn("GUID", ImGuiTableColumnFlags_WidthStretch);
            TableSetupColumn("VID", ImGuiTableColumnFlags_WidthFixed);
            TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed);
            TableSetupColumn("REV", ImGuiTableColumnFlags_WidthFixed);
            TableSetupColumn("Actions", ImGuiTableColumnFlags_WidthStretch);
            TableHeadersRow();

            int joystickCount = 0;
            SDL_JoystickID *joysticks = SDL_GetJoysticks(&joystickCount);

            for (int i = 0; i < joystickCount; i++) {
                SDL_JoystickID joyId = joysticks[i];
                TableNextRow();
                PushID(i);

                TableSetColumnIndex(0);
                auto joystickName = SDL_GetJoystickNameForID(joyId);
                ImGui::Text("%s", joystickName);

                TableSetColumnIndex(1);
                char guidStr[33];
                SDL_GUIDToString(SDL_GetJoystickGUIDForID(joyId), guidStr, 33);
                ImGui::Text("%s", guidStr);

                TableSetColumnIndex(2);
                auto vid = SDL_GetJoystickVendorForID(joyId);
                ImGui::Text("%x", vid);

                TableSetColumnIndex(3);
                auto pid = SDL_GetJoystickProductForID(joyId);
                ImGui::Text("%x", pid);

                TableSetColumnIndex(4);
                auto rev = SDL_GetJoystickProductVersionForID(joyId);
                ImGui::Text("%x", rev);

                TableSetColumnIndex(5);
                if (SmallButton("Copy Name and GUID")) {
                    std::string clipContent = std::string(guidStr) + "," + joystickName;
                    ImGui::SetClipboardText(clipContent.c_str());
                }
                SameLine();
                static std::string mappingStr;
                if (SmallButton("Set Mapping")) {
                    OpenPopup(joystickName);
                    mappingStr = "";
                }
                if (BeginPopupModal(joystickName)) {
                    InputText("Paste mapping", &mappingStr);
                    if (Button("Apply")) {
                        CloseCurrentPopup();
                        SDL_AddGamepadMapping(mappingStr.c_str());
                    }
                    SameLine();
                    if (Button("Cancel")) {
                        CloseCurrentPopup();
                    }

                    EndPopup();
                }

                PopID();
            }
            EndTable();
        }
    }

    End();
}

void EncoreDebug::DrawEngineWindow(int slot) {
    auto &player = ThePlayerManager.GetActivePlayer(slot);
    if (Begin(player.Name.c_str())) {
        if (CollapsingHeader("Engine")) {
            auto engine = player.engine;
            BeginDisabled();
            float time = engine->LastUpdateTime;
            SliderFloat("Last Update Time", &time, time, time);
            time = engine->stats->InputOffset;
            SliderFloat("Input Offset", &time, time, time);
            time = engine->stats->InputTime;
            SliderFloat("Input Time", &time, time, time);
            EndDisabled();

            ProgressBar(engine->whammy, { 1.0f, 0.0f }, "Whammy");
            SeparatorText("Timers");
            for (auto Timer : engine->Timers) {
                for (auto timer : player.engine->Timers) {
                    float countdown = Clamp(
                        (timer.second.Time + timer.second.Duration)
                        - TheSongTime.GetElapsedTime(),
                        0,
                        timer.second.Duration
                    );
                    ProgressBar(countdown / timer.second.Duration,
                                { -FLT_MIN, 0 },
                                TextFormat("%s: %4.4f",
                                           timer.first.c_str(),
                                           countdown));
                };
            }
        }
        if (CollapsingHeader("Stats")) {
            auto stats = player.engine->stats;
        }
        if (CollapsingHeader("Events")) {
        }
        End();
    }
}

void EncoreDebug::DrawLocaleDebug() {
    if (Begin("Locale Debug", &showLocaleDebug)) {
        InputText("Current Locale", &TheGameSettings.Language);
        if (Button("Reload Locale")) {
            Encore::Locale::Init();
        }
        SameLine();
        if (Button("Unload Locale")) {
            Encore::Locale::layers.clear();
            Encore::Locale::unlocalizedTokens.clear();
        }
        Checkbox("Debug Long Strings", &Encore::Locale::debugLongStrings);

        if (BeginTabBar("localeTabs")) {
            if (BeginTabItem("Locale Layers")) {
                for (auto &layer : Encore::Locale::layers) {
                    if (CollapsingHeader(layer.name.c_str())) {
                        if (layer.fallback) {
                            ImGui::Text("Fallback layer, will report unlocalized tokens");
                        }

                        const ImGuiTableFlags flags = ImGuiTableFlags_ScrollY |
                            ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

                        if (BeginTable((std::string("Tokens##") + layer.name).c_str(),
                                       2,
                                       flags,
                                       GetContentRegionAvail())) {
                            TableSetupScrollFreeze(0, 1);
                            TableSetupColumn("Token", ImGuiTableColumnFlags_WidthFixed);
                            TableSetupColumn("String",
                                             ImGuiTableColumnFlags_WidthStretch);
                            TableHeadersRow();

                            for (auto &[key, value] : layer.entries) {
                                TableNextRow();

                                TableSetColumnIndex(0);
                                ImGui::Text("%s", key.c_str());

                                TableSetColumnIndex(1);
                                ImGui::Text("%s", value.c_str());
                            }

                            EndTable();
                        }
                    }
                }

                EndTabItem();
            }

            if (BeginTabItem("Unlocalized Tokens")) {
                for (auto &token : Encore::Locale::unlocalizedTokens) {
                    ImGui::Text("%s", token.c_str());
                }
                EndTabItem();
            }
            EndTabBar();
        }
    }
    End();
}

void EncoreDebug::DrawSongList() {
    ZoneScoped;
    static std::string filter;
    static std::vector<Song *> songs;
    static bool firstTime = true;

    static auto UpdateList = [&] {
        songs.clear();
        std::string lowerFilter = tolowerStr(filter);
        for (size_t i = 0; i < TheSongList.songs.size(); i++) {
            auto song = &TheSongList.songs[i];
            if (filter.empty()) {
                songs.push_back(song);
            } else {
                if (tolowerStr(song->title).find(lowerFilter) != std::string::npos) {
                    songs.push_back(song);
                    continue;
                }
                if (tolowerStr(song->artist).find(lowerFilter) != std::string::npos) {
                    songs.push_back(song);
                    continue;
                }
            }
        }
    };

    if (firstTime) {
        UpdateList();
    }
    if (Begin("Song List", &showSongList)) {
        if (InputText("Filter", &filter)) {
            UpdateList();
        }

        const ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;

        if (BeginTable("Song List", 5, flags, GetContentRegionAvail())) {
            TableSetupScrollFreeze(0, 1);
            TableSetupColumn("##Actions",
                             ImGuiTableColumnFlags_WidthFixed |
                             ImGuiTableColumnFlags_NoResize |
                             ImGuiTableColumnFlags_NoSort);
            TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch);
            TableSetupColumn("Artist", ImGuiTableColumnFlags_WidthFixed);
            TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed);
            TableSetupColumn("Hash", ImGuiTableColumnFlags_WidthFixed);
            TableHeadersRow();

            for (size_t i = 0; i < songs.size(); i++) {
                auto song = songs[i];
                TableNextRow();
                PushID(i);

                TableSetColumnIndex(1);
                ImGui::Text("%s", song->title.c_str());

                TableSetColumnIndex(2);
                ImGui::Text("%s", song->artist.c_str());

                TableSetColumnIndex(3);
                ImGui::Text("%s", song->source.c_str());

                TableSetColumnIndex(4);
                /// WARNING: SLOW AS FUCK!!!!!!
                std::stringstream hashStr;
                hashStr << std::hex;
                for (size_t x = 0; x < sizeof(SongHash::hash); x++) {
                    hashStr << std::setw(2) << std::setfill('0') << (int)song->hash.hash[
                        x];
                }
                ImGui::Text("%s", hashStr.str().c_str());

                TableSetColumnIndex(0);
                if (SmallButton("Play")) {
                    if (!TheAudioManager.loadedStreams.empty()) {
                        for (auto &stream : TheAudioManager.loadedStreams) {
                            TheAudioManager.StopPlayback(stream.handle);
                        }
                        TheAudioManager.loadedStreams.clear();
                    }
                    TheSongList.curSong = song;
                    TheSongList.curSong->LoadSongIni(TheSongList.curSong->songDir);
                    TheMenuManager.CreateAndSwitchMenu<ReadyUpMenu>(TheSongList.curSong);
                }

                PopID();
            }
            EndTable();
        }
    }
    End();
}

void EncoreDebug::StartReloadAssets() {
    for (auto asset : TheAssets.assets) {
        if (dynamic_cast<FontAsset *>(asset)) {
            if (!reloadFonts) {
                continue;
            }
        }
        if (asset->state == LOADING || asset->state == PREFINALIZED) {
            asset->CheckForFetch();
        }
        if (asset->state == LOADED) {
            asset->Unload();
        }
        if (asset->state == UNLOADED) {
            asset->StartLoad();
        }
    }
    reloadQueued = false;
}

void EncoreDebug::DrawAssetViewer() {
    ZoneScoped;
    SetNextWindowSize({ 200, 300 }, ImGuiCond_FirstUseEver);
    if (Begin("Assets", &showAssets, 0)) {
        TextWrapped("Base Path: %s",
                    TheAssets.getDirectory().generic_string().c_str());
        if (Button("Reload All")) {
            reloadQueued = true;
        }
        SameLine();
        Checkbox("Reload Fonts", &reloadFonts);

        const ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg |
            ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV;
        if (BeginTable("AssetList", 4, flags, GetContentRegionAvail())) {
            TableSetupScrollFreeze(0, 1);
            TableSetupColumn("##Actions", ImGuiTableColumnFlags_WidthFixed);
            TableSetupColumn("Filename", ImGuiTableColumnFlags_WidthStretch);
            TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
            TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed);
            TableHeadersRow();

            PushFont(GetIO().FontDefault, 16);
            size_t i = 0;
            for (auto asset : TheAssets.assets) {
                TableNextRow();
                PushID(i);
                TableSetColumnIndex(1);
                ImGui::Text("%s",
                            std::filesystem::path(asset->id).filename().generic_string().
                            c_str());
                TableSetColumnIndex(2);
                ImGui::Text("%s", typeid(*asset).name());
                TableSetColumnIndex(3);
                ImGui::Text("%s", AssetStateName(asset->state));

                TableSetColumnIndex(0);
                switch (asset->state) {
                case UNLOADED:
                    if (SmallButton("Load")) {
                        asset->StartLoad();
                    }
                    break;
                case PREFINALIZED:
                    if (SmallButton("Finalize")) {
                        asset->CheckForFetch();
                    }
                    break;
                case LOADED:
                    if (SmallButton("Unload")) {
                        asset->Unload();
                    }
                    break;
                default:
                    break;
                }
                PopID();
                i++;
            }
            PopFont();
            EndTable();
        }
    }
    End();
}

void EncoreDebug::DrawLog() {
    SetNextWindowSize({ 1000, 500 }, ImGuiCond_Once);
    if (Begin("Log", &showLog)) {
        if (BeginChild("messages", GetContentRegionAvail(), ImGuiChildFlags_Borders)) {
            PushFont(GetIO().FontDefault, 14);
            auto messages = Encore::Log::GetRecentMessages();
            for (auto &message : messages) {
                Text("%s", message.c_str());
            }
            ScrollToItem();
            PopFont();
        }
        EndChild();
    }
    End();
}

float bounceTimer = 1.0f;
float bounceMult = 5.0f;

void Encore::Track::DrawTrackDebugWindow() {
    ZoneScoped;
    SetNextWindowSizeConstraints({ 400, 0.0f }, { FLT_MAX, FLT_MAX });
    if (Begin(
        std::string("Track Settings: " + player.Name).c_str(),
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize
    )) {
        if (!BeginTabBar("Track Debug")) {
            End();
            return;
        }
        if (BeginTabItem("Camera")) {
            DragFloat3("Camera Position", (float *)&BaseCamera.position, 0.1);
            DragFloat3("Camera Target", (float *)&BaseCamera.target, 0.1);
            DragFloat("Camera FOV", &BaseCamera.fovy);
            DragFloat("Base Length", &BaseLength, 0.1);
            DragFloat("Track Fade Size", &FadeSize, 0.1);
            DragFloat("Curve Factor", &CurveFac, 1);
            Checkbox("Column Fitting", &ColumnFitting);
            if (!ColumnFitting) {
                DragFloat("Offset", &Offset, 0.01);
                DragFloat("Scale", &Scale, 0.01);
            } else {
                DragFloat("Column Left", &ColumnLeft, 0.01);
                DragFloat("Column Right", &ColumnRight, 0.01);
            }
            DragFloat("Note Height", &NoteHeight, 0.01);
            EndTabItem();
        }
        if (BeginTabItem("RhythmEngine")) {
            BeginTabBar("guh");
            if (BeginTabItem("Engine")) {
                auto engine = player.engine;
                BeginDisabled();
                float time = engine->LastUpdateTime;
                InputFloat("Last Update Time", &time);
                time = engine->stats->InputOffset;
                InputFloat("Input Offset", &time);
                time = engine->stats->InputTime;
                InputFloat("Input Time", &time);
                EndDisabled();
                float health = player.engine->stats->Health;
                SliderFloat("Health", &health, 0, 1);
                player.engine->stats->Health = health;
                float od = player.engine->stats->overdrive.Fill;
                SliderFloat("Overdrive", &od, 0, 1);
                Checkbox("Overdrive Active", &player.engine->stats->overdrive.Active);
                player.engine->stats->overdrive.Fill = od;
                ProgressBar(engine->whammy, { -1.0f, 0.0f }, "Whammy");
                SeparatorText("Timers");
                for (auto Timer : engine->Timers) {
                    for (auto timer : player.engine->Timers) {
                        float countdown = Clamp(
                            (timer.second.Time + timer.second.Duration)
                            - TheSongTime.GetElapsedTime(),
                            0,
                            timer.second.Duration
                        );
                        ProgressBar(countdown / timer.second.Duration,
                                    { -FLT_MIN, 0 },
                                    TextFormat("%s: %4.4f",
                                               timer.first.c_str(),
                                               countdown));
                    };
                }
                SeparatorText("Multiplier Flash");
                if (Button("Send Combo Break Event")) {
                    player.engine->FireEventTemp(MultFlashEvent(true));
                }
                SameLine();
                if (Button("Send Combo Gain Event")) {
                    player.engine->FireEventTemp(MultFlashEvent(false));
                }
                SeparatorText("Overdrive Gain Flash");
                if (Button("Send Gain Flash Event")) {
                    player.engine->FireEventTemp(OverdriveGain());
                }
                SeparatorText("Highway Bounce");
                SameLine();
                if (Button("Send Event")) {
                    HighwayBounceEvent bounce(bounceTimer, bounceMult);
                    player.engine->FireEvent(&bounce);
                }
                DragFloat("Timer", &bounceTimer);
                DragFloat("Mult", &bounceMult);
                SeparatorText("Overhit");
                if (Button("Send Engine Event")) {
                    if (player.engine->stats->Type == 0) {
                        player.engine->Overhit(0);
                    } else {
                        for (int i = 0; i < player.engine->stats->HeldFrets.size(); i++) {
                            if (player.engine->stats->HeldFrets.at(i))
                                player.engine->Overhit(i);
                        }
                    }
                }
                SameLine();
                if (Button("Send Track Event")) {
                    if (player.engine->stats->Type == 0) {
                        player.engine->FireEventTemp(OverhitEvent(0));
                    } else {
                        for (int i = 0; i < player.engine->stats->HeldFrets.size(); i++) {
                            if (player.engine->stats->HeldFrets.at(i))
                                player.engine->FireEventTemp(OverhitEvent(i));
                        }
                    }
                }
                EndTabItem();
            }
            if (BeginTabItem("Stats")) {
                DragInt("Combo", &player.engine->stats->Combo);

                ImGui::Text("%s",
                            TextFormat("Ghost Count: %i", player.engine->GhostCount));
                ImGui::Text("%s",
                            TextFormat("Max combo: %i", player.engine->stats->MaxCombo));
                ImGui::Text("%s",
                            TextFormat("Attempted notes: %i",
                                       player.engine->stats->AttemptedNotes)
                );
                ImGui::Text("%s", TextFormat("Misses: %i", player.engine->stats->Misses));
                ImGui::Text("%s",
                            TextFormat("Notes hit: %i (%.0f%)",
                                       player.engine->stats->NotesHit,
                                       (float)player.engine->stats->NotesHit / player.
                                       engine->
                                       stats->AttemptedNotes * 100));
                ImGui::Text("%s",
                            TextFormat("Score: %4.2f", player.engine->stats->Score));
                ImGui::Text("%s",
                            TextFormat("Base score: %4.2f",
                                       player.engine->chart->BaseScore));
                ImGui::Text("%s", TextFormat("Stars: *%i", player.engine->stats->Stars));
                ImGui::Text("%s",
                            TextFormat("Multiplier: %.2fx",
                                       player.engine->stats->multiplier()));
                Checkbox("Allow Timestamped Inputs",
                         &player.engine->allowTimestampedInputs);
                EndTabItem();
            }
            if (BeginTabItem("Info")) {
                const unsigned int count = 1000;
                static float values[count] = { 0 };
                auto acc = [this](double time) {
                    if (time < perfectFrontend / goodFrontend) {
                        return 1.0;
                    }
                    return (1 - time) / 1;
                };
                for (unsigned int i = 0; i < count; i++) {
                    double f = (double)i / (double)count;
                    values[i] = acc(f);
                }
                PlotLines(
                    "###acc",
                    values,
                    count,
                    0,
                    "Accuracy",
                    FLT_MAX,
                    FLT_MAX,
                    { 0.0f, GetContentRegionAvail().x * 0.8f }
                );

                const unsigned int hcount = 1000;
                static float hvalues[hcount] = { 0 };
                auto health = [this](float time) {
                    if (time < perfectFrontend / goodFrontend) {
                        return healthGainPerNote;
                    }
                    if (time > 0.7) {
                        return -(healthGainPerNote * (1 - ((1 - time) / 1)));
                    }
                    return healthGainPerNote * ((1 - time) / 1);
                };
                for (unsigned int i = 0; i < hcount; i++) {
                    double f = (double)i / (double)hcount;
                    hvalues[i] = health(f);
                }
                PlotLines(
                    "###health",
                    hvalues,
                    hcount,
                    0,
                    "Health gain/note accuracy",
                    FLT_MAX,
                    FLT_MAX,
                    { 0.0f, GetContentRegionAvail().x * 0.8f }
                );
                EndTabItem();
            }
            EndTabBar();
            EndTabItem();
        }
        if (BeginTabItem("Sections")) {
            if (BeginTable("Sections", 3)) {
                int i = 0;
                for (auto &section : player.engine->chart->sections) {
                    TableNextRow();
                    TableSetColumnIndex(0);
                    if (i == player.engine->chart->CurrentSection) {
                        ImGui::Text("%s", ">");
                    }
                    TableSetColumnIndex(1);
                    ImGui::Text("%s", section.name.c_str());
                    TableSetColumnIndex(2);
                    ImGui::Text("%s",
                                TextFormat("%01i/%01i -%01i",
                                           section.hit,
                                           section.notes,
                                           section.overhits));
                    i++;
                }
                EndTable();
            }
            EndTabItem();
        }
        if (BeginTabItem("Track Slots")) {
            if (Button("Configure 5 Lane")) {
                Configure5Lane();
            }
            if (Button("Configure 5 Lane (Gem Open)")) {
                Configure5LaneGemOpen();
            }
            if (Button("Configure 5 Lane (Kick Open)")) {
                Configure5LaneKickOpen();
            }
            if (Button("Configure 4 Lane")) {
                Configure4Lane();
            }
            if (Button("Configure Drums")) {
                ConfigureDrums();
            }
            if (Button("Configure 7 Lane Drums")) {
                ConfigurePSDrums();
            }
            if (Button("Configure Fuck You Drums")) {
                ConfigureFuckYoyDrums();
            }
            if (Button("Configure Drums (Gem Kick)")) {
                ConfigureDrumsGemKick();
            }
            for (auto &slot : slots) {
                Separator();
                PushID(slot->index);
                if (ColorButton("Hit",
                                ImGuiColor(
                                    player.QueryColorProfile(
                                        slot->colorSlot,
                                        ColorProfileType)))) {
                    slot->AnimateHit(false,
                                     player.QueryColorProfile(
                                         slot->colorSlot,
                                         ColorProfileType));
                }
                DragFloat("X Position", &slot->xPos, 0.01);
                SameLine();
                DragFloat("Width", &slot->width, 0.01);
                SameLine();
                DragFloat("Length", &slot->length, 0.01);
                PopID();
            }
            EndTabItem();
        }
        EndTabBar();
    }
    End();
}