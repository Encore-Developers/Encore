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
// - RR: Number of times the replay format was revised that day, starting from 1
#define REPLAY_VERSION 26092801
#define REPLAY_HEADER 0x52434E45 // "ENCR"

namespace Encore::RhythmEngine {
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

        std::deque<ControllerEvent> inputs;

        void Write(encore::bin_ofstream_le& stream);
        void Load(encore::bin_ifstream_le& stream);

        ReplayParticipant();
        ReplayParticipant(Player& player);
    };
    class Replay {
    public:


        std::vector<ReplayParticipant> participants;
        SongHash song;
        bool loaded = false;

        void Save(encore::bin_ofstream_le& stream);
        void Load(encore::bin_ifstream_le& stream);
    };

    class ReplayPlayer {
    public:
        ReplayParticipant* replayParticipant;
        std::deque<ControllerEvent>::iterator nextInput;
        double lastUpdateTime;

        bool lastEventFetched = false;

        ReplayPlayer(ReplayParticipant& replay);

        void Advance(double time);
        bool HasNextInput();
        ControllerEvent* GetNextInput();
    };
}

