#include "Replay.h"
namespace Encore::RhythmEngine {

    void Replay::ReplayParticipant::Write(encore::bin_ofstream_le &stream) {
        stream << name;
        stream << instrument;
        stream << difficulty;
        stream << activeSlot;
        stream << bindingType;
        stream << noteSpeed;
        stream << trackLength;
    }
    void Replay::ReplayParticipant::Load(encore::bin_ifstream_le &stream) {
        stream >> name;
        stream >> instrument;
        stream >> difficulty;
        stream >> activeSlot;
        uint8_t bindType;
        stream >> bindType;
        bindingType = (ControllerBindingType)bindType;
        stream >> noteSpeed;
        stream >> trackLength;
    }
    Replay::ReplayParticipant::ReplayParticipant()
        : instrument(0), difficulty(0), activeSlot(0), bindingType(), noteSpeed(0),
          trackLength(0) {}
    Replay::ReplayParticipant::ReplayParticipant(Player &player)
        : name(player.Name), instrument(player.Instrument), difficulty(player.Difficulty),
          activeSlot(player.ActiveSlot), bindingType(player.bindingType), noteSpeed(player.NoteSpeed),
          trackLength(player.HighwayLength) {}
    ReplayPlayer::ReplayPlayer(Replay &replay)
        : replay(&replay), lastUpdateTime(0) {
        nextInput = replay.inputs.begin();
    }
    bool ReplayPlayer::EventMatchesFilter(ControllerEvent &event) {
        if (slotFilter == -1) return true;
        return event.slot == slotFilter;
    }
    void ReplayPlayer::SkipNonFilter() {
        while (HasNextInput() && !EventMatchesFilter(*nextInput)) {
            nextInput++;
        }
    }

    void ReplayPlayer::Advance(double time) {
        lastUpdateTime = time;
    }

    bool ReplayPlayer::HasNextInput() {
        if (nextInput == replay->inputs.end()) return false;
        return nextInput->timestamp <= lastUpdateTime;
    }

    ControllerEvent *ReplayPlayer::GetNextInput() {
        if (lastEventFetched) {
            return &*nextInput;
        }
        SkipNonFilter();
        auto ret = &*nextInput;
        ++nextInput;
        if (nextInput == replay->inputs.end()) lastEventFetched = true;
        return ret;
    }

    void Replay::Save(encore::bin_ofstream_le& stream) {
        stream << (uint32_t)REPLAY_HEADER;
        stream << (uint32_t)REPLAY_VERSION;
        stream << song;

        stream << (u_int64_t)participants.size();
        for (auto& participant : participants) {
            participant.Write(stream);
        }

        stream << (u_int64_t)inputs.size();
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

        u_int64_t participantCount;
        stream >> participantCount;
        for (uint32_t i = 0; i < participantCount; i++) {
            auto part = &participants.emplace_back();
            part->Load(stream);
        }

        u_int64_t inputCount;
        stream >> inputCount;

        for (u_int64_t i = 0; i < inputCount; i++) {
            ControllerEvent newInput;
            LOAD_VALUE_TYPE(int8_t, newInput.channel)
            LOAD_VALUE_TYPE(int8_t, newInput.action)
            stream >> newInput.axis;
            int8_t slot;
            stream >> slot;
            newInput.slot = slot;
            stream >> newInput.timestamp;
            inputs.push_back(newInput);
        }

        std::ranges::sort(inputs, [](const ControllerEvent& a, const ControllerEvent& b){return a.timestamp < b.timestamp;});
        loaded = true;
    }
}

