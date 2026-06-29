#include "Replay.h"
namespace Encore::RhythmEngine {

    Replay::ReplayPlayer::ReplayPlayer(Replay &replay)
        : replay(&replay), lastUpdateTime(0) {
        nextInput = replay.inputs.begin();
    }

    void Replay::ReplayPlayer::Advance(double time) {
        lastUpdateTime = time;
    }

    bool Replay::ReplayPlayer::HasNextInput() {
        if (nextInput == replay->inputs.end()) return false;
        return nextInput->timestamp <= lastUpdateTime;
    }

    ControllerEvent *Replay::ReplayPlayer::GetNextInput() {
        return &*nextInput++;
    }

    void Replay::Save(encore::bin_ofstream_le& stream) {
        stream << song;

        stream << (uint64_t)inputs.size();
        for (auto& input : inputs) {
            stream << (int8_t)input.channel;
            stream << (int8_t)input.action;
            stream << (int8_t)input.axis;
            // Yes, this is smaller than SDL_JoystickID, slots in replays represent player numbers
            // so even 8 bits is overkill
            stream << (int8_t)input.slot;

            stream << input.timestamp;
        }
    }

#define LOAD_VALUE_TYPE(type, target) {type temp; stream >> temp; *(type*)(&target) = temp;}

    void Replay::Load(encore::bin_ifstream_le& stream) {
        stream >> song;

        uint64_t inputCount;
        stream >> inputCount;

        for (uint64_t i = 0; i < inputCount; i++) {
            ControllerEvent newInput;
            LOAD_VALUE_TYPE(int8_t, newInput.channel)
            LOAD_VALUE_TYPE(int8_t, newInput.action)
            stream >> newInput.axis;
            stream >> newInput.slot;
            stream >> newInput.timestamp;
            inputs.push_back(newInput);
        }
    }
}

