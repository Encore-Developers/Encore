
#ifndef ENCORE_CONTINUOUSTWEEN_H
#define ENCORE_CONTINUOUSTWEEN_H
#include "raymath.h"

namespace Encore {
    enum class Easing {
        NONE = 0b00,
        EASE_IN = 0b01,
        EASE_OUT = 0b10,
        EASE_IN_OUT = 0b11,
    };

    enum class EasingFormula {
        QUAD
    };

    template <typename T> class ContinuousTween {
    public:
        T startValue;
        T targetValue;
        T currentValue;
        float duration;
        float timer;
        Easing easing;
        EasingFormula formula = EasingFormula::QUAD;

        ContinuousTween(T initialValue, float duration, Easing easing = Easing::NONE) {
            this->startValue = initialValue;
            this->targetValue = initialValue;
            this->duration = duration;
            this->timer = 1;
            this->easing = easing;
        }

        void Update(float deltaTime) {
            if (timer < 0) {
                timer = 0;
            }
            if (timer < duration) {
                timer += deltaTime;
            }
            if (timer >= duration) {
                timer = duration;
            }
            float frac = ApplyEasing(timer / duration);
            currentValue = Interp(startValue, targetValue, frac);
        }

        void AnimateToValue(T newValue) {
            timer = 0;
            startValue = currentValue;
            targetValue = newValue;
            Update(0);
        }

        ContinuousTween& operator=(T newValue) {
            if (newValue == targetValue) {
                return *this;
            }
            AnimateToValue(newValue);
            return *this;
        }

        operator T() {
            return currentValue;
        }

        static T Interp(T startValue, T endValue, float frac) {
            frac = Clamp(frac, 0, 1);
            T delta = endValue - startValue;
            return startValue + frac * delta;
        }
        float ApplyEasing(float frac) {
            float mult = 1.0f;
            float add = 0.0f;
            frac = Clamp(frac, 0, 1);
            if (easing == Easing::NONE) {
                return frac;
            }
            // we're converting the frac to the ease in case
            switch (easing) {
            case Easing::EASE_IN:
                break;
            case Easing::EASE_OUT:
                frac = 1 - frac;
                mult = -1.0f;
                add = 1.0f;
                break;
            case Easing::EASE_IN_OUT:
                if (frac < 0.5) {
                    frac *= 2;
                    mult = 0.5;
                } else {
                    frac = 1 - 2 * (frac - 0.5);
                    add = 1.0f;
                    mult = -0.5f;
                }
            default:
                break;
            }

            switch (formula) {
            case EasingFormula::QUAD:
                return frac * frac * mult + add;
            default:
                return frac * mult + add;
            }

        }
    };
}


#endif // ENCORE_CONTINUOUSTWEEN_H
