//
// Created by maria on 09/05/2025.
//

#ifndef REENUMS_H
#define REENUMS_H
#include "SDL3/SDL_joystick.h"

#include <cstdint>


namespace Encore::RhythmEngine {
    constexpr uint8_t PlasticFrets[7] { // open			0		     0| technically not a
                                        // "fretted note" so i put it on
                                        // the empty space
                                        0b000001,
                                        // green		1		|____0|
                                        0b000010,
                                        // red			2		|___0_|
                                        // gr chord		3		|___00|
                                        0b000100,
                                        // yellow		4		|__0__|
                                        // gy chord		5	    |__0_0|
                                        // ry chord		6		|__00_|
                                        // gry chord	7		|__000|
                                        0b001000,
                                        // blue			8		|_0___|
                                        // gb chord		9		|_0__0|
                                        // rb chord		10		|_0_0_|
                                        // grb chord	11		|_0_0_|
                                        // yb chord		12		|_00__|
                                        // gyb chord	13		|_00_0|
                                        // ryb chord	14		|_000_|
                                        // gryb chord	15		|_0000|
                                        0b010000,
                                        // orange		16		|0____|
                                        // go chord		17		|0___0|
                                        // ro chord		18		|0__0_|
                                        // gro chord	19		|0__00|
                                        // yo chord		20		|0_0__|
                                        // gyo chord	21		|0_0_0|
                                        // ryo chord	22		|0_00_|
                                        // gryo chord	23		|0_000|
                                        // bo chord		24		|00___|
                                        // gbo chord	25		|00__0|
                                        // rbo chord	26		|00_0_|
                                        // grbo chord	27		|00_00|
                                        // ybo chord	28		|000__|
                                        // gybo chord	29		|000_0|
                                        // rybo chord	30		|0000_|
                                        // grybo chord	31		|00000|
                                        0b100000, // The horrors.
                                        0b000000
    };

    enum Judgement : int {
        GOOD = 0,
        PERFECT = 1
    };
}
#endif // REENUMS_H