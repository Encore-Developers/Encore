#include "EncoreDebug.h"

#include "assets.h"
#include "imgui.h"
#include "raymath.h"
#include "gameplay/inputCallbacks.h"
#include "users/playerManager.h"
#include "misc/imgui_stdlib.h"
#include "settings/settings.h"
#include "util/frame-manager.h"
#include "gameplay/trackRenderer/Track.h"
#include "menus/GameplayMenu.h"
#include "menus/MenuManager.h"
#include "song/audio.h"
#include "song/song.h"
#include "song/songlist.h"

bool EncoreDebug::showDebug = false;
bool EncoreDebug::reloadQueued = false;
bool EncoreDebug::reloadFonts = false;

bool showDemoWindow = false;
bool showAssets = false;
bool showPlayerManager = false;
bool showSongList = false;
bool showQuickSettings = false;
bool showPractice = false;

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
    if (showSongList && TheMenuManager.currentScreen != GAMEPLAY) {
        DrawSongList();
    }
    if (showQuickSettings) {
        DrawQuickSettings();
    }
    if (showPractice) {
        DrawPracticeSectionSelector();
    }
    DrawSongScrubber();
}

void EncoreDebug::MenuBar() {
    ZoneScoped;
    BeginMainMenuBar();
    if (BeginMenu("Windows")) {
        MenuItem("Assets", 0, &showAssets);
        MenuItem("Player Manager", 0, &showPlayerManager);
        MenuItem("Song List", 0, &showSongList, TheMenuManager.currentScreen != GAMEPLAY);
        MenuItem("ImGui Demo Window", 0, &showDemoWindow);
        EndMenu();
    }
    if (MenuItem(TextFormat("Quick Settings (%i FPS)###QuickSettings", GetFPS()), 0, &showQuickSettings)) {

    }

    if (TheMenuManager.currentScreen == GAMEPLAY && MenuItem("End Song")) {
        TheSongTime.Reset();
        TheAudioManager.unloadStreams();
        songPlaying = false;
        TheSongTime.Beatlines.erase(
            TheSongTime.Beatlines.begin(),
            TheSongTime.Beatlines.end()
        );
        TheSongTime.OverdriveTicks.erase(
            TheSongTime.OverdriveTicks.begin(),
            TheSongTime.OverdriveTicks.end()
        );
        TheSongTime.TimeSigChanges.erase(
            TheSongTime.TimeSigChanges.begin(),
            TheSongTime.TimeSigChanges.end()
        );
        TheSongTime.BPMChanges.erase(
            TheSongTime.BPMChanges.begin(),
            TheSongTime.BPMChanges.end()
        );
        TheSongTime.LastTick = 0;
        TheSongTime.CurrentTick = 0;
        TheSongTime.LastODTick = 0;
        TheSongTime.CurrentODTick = 0;
        TheSongTime.CurrentBPM = 0;
        TheSongTime.CurrentODTickItr = 0;
        TheSongTime.CurrentTimeSig = 0;
        TheSongTime.CurrentBeatline = 0;
        TheMenuManager.SwitchScreen(RESULTS);
    }
    if (TheMenuManager.currentScreen == GAMEPLAY) {
        MenuItem("Practice", 0, &showPractice);
    }

    if (TheMenuManager.currentScreen == GAMEPLAY && MenuItem(pauseText.c_str())) {
        paused = !paused;
        for (auto index : ThePlayerManager.ActivePlayers) {
            if (index == -1)
                continue;
            auto player = ThePlayerManager.PlayerList[index];
            player.engine->stats->Paused = paused;
        }
        if (paused) {
            pauseText = "Play";
            TheAudioManager.pauseStreams();
        }
        else{
            pauseText = "Pause";
            TheAudioManager.unpauseStreams();
        }
    }
    auto avail = GetWindowWidth();
    auto size = CalcTextSize(debugVersionHash.c_str()).x;

    SetCursorPosX(avail - size - GetStyle().FramePadding.x);
    Text(debugVersionHash.c_str());

    EndMainMenuBar();
}


void EncoreDebug::DrawQuickSettings() {
    ZoneScoped;
    if (Begin("Quick Settings")) {
        SliderFloat("Song Speed", &TheAudioManager.songSpeed, 0, 3);
        Checkbox("Uncap Framerate", &TheFrameManager.removeFPSLimit);
        Checkbox("VSync", &TheGameSettings.VerticalSync);
        SliderInt("Menu FPS", &TheFrameManager.menuFPS, 1, 300);
        SliderInt("Gameplay FPS", &TheGameSettings.Framerate, 1, 1500);
        SliderInt("Controller Poll Rate", &ControllerPoller::controllerPollRate, 10, 1000, "%dhz");
        if (DragInt("Audio Calibration", &TheGameSettings.AudioOffset, 1, 0, 0, "%dms")) {
            TheSongTime.SetOffset(TheGameSettings.AudioOffset / 1000.0);
        }
        DragInt("Video Calibration", &TheGameSettings.VideoOffset, 1, 0, 0, "%dms");

        Text("Audio Settings");
        SliderFloat("Main Volume", &TheGameSettings.avMainVolume, 0.0, 1.0);
        if (CollapsingHeader("Advanced")) {
            SliderFloat("Active Inst Volume", &TheGameSettings.avActiveInstrumentVolume, 0.0, 1.0);
            SliderFloat("Inactive Inst Volume", &TheGameSettings.avInactiveInstrumentVolume, 0.0, 1.0);
            SliderFloat("Track Mute Volume", &TheGameSettings.avMuteVolume, 0.0, 1.0);
            SliderFloat("SFX Volume", &TheGameSettings.avSoundEffectVolume, 0.0, 1.0);
            SliderFloat("Menu Music Volume", &TheGameSettings.avMenuMusicVolume, 0.0, 1.0);
        }
        if (Button("Save Settings")) {
            TheGameSettings.SaveToFile((TheGameSettings.directory / "settings.json").string());
        }
    }
    End();
}

void DebugSeek(float time, float audioTime) {
    TheAudioManager.seekStreams(audioTime);
    for (auto index : ThePlayerManager.ActivePlayers) {
        if (index == -1) {
            continue;
        }
        auto player = ThePlayerManager.PlayerList[index];
        auto engine = player.engine.get();
        for (int i = 0; i < engine->chart->CurrentNoteIterators.size(); i++) {
            if (i >= engine->chart->Lanes.size()) {
                break;
            }
            for (auto iter = engine->chart->Lanes[i].begin(); iter < engine->chart->Lanes[i].end(); ++iter) {
                if (iter->StartSeconds > time) {
                    engine->chart->CurrentNoteIterators[i] = iter;
                    break;
                }
            }

        }
    }
}


void EncoreDebug::DrawPracticeSectionSelector() {
    ZoneScoped;
    if (TheMenuManager.currentScreen == GAMEPLAY) {
        if (Begin("Practice Section Selector")) {
            for (int sectionInt = 0; sectionInt < TheSongTime.Sections.size(); sectionInt++) {
                Text(TheSongTime.Sections.at(sectionInt).name.c_str());
                SameLine();
                float buttWidth = CalcTextSize(" whole").x;
                SetCursorPosX(GetWindowWidth() - (buttWidth * 3));
                PushID(sectionInt);
                if (Button("whole")) {
                    double startTime;
                    for (auto &playerInt : ThePlayerManager.ActivePlayers) {
                        if (playerInt == -1) {
                            continue;
                        }
                        auto &player = ThePlayerManager.PlayerList.at(playerInt);
                        double endTime = 0.0;
                        startTime = TheSongTime.Sections.at(sectionInt).start;
                        if (sectionInt == TheSongTime.Sections.size() - 1)
                            endTime = TheSongTime.GetSongLength();
                        else
                            endTime = TheSongTime.Sections.at(sectionInt + 1).start;
                        player.engine->pStartTime = startTime - 0.1;
                        player.engine->pStopTime = endTime;
                        player.engine->practice = true;
                    }
                    DebugSeek(startTime, startTime - 2);
                }
                SameLine();
                if (Button("start")) {
                    double startTime;
                    for (auto &playerInt : ThePlayerManager.ActivePlayers) {
                        if (playerInt == -1) {
                            continue;
                        }
                        auto &player = ThePlayerManager.PlayerList.at(playerInt);
                        startTime = TheSongTime.Sections.at(sectionInt).start;
                        player.engine->pStartTime = startTime - 0.1;
                        player.engine->pStopTime = TheSongTime.GetSongLength();
                        player.engine->practice = true;
                    }
                    DebugSeek(startTime, startTime - 2);
                }
                SameLine();
                if (Button("end")) {
                    for (auto &playerInt : ThePlayerManager.ActivePlayers) {
                        if (playerInt == -1) {
                            continue;
                        }
                        auto &player = ThePlayerManager.PlayerList.at(playerInt);
                        double endTime = 0.0;
                        if (sectionInt == TheSongTime.Sections.size() - 1)
                            endTime = TheSongTime.GetSongLength();
                        else
                            endTime = TheSongTime.Sections.at(sectionInt + 1).start;
                        player.engine->pStopTime = endTime;
                        player.engine->practice = true;
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
    if (TheMenuManager.currentScreen == GAMEPLAY) {
        SetNextWindowPos({0, GetFrameHeight()+4}, ImGuiCond_Always);
        SetNextWindowSize({ImGui::GetIO().DisplaySize.x, 0}, ImGuiCond_Always);
        if (Begin("Song Scrubber", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
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
            if (IsItemActive()) {
                DebugSeek(GetTimeAtPos(GetMouseLocalPos()), GetTimeAtPos(GetMouseLocalPos()));
            }
            auto drawlist = GetWindowDrawList();
            drawlist->AddRectFilled(pos, pos + size, ColorConvertFloat4ToU32(GetStyle().Colors[IsItemHovered() && !IsItemActive() ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg]));
            drawlist->AddLine(pos + ImVec2(TimeToPos(time), 0), pos + ImVec2(TimeToPos(time), size.y), ColorConvertFloat4ToU32({1, 0, 1, 1}), 2);

            static std::vector<TimelineTextSpacer> texts = {};
            texts.clear();
            auto occupied = [&](float x, int layer) {
                if (layer < 0) {
                    return true;
                }
                for (auto& spacer : texts) {
                    if (x > spacer.startPos && x < spacer.endPos && spacer.layer == layer) {
                        return true;
                    }
                }
                return false;
            };
            int layer = 0;
            int maxlayer = 0;
            for (int i = 0; i < TheSongTime.Sections.size(); i++) {
                auto& section = TheSongTime.Sections[i];
                float endTime = TheSongTime.GetSongLength();
                if (i < TheSongTime.Sections.size() - 1) {
                    endTime = TheSongTime.Sections[i+1].start;
                }
                float rectPos = TimeToPos(section.start);
                float startPos = TimeToPos(section.start);
                float textWide = MeasureText(section.name.c_str(), GetFontSize());
                if (startPos+textWide > size.x) {
                    startPos = size.x-textWide;
                }
                drawlist->AddRectFilled(pos + ImVec2(rectPos, 0), pos + ImVec2(TimeToPos(endTime), size.y), ColorConvertFloat4ToU32(i % 2 == 0 ? ImVec4 {1, 1, 1, 0.2} : ImVec4 {1, 1, 1, 0.1}));
                drawlist->AddLine(pos + ImVec2(rectPos, 0), pos + ImVec2(rectPos, size.y), ColorConvertFloat4ToU32({1, 1, 1, 0.9}));
                layer = 0;
                while (occupied(startPos, layer)) {
                    layer++;
                }
                float textY = pos.y + size.y + layer * GetFontSize();
                drawlist->AddText(ImVec2(startPos+pos.x, textY), 0xffffffff, section.name.c_str());
                texts.push_back({startPos, startPos+textWide, layer});
                if (layer > maxlayer) {
                    maxlayer = layer;
                }
            }

            Dummy(ImVec2{0, GetFontSize()*(maxlayer+1)});
            PopStyleVar();
        }
        End();
    }

}

void EncoreDebug::DrawPlayerManager() {
    ZoneScoped;
    if (Begin("Player Manager", &showPlayerManager, 0)) {
        if (Button("Save All")) {
            ThePlayerManager.SavePlayerList();
        }

        if (BeginTabBar("Players")) {
            for (auto &player : ThePlayerManager.PlayerList) {
                if (BeginTabItem(
                    (player.Name + TextFormat("###%x", &player)).c_str())) {
                    InputText("Username", &player.Name);
                    SeparatorText("Color Profile");
                    ColorEdit("Green",
                              &player.GetColorProfile()->colors[Encore::SLOT_GREEN],
                              0);
                    ColorEdit("Red",
                              &player.GetColorProfile()->colors[Encore::SLOT_RED],
                              0);
                    ColorEdit("Yellow",
                              &player.GetColorProfile()->colors[Encore::SLOT_YELLOW],
                              0);
                    ColorEdit("Blue",
                              &player.GetColorProfile()->colors[Encore::SLOT_BLUE],
                              0);
                    ColorEdit("Orange",
                              &player.GetColorProfile()->colors[Encore::SLOT_ORANGE],
                              0);
                    ColorEdit("Open",
                              &player.GetColorProfile()->colors[Encore::SLOT_OPEN],
                              0);
                    SeparatorText("Drums Colors");
                    ColorEdit("Kick",
                              &player.GetColorProfile()->colors[Encore::SLOT_KICK],
                              0);
                    ColorEdit("Yellow Cymbal",
                              &player.GetColorProfile()->colors[Encore::SLOT_HIHAT],
                              0);
                    ColorEdit("Blue Cymbal",
                              &player.GetColorProfile()->colors[Encore::SLOT_RIDE],
                              0);
                    ColorEdit("Green Cymbal",
                              &player.GetColorProfile()->colors[Encore::SLOT_CRASH],
                              0);

                    SeparatorText(std::string("Player: " + player.Name).c_str());
                    SliderFloat("Note Speed", &player.NoteSpeed, 0, 3);
                    SliderFloat("Track Length", &player.HighwayLength, 0, 5);
                    int inputOffset = player.InputCalibration * 1000;
                    DragInt("Input Calibration",
                                   &inputOffset,
                                   1,
                                   -1000,
                                   1000,
                                   "%dms");
                    player.InputCalibration = inputOffset / 1000.0;
                    ColorEdit("Accent Color", &player.AccentColor, 0);
                    Checkbox("Bot", &player.Bot);
                    Checkbox("Lefty Flip", &player.LeftyFlip);
                    Checkbox("Brutal Mode", &player.BrutalMode);
                    EndTabItem();

                    if (Button("Delete Player")) {
                        ThePlayerManager.DeletePlayer(player);
                        ThePlayerManager.SavePlayerList();
                    }
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
        if (c == '\'') {
            continue; // Quirk: ignore apostrophe, makes searching better
        }
        out += std::tolower(c);
    }
    return out;
}

void EncoreDebug::DrawSongList() {
    ZoneScoped;
    static std::string filter;
    static std::vector<Song *> songs;
    static bool firstTime = true;

    static auto UpdateList = [&] {
        songs.clear();
        std::string lowerFilter = tolowerStr(filter);
        for (int i = 0; i < TheSongList.songs.size(); i++) {
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

        if (BeginTable("Song List", 4, flags, GetContentRegionAvail())) {
            TableSetupScrollFreeze(0, 1);
            TableSetupColumn("##Actions", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoSort);
            TableSetupColumn("Title", ImGuiTableColumnFlags_WidthStretch);
            TableSetupColumn("Artist", ImGuiTableColumnFlags_WidthFixed);
            TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed);
            TableHeadersRow();

            for (int i = 0; i < songs.size(); i++) {
                auto song = songs[i];
                TableNextRow();
                PushID(i);

                TableSetColumnIndex(1);
                Text("%s", song->title.c_str());

                TableSetColumnIndex(2);
                Text("%s", song->artist.c_str());

                TableSetColumnIndex(3);
                Text("%s", song->source.c_str());

                TableSetColumnIndex(0);
                if (SmallButton("Play")) {
                    if (!TheAudioManager.loadedStreams.empty()) {
                        for (auto& stream : TheAudioManager.loadedStreams) {
                            TheAudioManager.StopPlayback(stream.handle);
                        }
                        TheAudioManager.loadedStreams.clear();
                    }
                    TheSongList.curSong = song;
                    TheSongList.curSong->LoadSongIni(TheSongList.curSong->songDir);
                    TheMenuManager.SwitchScreen(READY_UP);
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
        if (dynamic_cast<FontAsset*>(asset)) {
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
            int i = 0;
            for (auto asset : TheAssets.assets) {
                TableNextRow();
                PushID(i);
                TableSetColumnIndex(1);
                Text(
                    std::filesystem::path(asset->id).filename().generic_string().c_str());
                TableSetColumnIndex(2);
                Text(typeid(*asset).name());
                TableSetColumnIndex(3);
                Text(AssetStateName(asset->state));

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

void Encore::Track::DrawTrackDebugWindow() {
    ZoneScoped;
    SetNextWindowSizeConstraints({ 400, 0.0f }, { FLT_MAX, FLT_MAX });
    if (Begin(
        std::string("Track Settings: " + player.Name).c_str(),
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize
    )) {
        if (CollapsingHeader("Camera Settings")) {
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

            if (Button("Configure 5 Lane")) {
                Configure5Lane();
            }
            if (Button("Configure 5 Lane (Gem Open)")) {
                Configure5LaneGemOpen();
            }
            if (Button("Configure 4 Lane")) {
                Configure4Lane();
            }
            if (Button("Configure Drums")) {
                ConfigureDrums();
            }
            if (Button("Configure Drums (Gem Kick)")) {
                ConfigureDrumsGemKick();
            }
        }
        if (CollapsingHeader("Engine State")) {
            SeparatorText("Timers");
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
            SeparatorText("Stats");
            Text(TextFormat("Combo: %i", player.engine->stats->Combo));
            Text(TextFormat("Ghost Count: %i", player.engine->GhostCount));
            Text(TextFormat("Max combo: %i", player.engine->stats->MaxCombo));
            Text(
                TextFormat("Attempted notes: %i", player.engine->stats->AttemptedNotes)
            );
            Text(TextFormat("Misses: %i", player.engine->stats->Misses));
            Text(TextFormat("Notes hit: %i (%.0f%)",
                                   player.engine->stats->NotesHit,
                                   (float)player.engine->stats->NotesHit / player.engine->
                                   stats->AttemptedNotes * 100));
            Text(TextFormat("Score: %4.2f", player.engine->stats->Score));
            Text(TextFormat("Base score: %4.2f", player.engine->chart->BaseScore));
            Text(TextFormat("Stars: *%i", player.engine->stats->Stars));
            Text(TextFormat("Multiplier: %ix",
                                   player.engine->stats->multiplier()));
            Checkbox("Allow Timestamped Inputs", &player.engine->allowTimestampedInputs);
        }
        if (CollapsingHeader("Chart Information")) {
            if (BeginTable("Note List", player.engine->chart->Lanes.size())) {
                TableSetupScrollFreeze(0, 1);
                for (int lane = 0; lane < player.engine->chart->Lanes.size(); lane++) {
                    TableSetupColumn(TextFormat("##%i", lane),
                                            ImGuiTableColumnFlags_WidthStretch);
                }
                TableHeadersRow();
                for (int lane = 0; lane < player.engine->chart->Lanes.size(); lane++) {
                    TableSetColumnIndex(lane);
                    for (auto note : player.engine->chart->Lanes[lane]) {
                        TableNextRow();
                        Text(TextFormat("##%i", note.Lane));
                    }
                }

                EndTable();
            }
        }
    }
    End();
}