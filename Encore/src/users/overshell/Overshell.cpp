
#include "Overshell.h"

#include "menus/MenuManager.h"
#include "raymath.h"
#include "raylib.h"
#include "rlgl.h"
#include "menus/uiUnits.h"

using namespace Encore;

// TODO: we should probably make keyboard a fake controller, give it an id of -1 maybe?

bool Overshell::keyCallback(
    GLFWwindow *wind, int key, int scancode, int action, int mods
) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ENTER) {
            RhythmEngine::ControllerEvent event;
            event.action = RhythmEngine::Action::PRESS;
            event.channel = RhythmEngine::InputChannel::PAUSE;
            event.slot = -1;
            return gamepadStateCallback(event);
        }
    }
    return false;
}
bool Overshell::gamepadStateCallback(RhythmEngine::ControllerEvent event) {
    if (auto slot = getSlotForPad(event.slot)) {
        if (event.action == RhythmEngine::Action::PRESS && event.channel == RhythmEngine::InputChannel::PAUSE) {
            slot->ToggleOpen();
            return true;
        }
        if (slot->open) {
            // bla bla bla let the slot do slot things
            return true;
        }
    } else {
        if (event.action == RhythmEngine::Action::PRESS && event.channel == RhythmEngine::InputChannel::PAUSE) {
            slots.push_back(OvershellSlot(nullptr));
            return true;
        }
    }

    return false;
}
OvershellSlot *Overshell::getSlotForPad(EncorePadID pad) const {
    for (auto &slot : slots) {
        if (slot.pad == pad) {
            return &slot;
        }
    }
    return nullptr;
}
void Overshell::Draw() {
    // TODO: find a way to do ease in and out on this slide anim
    animSlide = Lerp(animSlide, TheMenuManager.ActiveMenu && TheMenuManager.ActiveMenu->showOvershell ? 1 : 0, 1 - exp(-20*GetFrameTime()));


    if (animSlide < 0.01) {
        return;
    }
    Units& u = Units::getInstance();

    // how far the slots stick out from above the overshell background
    float slotPoke = u.hinpct(0.03f);
    float overshellHeight = u.hinpct(0.07f);

    rlPushMatrix();
    rlTranslatef(0, (1-animSlide)*(slotPoke+overshellHeight), 0);

    DrawRectangle(0, GetRenderHeight()-overshellHeight-slotPoke, GetRenderWidth(), overshellHeight, {0, 0, 0, 50});
    DrawRectangle(0, GetRenderHeight()-overshellHeight, GetRenderWidth(), overshellHeight, ColorBrightness(GetColor(0x181827FF), -0.5f));

    rlPopMatrix();
}