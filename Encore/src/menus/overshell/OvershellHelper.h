#pragma once
#include <string>
#include "raylib.h"
#include "users/player.h"
#include "users/playerManager.h"
#include "OvershellMenu.h"
#include "raymath.h"
#include "menus/util/ContinuousTween.h"

namespace encOS {

    class OvershellInputState {
    public:

        static OvershellInputState* currentState;

        bool selectPressed = false;
        bool backPressed = false;

        bool upPressed = false;
        bool downPressed = false;

        bool blockNav = false;

        int i;
        OvershellMenu* menu;
        OSState lastState;

        int menuLength;
        int focusedItem;

        Encore::ContinuousTween<float> raise = {0, 0.5, Encore::Easing::EASE_IN_OUT};

        OvershellInputState(int index) : i(index) {}

        void Begin(OvershellMenu *menu) {
            this->menu = menu;
            currentState = this;
            blockNav = false;
            raise.Update(GetFrameTime());
        }

        void Reset() {
            this->menu = nullptr;
            selectPressed = false;
            backPressed = false;
            upPressed = false;
            downPressed = false;
        }

        void SetLength(int length) {
            if (menuLength != length || lastState != menu->OvershellState[i]) {
                menuLength = length;
                focusedItem = 0;
                lastState = (OSState)menu->OvershellState[i];
            }
            raise = length;
        }

        Player* GetPlayer() {
            if (ThePlayerManager.ActivePlayers[i] != -1) {
                return &ThePlayerManager.GetActivePlayer(i);
            }
        }

        SDL_JoystickID GetJoystick() {
            if (menu->ControllersToAssign[i] != 0) {
                return menu->ControllersToAssign[i];
            }
            if (GetPlayer()) {
                return GetPlayer()->joypadID;
            }
            return 0;
        }

        void ControllerInput(Encore::ControllerEvent event) {
            if (event.action != Encore::Action::PRESS) {
                return;
            }

            switch (event.channel) {
            case Encore::InputChannel::STRUM_UP:
                if (!blockNav) {
                    focusedItem--;
                }
                upPressed = true;
                break;
            case Encore::InputChannel::STRUM_DOWN:
                if (!blockNav) {
                    focusedItem++;
                }
                downPressed = true;
                break;
            case Encore::InputChannel::LANE_1:
                selectPressed = true;
                break;
            case Encore::InputChannel::LANE_2:
                backPressed = true;
                break;
            default:;
            }

            if (focusedItem >= menuLength) {
                focusedItem = 0;
            }
            if (focusedItem < 0) {
                focusedItem = menuLength-1;
            }
        }
    };

    inline float osLeft = 0;
    inline float osWidth = 0;

    bool OvershellButton(int slot, int x, std::string string);
    void OvershellText(int slot, int x, std::string string);

    bool OvershellSlider(
        int slot, int x, std::string string, float *value, float step, float min, float max
    );
    void DrawBeacon(int slot, float x, float y, float width, float height, bool top, Color playerColor);
    bool DrawOvershellRectangleHeader(
        float x,
        float y,
        float width,
        float height,
        std::string username,
        Color accentColor,
        Color usernameColor,
        bool drawBG = true
    );
    bool DrawOvershellBottomCover(float x, float width, int state, Color accentColor);
    float GetYPos(int buttonIndex, bool ignoreRaise = false);
    bool OvershellCheckbox(int slot, int x, std::string string, bool initialVal);

    extern OvershellInputState inputStates[MAX_PLAYERS];
}
