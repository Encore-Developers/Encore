

#ifndef ENCORE_OVERSHELL_H
#define ENCORE_OVERSHELL_H
#include "OvershellSlot.h"
#include "util/ContinuousTween.h"

#include <vector>

namespace Encore {

    class Overshell {
        ContinuousTween<float> animSlide;

    public:
        std::vector<OvershellSlot>& slots;

        Overshell(std::vector<OvershellSlot>& slots) : animSlide(0.0, 0.5, Easing::EASE_IN_OUT), slots(slots) {}

        // Overshell can eat inputs, returning true if it wants to do so
        bool keyCallback(GLFWwindow *wind, int key, int scancode, int action, int mods);
        bool gamepadStateCallback(RhythmEngine::ControllerEvent event);

        OvershellSlot *getSlotForPad(EncorePadID pad) const;

        void Draw();
    };

} // Encore

#endif // ENCORE_OVERSHELL_H
