#include "EncoreDebug.h"

#include "assets.h"
#include "imgui.h"
#include "raymath.h"
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

bool showDemoWindow = false;
bool showAssets = false;
bool showPlayerManager = false;
bool showSongList = false;

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
}


void EncoreDebug::MenuBar() {
    BeginMainMenuBar();
    if (BeginMenu("Windows")) {
        MenuItem("Assets", 0, &showAssets);
        MenuItem("Player Manager", 0, &showPlayerManager);
        MenuItem("Song List", 0, &showSongList, TheMenuManager.currentScreen != GAMEPLAY);
        MenuItem("ImGui Demo Window", 0, &showDemoWindow);
        EndMenu();
    }
    if (BeginMenu(TextFormat("Framerate (%i FPS)###Framerate", GetFPS()))) {
        Text("%i FPS", GetFPS());
        MenuItem("Uncap Framerate", 0, &TheFrameManager.removeFPSLimit);
        MenuItem("VSync", 0, &TheGameSettings.VerticalSync);
        SliderInt("Menu FPS", &TheFrameManager.menuFPS, 1, 300);
        SliderInt("Gameplay FPS", &TheGameSettings.Framerate, 1, 1500);
        EndMenu();
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

    auto avail = GetWindowWidth();
    auto size = CalcTextSize(debugVersionHash.c_str()).x;

    SetCursorPosX(avail - size - GetStyle().FramePadding.x);
    Text(debugVersionHash.c_str());

    EndMainMenuBar();
}

void EncoreDebug::DrawPlayerManager() {
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
                    if (!TheSongList.curSong->ini) {
                        TheSongList.curSong->LoadSongJSON(TheSongList.curSong->songInfoPath);
                    } else {
                        TheSongList.curSong->LoadSongIni(TheSongList.curSong->songDir);
                    }
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
    SetNextWindowSize({ 200, 300 }, ImGuiCond_FirstUseEver);
    if (Begin("Assets", &showAssets, 0)) {
        TextWrapped("Base Path: %s",
                           TheAssets.getDirectory().generic_string().c_str());
        if (Button("Reload All")) {
            reloadQueued = true;
        }

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
    SetNextWindowSizeConstraints({ 400, 0.0f }, { FLT_MAX, FLT_MAX });
    if (Begin(
        std::string("Track Settings: " + player.Name).c_str(),
        nullptr,
        ImGuiWindowFlags_AlwaysAutoResize
    )) {
        if (CollapsingHeader("Camera Settings")) {
            DragFloat3("Camera Position", (float *)&camera.position, 0.1);
            DragFloat3("Camera Target", (float *)&camera.target, 0.1);
            DragFloat("Camera FOV", &camera.fovy);
            DragFloat("Base Length", &BaseLength, 0.1);
            DragFloat("Track Fade Size", &FadeSize, 0.1);
            DragFloat("Curve Factor", &CurveFac, 1);
            DragFloat("Offset", &Offset, 0.01);
            DragFloat("Scale", &Scale, 0.01);
            DragFloat("Note Height", &NoteHeight, 0.01);
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