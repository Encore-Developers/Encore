

#ifndef ENCORE_OVERSHELL_H
#define ENCORE_OVERSHELL_H
#include "OvershellSlot.h"

#include <vector>

namespace Encore {

    class Overshell {
        float animSlide = 0.0;

    public:
        std::vector<OvershellSlot>& slots;

        Overshell(std::vector<OvershellSlot>& slots) : slots(slots) {}

        // Overshell can eat inputs, returning true if it wants to do so
        bool keyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods);
        bool gamepadStateCallback(RhythmEngine::ControllerEvent event);

        OvershellSlot *getSlotForPad(EncorePadID pad) const;

        void Draw();
    };

} // Encore

#endif // ENCORE_OVERSHELL_H
