//
// Created by maria on 09/05/2025.
//

#ifndef REENUMS_H
#define REENUMS_H
#include <cstdint>

#endif // REENUMS_H
namespace Encore::RhythmEngine {
    constexpr uint8_t PlasticFrets[6] { // open			0		     0| technically not a
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
                                        0b000000
    };

    enum class InputChannel {
        LANE_1 = 0,
        LANE_2 = 1,
        LANE_3 = 2,
        LANE_4 = 3,
        LANE_5 = 4,
        STRUM_UP = 7,
        STRUM_DOWN = 8,
        PAUSE = 9,
        OVERDRIVE = 10
    };
    inline int ICInt(InputChannel channel) {
        switch (channel) {
        case InputChannel::LANE_1:
            return 0;
        case InputChannel::LANE_2:
            return 1;
        case InputChannel::LANE_3:
            return 2;
        case InputChannel::LANE_4:
            return 3;
        case InputChannel::LANE_5:
            return 4;
        }

        return 0;
    }
    enum class Action {
        PRESS = 1,
        RELEASE = 0,
        REPEAT = 2 // not needed but whatever
    };
}