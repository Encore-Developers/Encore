#include "OvershellMenu.h"

#include "../gameplay/GameplayMenu.h"
#include "../MenuManager.h"
#include "assets.h"
#include "../util/uiUnits.h"
#include "../main/MainMenu.h"
#include "users/playerManager.h"
#include "OvershellHelper.h"
#include "raygui.h"
#include "raylib.h"
#include "menus/util/locale/Locale.h"
#include "gameplay/enctime.h"
#include "menus/gameplay/ChartLoadingMenu.h"
#include "menus/gameplay/PracticeMenu.h"
#include "menus/gameplay/ReadyUpMenu.h"
#include "menus/main/SongSelectMenu.h"
#include "users/profiles/ProfileManager.h"

using namespace encOS;
using namespace Encore::RhythmEngine;

bool OvershellKeyboardInputCallback(OvershellMenu *menu, SDL_KeyboardEvent* event) {
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (menu->OvershellState[i] == OS_CONTROLLER_ASSIGNMENT && event->key == SDLK_RETURN) {
            ThePlayerManager.GetActivePlayer(i).joypadID = -1;
            menu->OvershellState[i] = OS_OPTIONS;
            return true;
        }
    }
    return false;
}

const std::unordered_map<std::string, ControllerBindingType> OvershellMenu::hardcodedControllerTypes = {
    {"060074ae6f0e00004802000000010000", GUITAR} // RB4 Jaguar/Riffmaster
};

void DetectControllerType(Player& player) {
    auto type = SDL_GetJoystickType(SDL_GetJoystickFromID(player.joypadID));
    char guidStr[33];
    SDL_GUIDToString(SDL_GetJoystickGUIDForID(player.joypadID), guidStr, 33);
    std::string guid = guidStr;
    if (OvershellMenu::hardcodedControllerTypes.contains(guid)) {
        player.bindingType = OvershellMenu::hardcodedControllerTypes.at(guid);
        return;
    }
    switch (type) {
    case SDL_JOYSTICK_TYPE_GUITAR:
        player.bindingType = GUITAR;
        if (player.joypadID > 0) {
            if (SDL_GetJoystickVendorForID(player.joypadID) == 0x12ba && SDL_GetJoystickProductForID(player.joypadID) == 0x0100) {
                player.bindingType = GUITAR_GHPS3;
            }
        }
        break;
    case SDL_JOYSTICK_TYPE_DRUM_KIT:
        player.bindingType = DRUMS;
        break;
    default:
        player.bindingType = PAD;
    }
}

// required to be changed for more than 4 players
OvershellInputState encOS::inputStates[] = {0, 1, 2, 3};
OvershellInputState* OvershellInputState::currentState = nullptr;

bool OvershellControllerInputCallback(OvershellMenu *menu, Encore::ControllerEvent event) {
    if (event.channel == Encore::InputChannel::WHAMMY || event.channel == Encore::InputChannel::INVALID) {
        return false;
    }
    if (event.action == Encore::Action::RELEASE) {
        return false;
    }
    bool controllerSignedIn = false;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        bool thisSlotIsController = false;
        auto playerId = ThePlayerManager.ActivePlayers[i];
        if (menu->OvershellState[i] == OS_CONTROLLER_ASSIGNMENT) {
            auto &player = ThePlayerManager.GetActivePlayer(i);
            player.joypadID = event.slot;
            player.ActiveSlot = i;
            DetectControllerType(player);
            menu->OvershellState[i] = OS_OPTIONS;
            return true;
        }
        if (playerId != -1) {
            auto &player = ThePlayerManager.GetActivePlayer(i);
            if (player.joypadID == event.slot) {
                controllerSignedIn = true;
                thisSlotIsController = true;
            }
        }
        if (menu->ControllersToAssign[i] == event.slot) {
            controllerSignedIn = true;
            thisSlotIsController = true;
        }
        if (thisSlotIsController) {
            if (event.channel == Encore::InputChannel::PAUSE) {
                switch (menu->OvershellState[i]) {
                case OS_ATTRACT:
                    menu->OvershellState[i] = OS_OPTIONS;
                    return true;
                case OS_OPTIONS:
                    menu->OvershellState[i] = OS_ATTRACT;
                    return true;
                }
            }
            if (menu->OvershellState[i] != OS_ATTRACT) {
                inputStates[i].ControllerInput(event);
                return true;
            }
        }

    }
    if (menu->dropInDropOut && (event.channel == Encore::InputChannel::PAUSE || event.channel == Encore::InputChannel::LANE_1) && !controllerSignedIn) {
        for (int i = 0; i < 4; i++) {
            if (ThePlayerManager.ActivePlayers[i] == -1 && menu->ControllersToAssign[i] == 0) {
                menu->ControllersToAssign[i] = event.slot;
                menu->OvershellState[i] = OS_PLAYER_SELECTION;
                return true;
            }
        }
    }
    return false;
}

bool OvershellMenu::isOSOpen() {
    for (const auto state : OvershellState) {
        if (state != OS_ATTRACT) return true;
    }
    return false;
}

void OvershellMenu::DrawOvershell() {
    ZoneScoped
    Assets &assets = Assets::getInstance();
    Units &unit = Units::getInstance();
float BottomBottomOvershell = GetRenderHeight() - unit.hpct(0.13f);
    float InnerBottom = BottomBottomOvershell + unit.hinpct(0.005f);

    DrawRectangleGradientV(
        0,
        BottomBottomOvershell,
        GetRenderWidth(),
        unit.hinpct(0.13f) + 1,
        Color { 32, 32, 51, 255 },
        Color { 24, 24, 39, 255 }
    );
    DrawRectangleGradientV(
        0,
        BottomBottomOvershell,
        GetRenderWidth(),
        unit.hinpct(0.0075f),
        Color { 0, 0, 0, 64 },
        Color { 0, 0, 0, 0 }
        );
    float LeftMin = unit.wpct(0.025f);
    float LeftMax = unit.wpct(0.975f);
    GuiSetFont(ASSET(rubik));
    float ButtonHeight = unit.winpct(0.03f);

    float OvershellTopLoc = BottomBottomOvershell + unit.hinpct(0.02f);
    float osCoverHeight = unit.hinpct(0.11f);
    float osBottom = GetYPos();
    PlayerManager &playerManager = ThePlayerManager;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        osLeft = GetOvershellSlotLeft(i);
        osWidth = GetOvershellSlotWidth();
        bool EmptySlot = true;
        OvershellInputState& input = inputStates[i];
        input.Begin(this);

        float SlotWidth = GetOvershellSlotWidth();
        float OvershellCenterLoc = (unit.wpct(0.125) + (unit.winpct(0.25) * i));
        float HalfWidth = OvershellCenterLoc - osLeft;
        Color headerUsernameColor;
        if (playerManager.ActivePlayers.at(i) != -1) {
            if (playerManager.GetActivePlayer(i).Bot)
                headerUsernameColor = SKYBLUE;
            else {
                if (playerManager.GetActivePlayer(i).BrutalMode)
                    headerUsernameColor = RED;
                else
                    headerUsernameColor = WHITE;
            }
        }
        switch (OvershellState[i]) {
        case CREATION: {
            static char name[32] = { 0 };
            Rectangle textBoxPosition { osLeft,
                                        osBottom - (ButtonHeight * 3),
                                        SlotWidth,
                                        float(ButtonHeight) };
            GuiTextBox(textBoxPosition, name, 32, true);
            input.SetLength(2);
            if (OvershellButton(i, 1, "Confirm")) {
                playerManager.CreatePlayer(name);
                playerManager.AddActivePlayer(playerManager.PlayerList.size() - 1, i);
                CancelButtonActivation = true;
                OvershellState[i] = OS_ATTRACT;
                *name = 0;
                continue;
            }
            if (OvershellButton(i, 0, "Cancel")) {
                OvershellState[i] = OS_ATTRACT;
                *name = 0;
                continue;
            }
            break;
        }
        case OS_PLAYER_SELECTION: {
            // for selecting players
            float InfoLoc = (playerManager.PlayerList.size() + 2);
            if (OvershellButton(i, 0, "Cancel")) {
                CancelButtonActivation = true;
                OvershellState[i] = OS_ATTRACT;
                ControllersToAssign[i] = 0;
            }
            BeginBlendMode(BLEND_MULTIPLIED);
            DrawRectangleRec(
                { osLeft,
                  osBottom - ButtonHeight,
                  SlotWidth,
                  ButtonHeight },
                Color { 255, 0, 0, 128 }
            );
            EndBlendMode();
            int pos = 0;
            if (!playerManager.PlayerList.empty()) {
                for (int x = 0; x < playerManager.PlayerList.size(); x++) {
                    bool playerAlreadyLoggedIn = false;
                    for (int s = 0; s < playerManager.ActivePlayers.size(); s++) {
                        if (playerManager.ActivePlayers[s] == x) {
                            playerAlreadyLoggedIn = true;
                            break;
                        }
                    }
                    if (playerAlreadyLoggedIn) {
                        continue;
                    }
                    if (OvershellButton(
                        i, pos + 2, playerManager.PlayerList[x].Name.c_str()
                    )) {
                        playerManager.AddActivePlayer(x, i);

                        if (ControllersToAssign[i] != 0) {
                            playerManager.GetActivePlayer(i).joypadID = ControllersToAssign[i];
                            playerManager.GetActivePlayer(i).ActiveSlot = i;
                            DetectControllerType(playerManager.GetActivePlayer(i));
                            ControllersToAssign[i] = 0;
                        }
                        CancelButtonActivation = true;
                        OvershellState[i] = OS_ATTRACT;
                    }
                    pos++;
                }
            }
            input.SetLength(pos + 2);
            DrawOvershellRectangleHeader(
                osLeft,
                OvershellTopLoc - (ButtonHeight * (pos+2)),
                osWidth,
                osCoverHeight,
                "Select a player",
                Color { 255, 0, 255, 255 },
                WHITE,
                false
            );
            if (OvershellButton(i, 1, "New Profile")) {
                OvershellState[i] = CREATION;
            }
            // 0 is "none"   1 is "join"   2 is "open"   3 is "close"   cba to make enum
            DrawOvershellBottomCover(osLeft, osWidth, 3, Color{255,0,255,255});
            break;
        }
        case OS_OPTIONS: {

            int len = dropInDropOut ? 9 : 6;
            if (!dropInDropOut && TheSongList.PlaylistMode && TheSongList.playlist.size() > 1) {
                len++;
            }
            int curSlot = len-1;
            if (DrawOvershellRectangleHeader(
                    osLeft,
                    OvershellTopLoc - (ButtonHeight * len),
                    osWidth,
                    osCoverHeight,
                    playerManager.GetActivePlayer(i).Name,
                    playerManager.GetActivePlayer(i).AccentColor,
                    headerUsernameColor
                )) {
                OvershellState[i] = OS_ATTRACT;
                CancelButtonActivation = true;
                continue;
            }
            input.SetLength(len);
            if (!dropInDropOut) {
                if (OvershellButton(i, curSlot--,LOCALIZE("overshell.return")) || input.backPressed) {
                    OvershellState[i] = OS_ATTRACT;
                    ThePlayerManager.SaveSpecificPlayer(i, true);
                }
                if (OvershellButton(i, curSlot--,LOCALIZE("generic.restart"))) {
                    if (auto gameplayMenu = dynamic_cast<GameplayMenu*>(this)) {
                        TheAudioManager.unloadStreams();
                        for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
                            Player &player = ThePlayerManager.GetActivePlayer(i);
                            player.engine->stats.reset();
                            player.engine->chart.reset();
                            player.engine.reset();
                        }
                        TheSongTime.FullReset();
                        TheMenuManager.CreateAndSwitchMenu<ChartLoadingMenu>(gameplayMenu->curSong);
                    }
                }
                if (OvershellButton(i, curSlot--,LOCALIZE("overshell.exitSong"))) {
                    TheAudioManager.unloadStreams();
                    TheSongTime.FullReset();
                    for (int i = 0; i < MAX_PLAYERS; i++) {
                        if (ThePlayerManager.ActivePlayers[i] == -1) continue;
                        Player &player = ThePlayerManager.GetActivePlayer(i);
                        player.engine->stats.reset();
                        player.engine->chart.reset();
                        player.engine.reset();
                    }

                    TheSongList.PlaylistMode = false;
                    TheSongList.playlist.clear();
                    TheSongList.PlaylistSize = 0;
                    TheSongList.PlaylistIndex = 0;
                    TheMenuManager.CreateAndSwitchMenu<SongSelectMenu>();
                }
                if (TheSongList.PlaylistMode && TheSongList.playlist.size() > 1) {
                    if (OvershellButton(i, curSlot--,LOCALIZE("overshell.nextSong"))) {
                        TheAudioManager.unloadStreams();
                        TheSongTime.FullReset();
                        for (int i = 0; i < MAX_PLAYERS; i++) {
                            if (ThePlayerManager.ActivePlayers[i] == -1) continue;
                            Player &player = ThePlayerManager.GetActivePlayer(i);
                            player.engine->stats.reset();
                            player.engine->chart.reset();
                            player.engine.reset();
                        }
                        TheSongList.playlist.pop_front();
                        TheSongList.PlaylistIndex++;
                        TheMenuManager.CreateAndSwitchMenu<ReadyUpMenu>(TheSongList.playlist.front());
                    }
                }
                // if (OvershellButton(i, curSlot--, LOCALIZE("generic.practice"))) {
                //     if (auto gameplayMenu = dynamic_cast<GameplayMenu*>(this)) {
                //         TheMenuManager.CreateAndSwitchMenu<ChartLoadingMenu>(gameplayMenu->curSong, ChartLoadingMenu::PRACTICE);
                //     }
                // }
            }

            auto bnsString = LOCALIZE_FMT("overshell.breakneckSpeed", playerManager.GetActivePlayer(i).NoteSpeed);
            if (!BNSetting) {
                if (OvershellButton(i, curSlot--, bnsString)) {
                    BNSetting = true;
                }
            } else {
                if (OvershellSlider(
                    i,
                    curSlot--,
                    bnsString,
                    &playerManager.GetActivePlayer(i).NoteSpeed,
                    0.05,
                    0.25,
                    3
                )) {
                    BNSetting = false;
                };
            }
            if (OvershellButton(i, curSlot--,LOCALIZE("overshell.colorProfiles"))) {
                OvershellState[i] = OS_COLOR_PROFILE_TYPE_SELECTOR;
            }
            if (dropInDropOut) {
                const char* typeString;
                switch (playerManager.GetActivePlayer(i).bindingType) {
                case GUITAR:
                case GUITAR_GHPS3:
                    typeString = LOCALIZE("overshell.types.guitar");
                    break;
                case DRUMS:
                    typeString = LOCALIZE("overshell.types.drums");
                    break;
                case PAD:
                    typeString = LOCALIZE("overshell.types.pad");
                    break;
                default:
                    typeString = LOCALIZE("generic.unknown");
                }
                if (OvershellButton(i, curSlot--,LOCALIZE_FMT("overshell.instrumentType", typeString))) {
                    OvershellState[i] = OS_INSTRUMENT_SELECTIONS;
                }
                auto padId = playerManager.GetActivePlayer(i).joypadID;
                const char* padName = "Unknown";
                if (padId == -2) {
                    padName = "All";
                }
                if (padId == -1) {
                    padName = "Keyboard";
                }
                if (SDL_GetJoystickFromID(padId)) {
                    padName = SDL_GetJoystickNameForID(padId);
                }
                GuiSetStyle(DEFAULT, TEXT_SIZE, (int)unit.hinpct(0.018f));
                if (OvershellButton(i, curSlot--,LOCALIZE_FMT("overshell.currentController", padName))) {
                    OvershellState[i] = OS_CONTROLLER_ASSIGNMENT;
                    break;
                }
                GuiSetStyle(DEFAULT, TEXT_SIZE, (int)unit.hinpct(0.03f));

                playerManager.GetActivePlayer(i).BrutalMode =
                OvershellCheckbox(i, curSlot--,LOCALIZE("overshell.brutalMode"), playerManager.GetActivePlayer(i).BrutalMode);
                playerManager.GetActivePlayer(i).Bot =
                    OvershellCheckbox(i, curSlot--,LOCALIZE("overshell.autoplay"), playerManager.GetActivePlayer(i).Bot);
            }
            playerManager.GetActivePlayer(i).LeftyFlip = OvershellCheckbox(
                i, curSlot--,LOCALIZE("overshell.leftyFlip"), playerManager.GetActivePlayer(i).LeftyFlip
            );
            if (dropInDropOut) {
                if (OvershellButton(i, curSlot--,LOCALIZE("overshell.dropOut"))) {
                    playerManager.SaveSpecificPlayer(i, true);;
                    playerManager.RemoveActivePlayer(i);
                    OvershellState[i] = OS_ATTRACT;
                    CancelButtonActivation = true;
                    continue;
                }
                if (OvershellButton(i, curSlot--,LOCALIZE("generic.cancel")) || input.backPressed) {
                    playerManager.SaveSpecificPlayer(i, true);
                    OvershellState[i] = OS_ATTRACT;
                    CancelButtonActivation = true;
                }
            }

            DrawOvershellBottomCover(osLeft, osWidth, 3, playerManager.GetActivePlayer(i).AccentColor);
            break;
        }
        case OS_CONTROLLER_ASSIGNMENT: {
            DrawOvershellRectangleHeader(
               osLeft,
               OvershellTopLoc - (ButtonHeight * 3),
               osWidth,
               osCoverHeight,
               playerManager.GetActivePlayer(i).Name,
               playerManager.GetActivePlayer(i).AccentColor,
               headerUsernameColor
            );
            DrawRectangle(osLeft,
                    osBottom - (ButtonHeight * 3),
                    osWidth,
                    (ButtonHeight * 3), ColorBrightness({ 128, 0, 255, 255 }, -0.8));
            OvershellText(i, 2, "Press a button on any controller or press ENTER.");
            DrawOvershellBottomCover(osLeft, osWidth, 0, playerManager.GetActivePlayer(i).AccentColor);
            break;
        }
        case OS_ATTRACT: {
            input.menuLength = 0;
            if (playerManager.ActivePlayers[i] != -1) {
                // player active
                Color headerUsernameColor =
                    playerManager.GetActivePlayer(i).Bot ? SKYBLUE : WHITE;
                DrawOvershellBottomCover(osLeft, osWidth, 2, playerManager.GetActivePlayer(i).AccentColor);
                if (DrawOvershellRectangleHeader(
                        osLeft,
                        OvershellTopLoc,
                        osWidth,
                        osCoverHeight,
                        playerManager.GetActivePlayer(i).Name,
                        playerManager.GetActivePlayer(i).AccentColor,
                        headerUsernameColor
                    )) {
                    OvershellState[i] = OS_OPTIONS;
                    CancelButtonActivation = false;
                    continue;
                    // playerManager.RemoveActivePlayer(i);
                };
            } else { // no active players
                // if its the first slot, keyboard can join ALWAYS
                if (!dropInDropOut) {
                    continue;
                }
                int joysticks;
                SDL_GetJoysticks(&joysticks);
                if (i <= joysticks || i == 0) {
                    DrawOvershellBottomCover(osLeft, osWidth, 1, LIGHTGRAY);
                    if (DrawOvershellRectangleHeader(
                            osLeft,
                            OvershellTopLoc,
                            osWidth,
                            osCoverHeight,
                            LOCALIZE("overshell.pressStart"),
                            LIGHTGRAY,
                            RAYWHITE
                        )) {
                        CancelButtonActivation = false;
                        OvershellState[i] = OS_PLAYER_SELECTION;
                        continue;
                    };
                } else {
                    float asdfsdf = unit.hinpct(0.11f * 0.4f);
                    if (DrawOvershellRectangleHeader(
                            osLeft,
                            OvershellTopLoc + asdfsdf,
                            osWidth,
                            osCoverHeight,
                            LOCALIZE("overshell.connectController"),
                            { 255,255,255,32 },
                            LIGHTGRAY
                        )) {
                        CancelButtonActivation = false;
                        OvershellState[i] = OS_PLAYER_SELECTION;
                        continue;
                    };
                    ;
                }
            }

            break;
        }
        case OS_INSTRUMENT_SELECTIONS: {
            // int ButtonHeight = unit.winpct(0.03f);
            Color headerUsernameColor =
                playerManager.GetActivePlayer(i).Bot ? SKYBLUE : WHITE;
            DrawOvershellRectangleHeader(
                osLeft,
                OvershellTopLoc - (ButtonHeight * 4),
                osWidth,
                osCoverHeight,
                playerManager.GetActivePlayer(i).Name,
                playerManager.GetActivePlayer(i).AccentColor,
                headerUsernameColor
            );
            input.SetLength(4);

            if (OvershellButton(i, 3,LOCALISE("overshell.types.guitar"))) {
                playerManager.GetActivePlayer(i).bindingType = GUITAR;
                Uint32 joyId = playerManager.GetActivePlayer(i).joypadID;
                if (joyId > 0) {
                    if (SDL_GetJoystickVendorForID(joyId) == 0x12ba && SDL_GetJoystickProductForID(joyId) == 0x0100) {
                    playerManager.GetActivePlayer(i).bindingType = GUITAR_GHPS3;
                    }
                }
                OvershellState[i] = OS_OPTIONS;
            }
            if (OvershellButton(i, 2,LOCALISE("overshell.types.drums"))) {
                playerManager.GetActivePlayer(i).bindingType = DRUMS;
                OvershellState[i] = OS_OPTIONS;
            }
            if (OvershellButton(i, 1,LOCALISE("overshell.types.pad"))) {
                playerManager.GetActivePlayer(i).bindingType = PAD;
                OvershellState[i] = OS_OPTIONS;
            }

            if (OvershellButton(i, 0,LOCALISE("generic.back")) || input.backPressed) {
                OvershellState[i] = OS_OPTIONS;
            }

            DrawOvershellBottomCover(osLeft, osWidth, 3, playerManager.GetActivePlayer(i).AccentColor);
            break;
        }
        case OS_COLOR_PROFILE_TYPE_SELECTOR: {
            if (OvershellButton(i, 0,LOCALISE("generic.cancel")) || input.backPressed) {
                CancelButtonActivation = true;
                OvershellState[i] = OS_OPTIONS;
                ControllersToAssign[i] = 0;
            }
            BeginBlendMode(BLEND_MULTIPLIED);
            DrawRectangleRec(
                { osLeft,
                  osBottom - ButtonHeight,
                  osWidth,
                  unit.winpct(0.03f) },
                Color { 255, 0, 0, 128 }
            );
            EndBlendMode();
            if (OvershellButton(i, 1,LOCALISE("overshell.types.drums"))) {
                ColorProfileType[i] = Encore::ProfileManager::DRUMS;
                OvershellState[i] = OS_COLOR_PROFILE_SELECTION;
            }
            if (OvershellButton(i, 2,LOCALISE("overshell.types.pad"))) {
                ColorProfileType[i] = Encore::ProfileManager::PAD;
                OvershellState[i] = OS_COLOR_PROFILE_SELECTION;
            }
            if (OvershellButton(i, 3,LOCALISE("overshell.types.plastic"))) {
                ColorProfileType[i] = Encore::ProfileManager::PLASTIC;
                OvershellState[i] = OS_COLOR_PROFILE_SELECTION;
            }

            input.SetLength(4);
            DrawOvershellRectangleHeader(
                osLeft,
                OvershellTopLoc - (ButtonHeight * 4),
                osWidth,
                osCoverHeight,
                playerManager.GetActivePlayer(i).Name,
                playerManager.GetActivePlayer(i).AccentColor,
                WHITE,
                false
            );

            DrawOvershellBottomCover(osLeft, osWidth, 3, playerManager.GetActivePlayer(i).AccentColor);
            break;
        }
        case OS_COLOR_PROFILE_SELECTION: {
            float InfoLoc = (TheProfileManager.ColorProfiles.size() + 1);
            if (OvershellButton(i, 0, "Cancel")) {
                CancelButtonActivation = true;
                OvershellState[i] = OS_COLOR_PROFILE_TYPE_SELECTOR;
                ControllersToAssign[i] = 0;
            }

            DrawOvershellRectangleHeader(
                osLeft,
                OvershellTopLoc - (ButtonHeight * (TheProfileManager.ColorProfiles.size() + 1)),
                osWidth,
                osCoverHeight,
                playerManager.GetActivePlayer(i).Name,
                playerManager.GetActivePlayer(i).AccentColor,
                WHITE,
                false
            );
            BeginBlendMode(BLEND_MULTIPLIED);
            DrawRectangleRec(
                { osLeft,
                  osBottom - ButtonHeight,
                  osWidth,
                  ButtonHeight },
                Color { 255, 0, 0, 128 }
            );
            EndBlendMode();
            int pos = 0;
            if (!TheProfileManager.ColorProfiles.empty()) {
                for (auto &x : TheProfileManager.ColorProfiles) {
                    if (OvershellButton(i, pos + 1, x.second.Name)) {
                        playerManager.GetActivePlayer(i).SetColorProfile(x.second.Name, ColorProfileType[i]);
                        OvershellState[i] = OS_OPTIONS;
                    }
                    pos++;
                }
            }
            input.SetLength(pos + 1);

            //if (OvershellButton(i, pos + 1, "Select a profile")) {}

            DrawOvershellBottomCover(osLeft, osWidth, 3, playerManager.GetActivePlayer(i).AccentColor);
            break;
        }
        }
        input.Reset();
    }
}
float padding = 0.1f;


float OvershellMenu::GetOvershellSlotLeft(size_t slot) {
    Units &u = Units::getInstance();
    float LeftMin = u.wpct(0.01f);
    float LeftMax = u.wpct(0.99f);
    float MaxWidth = LeftMax - LeftMin;
    float AlottedSlotWidth = MaxWidth / MAX_PLAYERS;
    float RemaningPadding = AlottedSlotWidth * padding * slot / (MAX_PLAYERS - 1);
    return (AlottedSlotWidth * slot) + LeftMin + RemaningPadding;
}

float OvershellMenu::GetOvershellSlotWidth() {
    Units &u = Units::getInstance();
    float LeftMin = u.wpct(0.01f);
    float LeftMax = u.wpct(0.99f);
    float MaxWidth = LeftMax - LeftMin;
    return (MaxWidth / MAX_PLAYERS) * (1 - padding);
}
