//
// Created by marie on 02/05/2024.
//

#include <functional>
#include <random>
#include <raylib.h>
#include <raygui.h>
#include <raymath.h>
#include "TheGame.h"
#include "../../assets.h"
#include "../../song/audio.h"
#include "../../old/lerp.h"
#include "MainMenu.h"

#include "SongSelectMenu.h"

#include <variant>

#include "imgui.h"
#include "../menu.h"
#include "../overshell/overshellRenderer.h"
#include "../util/uiUnits.h"
#include "../util/locale/Locale.h"
#include "../../song/songlist.h"
#include "menus/util/ButtonActionRegistry.h"
#include "menus/util/Jukebox.h"
#include "SDL3/SDL_misc.h"
#include "menus/settings/SettingsMenu.h"
#include "song/ArtLoader.h"
#include "song/OpenSource.h"

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH
#endif
#include "../MenuManager.h"
#include "../../settings/settings.h"
#include "users/playerManager.h"

#ifndef ENCORE_VERSION
#define ENCORE_VERSION
#endif

const float Width = (float)GetRenderWidth();
const float Height = (float)GetRenderHeight();

bool randomSongChosen = false;
std::string menuCommitHash = GIT_COMMIT_HASH;
std::string menuVersion = ENCORE_VERSION;
std::string gitBranch = GIT_BRANCH;
Assets &menuAss = Assets::getInstance();
SongList &songListMenu = TheSongList;
Units u = Units::getInstance();
Encore::Jukebox TheGameJukebox;
MainMenu TheGameMenu;
int MainMenu::logoInt = 0;

void GameMenu::DrawTopOvershell(float TopOvershell) {
    DrawRectangleGradientV(
        0,
        u.hpct(TopOvershell) - 2,
        GetRenderWidth(),
        u.hinpct(0.025f),
        Color { 0, 0, 0, 128 },
        Color { 0, 0, 0, 0 }
    );
    DrawRectangle(0, 0, (int)GetRenderWidth(), u.hpct(TopOvershell), WHITE);
    DrawRectangle(
        0,
        0,
        (int)GetRenderWidth(),
        u.hpct(TopOvershell) - u.hinpct(0.005f),
        ColorBrightness(GetColor(0x181827FF), -0.25f)
    );
}

void GameMenu::DrawBottomOvershell() {
    float BottomOvershell = GetRenderHeight() - u.hpct(0.15f);
    DrawRectangle(
        0, BottomOvershell, (float)(GetRenderWidth()), (float)GetRenderHeight(), WHITE
    );
    DrawRectangle(
        0,
        BottomOvershell + u.hinpct(0.005f),
        (float)(GetRenderWidth()),
        (float)GetRenderHeight(),
        ColorBrightness(GetColor(0x181827FF), -0.5f)
    );
}

// should be reduced to just PlayerSongStats (instead of Player) eventually

// todo: replace player with band stats

void GameMenu::DrawAlbumArtBackground() {
    if (!TheArtLoader.loadedArtBlur) {
        return;
    }
    float diagonalLength = sqrtf(
        (float)(GetRenderWidth() * GetRenderWidth())
        + (float)(GetRenderHeight() * GetRenderHeight())
    );
    float RectXPos = GetRenderWidth() / 2;
    float RectYPos = diagonalLength / 2;

    BeginShaderMode(menuAss.bgShader);
    DrawTexturePro(
        *TheArtLoader.loadedArtBlur,
        Rectangle { 0, 0, (float)TheArtLoader.loadedArtBlur->GetTexture().width, (float)TheArtLoader.loadedArtBlur->GetTexture().width },
        Rectangle { RectXPos, -RectYPos * 2, diagonalLength * 2, diagonalLength * 2 },
        { 0, 0 },
        45,
        WHITE
    );
    EndShaderMode();
};

void GameMenu::DrawVersion() {
    DrawTextEx(
        menuAss.josefinSansItalic,
        TextFormat(
            "%s-%s:%s", menuVersion.c_str(), gitBranch.c_str(), menuCommitHash.c_str()
        ),
        { u.wpct(0.0025f), u.hpct(0.0025f) },
        u.hinpct(0.025f),
        0,
        WHITE
    );
};


void MainMenu::ChooseSplashText(std::filesystem::path directory) {
    auto splashes = Encore::Locale::GetLocaleList("mainMenu.splashes");
    // Gonna do some incredibly dumb bullshit for the Steam exclusive splashes. Sorry!
#ifdef STEAM
    Encore::LocaleList ownedSplashes = *splashes;
    splashes = &ownedSplashes;
    auto steamSplashes = Encore::Locale::GetLocaleList("mainMenu.steamOnlySplashes");
    for (auto& splash : *steamSplashes) {
        ownedSplashes.push_back(splash);
    }
#endif
    if (splashes->empty()) {
        return;
    }

    std::random_device seed;
    std::mt19937 prng(seed());
    std::string line, result;
    std::uniform_int_distribution<> dist(0, splashes->size()-1);
    int selection = dist(prng);
    SplashString = splashes->at(selection);
    if (selection == Encore::Locale::GetLocaleList("mainMenu.splashes")->size()) {
        SplashString += TheGameRPC.GetSteamNickname() + "!";
    }
    std::cout << result << std::endl;
}

int ChooseRandomLogo() {
    std::random_device seed;
    std::minstd_rand prng(seed());
    std::uniform_int_distribution<> dist(0, 3);
    return dist(prng);
}

void MainMenu::Load() {
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());
    ChooseSplashText(directory);
    if (GameMenu::FirstMainMenuBoot) {
        TheGameJukebox.FirstLoad();
        GameMenu::FirstMainMenuBoot = false;
    } else {
        TheGameJukebox.StartPlayback();
    };
    if (!mainMenuSet.PollLoaded()) {
        mainMenuSet.StartLoad();
    }
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg, LANE_1, "generic.confirm", {
        if (_action != Encore::Action::PRESS) return;
        switch (ControllerSelected) {
        case 0:
            GotoSongSelect();
            break;
        case 1:
            TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
            break;
        case 2:
            for (int p = 0; p < ThePlayerManager.ActivePlayers.size(); p++) {
                if (OvershellState[p] == OS_ATTRACT && ThePlayerManager.ActivePlayers.at(p) != -1)
                    ThePlayerManager.RemoveActivePlayer(p);
            }
            break;
        }
    })
    logoInt = ChooseRandomLogo();
    NEWBUTTONACTION2(buttReg, STRUM_UP, "generic.confirm", {
        if (_action != Encore::Action::PRESS) return;
        ControllerSelected -= 1;
        if (ControllerSelected < 0) ControllerSelected = 0;
    }, false)
    NEWBUTTONACTION2(buttReg, STRUM_DOWN, "generic.confirm", {
        if (_action != Encore::Action::PRESS) return;
        ControllerSelected += 1;
        if (ControllerSelected > 2) ControllerSelected = 2;
    }, false)
    //NEWBUTTONACTION2(buttReg, LANE_2, "Drop Out", {
    //    if (_action != Encore::RhythmEngine::Action::PRESS) return;
    //    slot -= 1;
    //    if (slot == -1) return;
    //    ThePlayerManager.RemoveActivePlayer(slot);
    //})
}
void MainMenu::KeyboardInputCallback(SDL_KeyboardEvent* event) {
    if (ThePlayerManager.PlayersActive == 0) {
        if (event->key == SDLK_RETURN) {
            OvershellState[0] = OS_PLAYER_SELECTION;
        }
        if (event->key == SDLK_ESCAPE) {
            game.shouldQuit = true;
            Encore::Log::Info("Quitting...");
        }
    }
}
void MainMenu::ControllerInputCallback(Encore::ControllerEvent event) {
    int curSlot = 0;
    if (ThePlayerManager.GetPlayerForJoystick(event.slot)) {
        curSlot = ThePlayerManager.GetPlayerForJoystick(event.slot)->ActiveSlot;
    }
    buttReg.HandleInput(event);
}

std::string version = ENCORE_VERSION;
std::string branch = GIT_BRANCH;
std::string build = GIT_COMMIT_HASH;

void DrawWarning(Vector2 pos, Vector2 size) {
    // this was nasty but i wanted to do this anyways
    std::string WindowName = "encore " + version + "-" + branch + ":" + build;
    ImGui::SetNextWindowPos({pos.x, pos.y}, ImGuiCond_Always);
    ImGui::SetNextWindowSize({size.x, size.y});
    if (ImGui::Begin(WindowName.c_str(), 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {
        ImGui::PushTextWrapPos();
        ImGui::TextColored({1, 0, 0, 1}, "%s", LOCALIZE("mainMenu.warning.warning").toChar());
        ImGui::Text("%s", LOCALIZE("mainMenu.warning.info1").toChar());
        //ImGui::Text("How the game is presented to you RIGHT NOW is not representative of the game's final state.");
        ImGui::Text("%s", LOCALIZE("mainMenu.warning.info2").toChar());
        ImGui::NewLine();
        ImGui::TextColored({1, 1, 0, 1}, "%s", LOCALIZE("mainMenu.warning.debug1").toChar());
        ImGui::Text("%s", LOCALIZE("mainMenu.warning.debug2").toChar());
        ImGui::NewLine();
        ImGui::Text("%s", LOCALIZE("mainMenu.warning.bulletList.header").toChar());
        static auto bulletText = [](const char* text) {
            ImGui::Bullet();
            ImGui::Text("%s", text);
        };
        bulletText(LOCALIZE("mainMenu.warning.bulletList.dsMode").toChar());
        bulletText(LOCALIZE("mainMenu.warning.bulletList.instType").toChar());
        bulletText(LOCALIZE("mainMenu.warning.bulletList.songPaths").toChar());
        bulletText(LOCALIZE("mainMenu.warning.bulletList.FUCKCHARTFILES").toChar());
        bulletText(LOCALIZE("mainMenu.warning.bulletList.support").toChar());
        ImGui::Indent();
        if (ImGui::TextLink(LOCALIZE("mainMenu.warning.bulletList.github").toChar())) {
            SDL_OpenURL("https://github.com/Encore-Developers/Encore");
        }
        if (ImGui::TextLink(LOCALIZE("mainMenu.warning.bulletList.discord").toChar())) {
            SDL_OpenURL("https://discord.gg/GhkgVUAC9v");
        }
        ImGui::Unindent();
        ImGui::NewLine();
        ImGui::Text("%s", LOCALIZE("mainMenu.warning.addendum").toChar());
        ImGui::PopTextWrapPos();
    }
    ImGui::End();
}

void MainMenu::DrawMiniMTVOverlay(unsigned char alpha, Vector2 pos)
{
    const auto sourceTex = TheSourceIcons[TheSongList.curSong->source]->GetTexture();
    float TitleFontSize = u.hinpct(0.0425f * 0.75f);
    float TitleFontOffset = (TitleFontSize * 1.25f);

    Encore::Text::DrawText(ASSET(josefinSansBold), TheSongList.curSong->title,
                           {pos.x, pos.y}, TitleFontSize,
                           {255, 255, 255, alpha}, RIGHT);
    Encore::Text::DrawText(ASSET(josefinSansBoldItalic), TheSongList.curSong->artist + ", " + TheSongList.curSong->releaseYear,
                           {pos.x - TitleFontOffset, pos.y + TitleFontOffset}, TitleFontSize * 0.85f,
                           {200, 200, 200, alpha}, RIGHT);
    DrawTexturePro(sourceTex, {0,0, (float)sourceTex.width, (float)sourceTex.height},
                   {pos.x - TitleFontSize, pos.y + TitleFontSize, TitleFontSize, TitleFontSize}, {0,0}, 0,
                   {255, 255, 255, alpha}

    );
}

void MainMenu::AttractScreen() {
    float SplashFontSize = u.hinpct(0.03f);
    float SplashHeight =
        MeasureTextEx(menuAss.josefinSansItalic, SplashString.c_str(), SplashFontSize, 0)
            .y;
    float SplashWidth =
        MeasureTextEx(menuAss.josefinSansItalic, SplashString.c_str(), SplashFontSize, 0)
            .x;
    Color accentColor =
        ColorBrightness(ColorContrast(Color { 255, 0, 255, 128 }, -0.125f), -0.25f);

    float SongFontSize = u.hinpct(0.03f);

    Vector2 StringBox = { u.wpct(0.01f), u.hpct(0.2125f) };
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color { 0, 0, 0, 128 });

    float LogoHeight = u.winpct(0.1f);
    float LogoWidth = u.winpct(0.48f);
    Vector2 GreetSplashPos = { u.wpct(0.5f) - (SplashWidth / 2),
                               u.hpct(0.36f) + LogoHeight };
    Rectangle LogoRect = {
        u.wpct(0.5f) - (LogoWidth / 2), u.hpct(0.35f), LogoWidth, LogoHeight
    };

    DrawTextEx(
        ASSET(josefinSansItalic),
        SplashString.c_str(),
        GreetSplashPos,
        SplashFontSize,
        0,
        WHITE
    );
    std::string hint = LOCALIZE("mainMenu.hint");
    float HintWidth = MeasureTextEx(menuAss.rubik, hint.c_str(), SplashFontSize, 0).x;
    Vector2 HintPos = { u.wpct(0.5f) - (HintWidth / 2), u.hpct(0.5f) + LogoHeight };
    DrawTextEx(menuAss.rubik, hint.c_str(), HintPos, SplashFontSize, 0, WHITE);
    switch (logoInt) {
        case (1): {
            ASSET(encorePrideLogo).Draw(LogoRect, WHITE);
            break;
        }
        case (2): {
            ASSET(encoreTransLogo).Draw(LogoRect, WHITE);
            break;
        }
        case (0):
        default: {
            ASSET(encoreWhiteLogo).Draw(LogoRect, WHITE);
            break;
        }
    }
    //DrawTexturePro(
    //    menuAss.encoreWhiteLogo,
    //    { 0,
    //      0,
     //     (float)menuAss.encoreWhiteLogo.width,
    //      (float)menuAss.encoreWhiteLogo.height },
    //    LogoRect,
    //    { 0, 0 },
    //    0,
    //    WHITE
    //);
    if (TheGameJukebox.streamsLoaded) {

        auto easeInOut = getEasingFunction(EaseInOutSine);
        if (TheAudioManager.GetMusicTimePlayed() > 5) {
            TitleAnimTimer -= GetFrameTime() * 2;
            if (TitleAnimTimer < 0) TitleAnimTimer = 0;
        } else {
            TitleAnimTimer += GetFrameTime() * 2;
            if (TitleAnimTimer > 1) TitleAnimTimer = 1;
        }
        unsigned char Alpha = 96 + (easeInOut(TitleAnimTimer) * 159);
        float TitleFontSize = u.hinpct(0.0425f * 0.75f);
        float TitleFontOffset = (TitleFontSize * 1.25f);
        float topOfVocalBar = GetRenderHeight() - u.hpct(0.1f) - (TitleFontOffset * 2);
        DrawMiniMTVOverlay(Alpha, {u.wpct(0.99f),topOfVocalBar});
    }
    DrawOvershell();

    DrawWarning({float(GetRenderWidth()) - 500, 0}, {500, 650});
}
void MainMenu::GotoSongSelect() {
    TheGameJukebox.UnloadStreams();
    // GameMenu::streamsLoaded = false;
    // streamsPaused = false;
    for (Song &songi : TheSongList.songs) {
        songi.titleScrollTime = GetTime();
        songi.titleTextWidth = menuAss.MeasureTextRubik(songi.title.c_str(), 24);
        songi.artistScrollTime = GetTime();
        songi.artistTextWidth = menuAss.MeasureTextRubik(songi.artist.c_str(), 20);
    }
    TheMenuManager.CreateAndSwitchMenu<SongSelectMenu>();
}
void MainMenu::MainMenuScreen() {
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());
    float SplashFontSize = u.hinpct(0.03f);
    float SplashHeight =
        MeasureTextEx(menuAss.josefinSansItalic, SplashString.c_str(), SplashFontSize, 0)
            .y;
    float SplashWidth =
        MeasureTextEx(menuAss.josefinSansItalic, SplashString.c_str(), SplashFontSize, 0)
            .x;
    Color accentColor =
        ColorBrightness(ColorContrast(Color { 255, 0, 255, 128 }, -0.125f), -0.25f);

    float SongFontSize = u.hinpct(0.03f);

    Vector2 StringBox = { u.wpct(0.01f), u.hpct(0.2125f) };
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color { 0, 0, 0, 128 });

    GameMenu::DrawTopOvershell(0.2f);

    GameMenu::DrawVersion();
    float logoHeight = u.hinpct(0.15f);
    DrawRectangle(0, u.hpct(0.2f), u.LeftSide, u.hinpct(0.05f), accentColor);
    DrawRectangleGradientH(
        u.LeftSide,
        u.hpct(0.2f),
        SplashWidth + u.winpct(0.1f),
        u.hinpct(0.05f),
        accentColor,
        Color { 0, 0, 0, 0 }
    );

    DrawTextEx(
        menuAss.josefinSansItalic, SplashString.c_str(), StringBox, SplashFontSize, 0, WHITE
    );

    Rectangle LogoRect = { u.LeftSide + u.winpct(0.01f),
                           u.hpct(0.035f),
                           Remap(
                               menuAss.encoreWhiteLogo.height,
                               0,
                               menuAss.encoreWhiteLogo.width / 4.25,
                               0,
                               u.winpct(0.5f)
                           ),
                           logoHeight };
    switch (logoInt) {
    case (1): {
        ASSET(encorePrideLogo).Draw(LogoRect, WHITE);
        break;
    }
    case (2): {
        ASSET(encoreTransLogo).Draw(LogoRect, WHITE);
        break;
    }
    case (0):
    default: {
        ASSET(encoreWhiteLogo).Draw(LogoRect, WHITE);
        break;
    }
    }
    auto DrawButtonGradient =  [](Rectangle _pos, Color _color) {
        DrawRectangle(0, _pos.y, _pos.x, _pos.height, _color);
        DrawRectangleGradientH(
            _pos.x,
            _pos.y,
            _pos.width,
            _pos.height,
            _color,
            Color { 0, 0, 0, 0 }
            );
    };
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, 0x00000000);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, 0x00000000);
    GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, 0x00000000);
    GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, 0x00000000);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, 0x00000000);
    GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, 0x00000000);
    GuiSetStyle(DEFAULT, BACKGROUND_COLOR, 0x00000000);
    if (GuiButton({ 0, u.hpct(0.8f), u.LeftSide + SplashWidth, u.hpct(0.05f) }, "") && !isOSOpen()) {
        ChooseSplashText(directory);
    }
    float LeftMMButton = u.wpct(0.03f);
    Color color = {170, 170, 170, 255};
    float MainMenuButtonTopLeft = u.hpct(0.3f);
    Rectangle pos = {LeftMMButton, MainMenuButtonTopLeft, u.winpct(0.35), u.hinpct(0.08f)};
    // Look. I'll admit this isnt the best thing I couldve done.
    // But I dont think any more unnecessary work should be needed for this.
    // It's three fucking buttons for crying out loud. What more should i do?
    // I mean i *guess* this could be shared with the Settings menus but. meh
    // i already have the settings menu to wrangle with
    if (!TheSongList.songs.empty()) {
        if (GuiButton(pos, "") && !isOSOpen()) {
            GotoSongSelect();
        }
        if (CheckCollisionPointRec(GetMousePosition(), pos) || ControllerSelected == 0) {
            DrawButtonGradient(pos, accentColor);
            color = WHITE;
        }
        Encore::Text::lDrawText(ASSET(redHatDisplayBlack), "generic.play", {pos.x, pos.y}, pos.height, color, LEFT);
    } else {
        Encore::Text::lDrawText(ASSET(redHatDisplayBlack), "mainMenu.invalidCache", {pos.x, pos.y}, pos.height, RED, LEFT);
    }

    {
        color = {170, 170, 170, 255};
        pos.y += u.hinpct(0.09f);

        if (GuiButton(pos, "") && !isOSOpen()) {
            TheMenuManager.CreateAndSwitchMenu<SettingsMenu>();
        }
        if (CheckCollisionPointRec(GetMousePosition(), pos) || ControllerSelected == 1) {
            DrawButtonGradient(pos, accentColor);
            color = WHITE;
        }
        Encore::Text::lDrawText(ASSET(redHatDisplayBlack), "mainMenu.options", {pos.x, pos.y}, pos.height, color, LEFT);
    }

    {
        pos.y += u.hinpct(0.09f);
        color = {170, 170, 170, 255};
        if (CheckCollisionPointRec(GetMousePosition(), pos) || ControllerSelected == 2) {
            DrawButtonGradient(pos, accentColor);
            color = WHITE;
        }
        if (GuiButton(pos, "") && !isOSOpen()) {
            // goes back to attract
            for (int p = 0; p < ThePlayerManager.ActivePlayers.size(); p++) {
                // Checking for OS_ATTRACT here will result in strange behavior
                // but it's better than a crash
                // TODO: can we kill this button?
                // Yes.
                if (OvershellState[p] == OS_ATTRACT && ThePlayerManager.ActivePlayers.at(p) != -1)
                    ThePlayerManager.RemoveActivePlayer(p);
            }
            TitleAnimTimer = 1;
        }
        Encore::Text::lDrawText(ASSET(redHatDisplayBlack), "mainMenu.quit", {pos.x, pos.y}, pos.height, color, LEFT);
    }

    if (GuiButton(
            { (float)GetRenderWidth() - 60,
              (float)GetRenderHeight() - u.hpct(0.15f) - 60,
              60,
              60 },
            ""
        )) {
        OpenURL("https://github.com/Encore-Developers/Encore");
    }

    if (GuiButton(
            { (float)GetRenderWidth() - 120,
              (float)GetRenderHeight() - u.hpct(0.15f) - 60,
              60,
              60 },
            ""
        )) {
        OpenURL("https://discord.gg/GhkgVUAC9v");
    }

    GuiSetFont(menuAss.rubik);
    DrawTextureEx(
        menuAss.github,
        { (float)GetRenderWidth() - 54, (float)GetRenderHeight() - 54 - u.hpct(0.15f) },
        0,
        0.2,
        WHITE
    );
    DrawTextureEx(
        menuAss.discord,
        { (float)GetRenderWidth() - 113, (float)GetRenderHeight() - 48 - u.hpct(0.15f) },
        0,
        0.075,
        WHITE
    );
    if (TheGameJukebox.streamsLoaded) {
        Vector2 TitleSize = MeasureTextEx(
            menuAss.rubikBoldItalic, TheSongList.curSong->title.c_str(), SongFontSize, 0
        );
        Vector2 ArtistSize = MeasureTextEx(
            menuAss.rubikItalic, TheSongList.curSong->artist.c_str(), SongFontSize, 0
        );

        Vector2 SongTitleBox = { u.RightSide - u.hinpct(0.13f),
                                 u.hpct(0.2f) - u.hinpct(0.131f) };
        DrawMiniMTVOverlay(255, SongTitleBox);
        // SongTitleBox.x = SongTitleBox.x - u.hinpct(0.12f);
        // SongArtistBox.x = SongArtistBox.x - u.hinpct(0.12f);
        // DrawTextEx(
        //     menuAss.rubikBoldItalic,
        //     TheSongList.curSong->title.c_str(),
        //     SongTitleBox,
        //     SongFontSize,
        //     0,
        //     WHITE
        // );
        // DrawTextEx(
        //     menuAss.rubikItalic,
        //     TheSongList.curSong->artist.c_str(),
        //     SongArtistBox,
        //     SongFontSize,
        //     0,
        //     WHITE
        // );

        float played = TheAudioManager.GetMusicTimePlayed();
        float length = TheAudioManager.GetMusicTimeLength();
        DrawRectangle(
            0,
            u.hpct(0.2f) - u.hinpct(0.01f),
            Remap(played, 0, length, 0, GetRenderWidth()),
            u.hinpct(0.005f),
            SKYBLUE
        );

        GuiSetStyle(BUTTON, BORDER_WIDTH, 0);
        if (GuiButton(
                { u.RightSide - u.hinpct(0.12f),
                  u.hpct(0.2f) - u.hinpct(0.1f) - u.hinpct(0.031f),
                  u.hinpct(0.06f),
                  u.hinpct(0.06f) },
                TheGameJukebox.playing ? "||" : ">"
            )) {
            TheGameJukebox.TogglePlayback();
        }
        if (GuiButton(
                { u.RightSide - u.hinpct(0.06f),
                  u.hpct(0.2f) - u.hinpct(0.1f) - u.hinpct(0.031f),
                  u.hinpct(0.06f),
                  u.hinpct(0.06f) },
                ">>"
            )) {
            TheGameJukebox.SkipSong();
            albumArtLoaded = false;
        }
        GuiSetStyle(BUTTON, BORDER_WIDTH, 2);
    } else {
        Vector2 TitleSize =
            MeasureTextEx(menuAss.rubikBoldItalic, LOCALISE("mainMenu.noSongLoaded"), SongFontSize, 0);
        Vector2 ArtistSize = MeasureTextEx(menuAss.rubikItalic, "", SongFontSize, 0);

        Vector2 SongTitleBox = { u.RightSide - TitleSize.x - u.winpct(0.01f),
                                 u.hpct(0.2f) - u.hinpct(0.1f) - (TitleSize.y * 1.1f) };
        Vector2 SongArtistBox = { u.RightSide - ArtistSize.x - u.winpct(0.01f),
                                  u.hpct(0.2f) - u.hinpct(0.1f) };
        DrawTextEx(
            menuAss.rubikBoldItalic, LOCALISE("mainMenu.noSongLoaded"), SongTitleBox, SongFontSize, 0, WHITE
        );
        DrawTextEx(menuAss.rubikItalic, "", SongArtistBox, SongFontSize, 0, WHITE);
    }
    GameMenu::DrawBottomOvershell();
    buttReg.DrawPrompts(isOSOpen());
    DrawOvershell();

    DrawWarning({GetRenderWidth() - 520.0f, u.hpct(0.2f)}, {500, 650});
}
// this is really nasty shit as far as im concerned please dont think about it too much
void MainMenu::Draw() {
    TheGameJukebox.Update();

    GameMenu::DrawAlbumArtBackground();


    if (ThePlayerManager.PlayersActive == 0) {
        AttractScreen();
    } else {
        MainMenuScreen();
    }
}
bool AlbumArtLoadingStuff = false;
// sentenced to

void GameMenu::DrawFPS(int posX, int posY) {
    Color color = LIME; // Good FPS
    int fps = GetFPS();

    if ((fps < 45) && (fps >= 15))
        color = ORANGE; // Warning FPS
    else if (fps < 20)
        color = RED; // Low FPS

    DrawTextEx(
        menuAss.josefinSansItalic,
        TextFormat("%2i FPS", fps),
        { (float)posX, (float)posY },
        u.hinpct(0.025f),
        0,
        color
    );
}