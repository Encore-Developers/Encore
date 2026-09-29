#include "Replay.h"

#define LOAD_VALUE_TYPE(type, target) {type temp; stream >> temp; *(type*)(&target) = temp;}

namespace Encore::RhythmEngine {

    void ReplayParticipant::Write(encore::bin_ofstream_le &stream) {
        stream << name;
        stream << instrument;
        stream << difficulty;
        stream << activeSlot;
        stream << bindingType;
        stream << noteSpeed;
        stream << trackLength;

        stream << (uint64_t)inputs.size();
        for (auto& input : inputs) {
            stream << (int8_t)input.channel;
            stream << (int8_t)input.action;
            stream << input.axis;
            stream << input.timestamp;
        }
    }
    void ReplayParticipant::Load(encore::bin_ifstream_le &stream) {
        stream >> name;
        stream >> instrument;
        stream >> difficulty;
        stream >> activeSlot;
        uint8_t bindType;
        stream >> bindType;
        bindingType = (ControllerBindingType)bindType;
        stream >> noteSpeed;
        stream >> trackLength;

        uint64_t inputCount;
        stream >> inputCount;

        for (uint64_t i = 0; i < inputCount; i++) {
            ControllerEvent newInput;
            LOAD_VALUE_TYPE(int8_t, newInput.channel)
            assert(newInput.channel < InputChannel::CHANNEL_MAX);
            LOAD_VALUE_TYPE(int8_t, newInput.action)
            assert(newInput.action <= Action::REPEAT);
            stream >> newInput.axis;
            stream >> newInput.timestamp;

            inputs.push_back(newInput);
        }

        std::ranges::sort(inputs, [](const ControllerEvent& a, const ControllerEvent& b){return a.timestamp < b.timestamp;});
    }
    ReplayParticipant::ReplayParticipant()
        : instrument(0), difficulty(0), activeSlot(0), bindingType(), noteSpeed(0),
          trackLength(0) {}
    ReplayParticipant::ReplayParticipant(Player &player)
        : name(player.Name), instrument(player.Instrument), difficulty(player.Difficulty),
          activeSlot(player.ActiveSlot), bindingType(player.bindingType), noteSpeed(player.NoteSpeed),
          trackLength(player.HighwayLength) {}
    ReplayPlayer::ReplayPlayer(ReplayParticipant &replayParticipant)
        : replayParticipant(&replayParticipant), lastUpdateTime(0) {
        nextInput = replayParticipant.inputs.begin();
    }

    void ReplayPlayer::Advance(double time) {
        lastUpdateTime = time;
    }

    bool ReplayPlayer::HasNextInput() {
        if (nextInput == replayParticipant->inputs.end()) return false;
        return nextInput->timestamp <= lastUpdateTime;
    }

    ControllerEvent *ReplayPlayer::GetNextInput() {
        if (lastEventFetched) {
            return &*nextInput;
        }
        auto ret = &*nextInput;
        ++nextInput;
        if (nextInput == replayParticipant->inputs.end()) lastEventFetched = true;
        return ret;
    }

    void Replay::Save(encore::bin_ofstream_le& stream) {
        stream << (uint32_t)REPLAY_HEADER;
        stream << (uint32_t)REPLAY_VERSION;
        stream << song;

        stream << (uint64_t)participants.size();
        for (auto& participant : participants) {
            participant.Write(stream);
        }
    }

    void Replay::Load(encore::bin_ifstream_le& stream) {
        uint32_t header;
        uint32_t version;
        stream >> header;
        if (header != REPLAY_HEADER) {
            Log::Error("Invalid replay file (header not found)");
            return;
        }

        stream >> version;
        if (version != REPLAY_VERSION) {
            Log::Error("Replay file version is {:x} but current version is {:x}", version, REPLAY_VERSION);
            return;
        }
        stream >> song;

        uint64_t participantCount;
        stream >> participantCount;
        for (uint32_t i = 0; i < participantCount; i++) {
            auto part = &participants.emplace_back();
            part->Load(stream);
        }

        loaded = true;
    }
}

