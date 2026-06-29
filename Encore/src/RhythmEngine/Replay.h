#pragma once
#include "song/song.h"
#include "users/player.h"
#include "util/Input.h"
#include "util/binary.h"

#include <deque>

namespace Encore::RhythmEngine {
    class Replay {
    public:
        struct ReplayParticipant {
            std::string name;
            int instrument;
            int difficulty;
            ControllerBindingType bindingType;

            // In case we want to create fake players from 
            float noteSpeed;
            float trackLength;
        };

        std::deque<ControllerEvent> inputs;
        SongHash song;

        class ReplayPlayer {
        public:
            Replay* replay;
            std::deque<ControllerEvent>::iterator nextInput;
            double lastUpdateTime;

            ReplayPlayer(Replay& replay);

            void Advance(double time);
            bool HasNextInput();
            ControllerEvent* GetNextInput();
        };

        void Save(encore::bin_ofstream_le& stream);
        void Load(encore::bin_ifstream_le& stream);
    };
}

