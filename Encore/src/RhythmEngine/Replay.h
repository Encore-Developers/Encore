#pragma once
#include "song/song.h"
#include "users/player.h"
#include "util/Input.h"
#include "util/binary.h"

#include <deque>

// Version is formatted as YY_MM_DD_RR, where:
// - YY: Current year (2 digits, 4 digits impedes on 32-bit integer limit)
// - MM: Current month
// - DD: Current day
// - RR: Number of times the cache was revised that day, starting from 1
#define REPLAY_VERSION 26062902
#define REPLAY_HEADER 0x52434E45 // "ENCR"

namespace Encore::RhythmEngine {
    class Replay {
    public:
        struct ReplayParticipant {
            std::string name;
            int instrument;
            int difficulty;
            /// Overshell slot this player was in at time of recording. This is what the
            /// slot parameter on events is set to.
            int activeSlot;
            ControllerBindingType bindingType;

            // In case we want to create fake players from
            float noteSpeed;
            float trackLength;

            void Write(encore::bin_ofstream_le& stream);
            void Load(encore::bin_ifstream_le& stream);

            ReplayParticipant();
            ReplayParticipant(Player& player);
        };

        std::deque<ControllerEvent> inputs;
        std::vector<ReplayParticipant> participants;
        SongHash song;
        bool loaded = false;

        void Save(encore::bin_ofstream_le& stream);
        void Load(encore::bin_ifstream_le& stream);
    };

    class ReplayPlayer {
    public:
        Replay* replay;
        std::deque<ControllerEvent>::iterator nextInput;
        double lastUpdateTime;
        int slotFilter = -1;
        bool lastEventFetched = false;

        ReplayPlayer(Replay& replay);

        bool EventMatchesFilter(ControllerEvent& event);
        void SkipNonFilter();
        void Advance(double time);
        bool HasNextInput();
        ControllerEvent* GetNextInput();
    };
}

