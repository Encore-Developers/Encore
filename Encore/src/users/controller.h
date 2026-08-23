#pragma once
#include "SDL3/SDL_joystick.h"
#include "SDL3/SDL_scancode.h"

#include <cstdint>
#include <nlohmann/json.hpp>

namespace Encore {
    enum class InputChannel : int8_t {
        LANE_1 = 0,
        LANE_2,
        LANE_3,
        LANE_4,
        LANE_5,
        LANE_6,
        LANE_7,
        LANE_8,
        STRUM_UP,
        STRUM_DOWN,
        OVERDRIVE,
        WHAMMY,
        CHANNEL_MAX, // Input channels less than this one are ignored for replays
        PAUSE,
        UI_ACCEPT,
        UI_CANCEL,
        UI_VIEW,
        UI_OPTIONS,
        UI_MODIFIER,
        UI_MENU,
        UI_UP,
        UI_DOWN,
        UI_LEFT,
        UI_RIGHT,
        DISCONNECT,
        INVALID = -1
    };

    enum PhysicalDeviceType : uint8_t {
        PAD,
        GUITAR,
        DRUMS,
        DRUMS_5L
    };

    // When mapping an instrument controller, we instead map by the physical input on the
    // device. This makes it easier to show instrument-specific input prompts and relieves
    // the user from having to map things like yellow/blue drum pad navigation
    // They get converted to InputChannel mappings at configure time.
    enum class PhysicalDeviceInput : uint8_t {
        LANE_GREEN,
        LANE_RED,
        LANE_YELLOW,
        LANE_BLUE,
        LANE_ORANGE,
        DPAD_UP,
        DPAD_DOWN,
        DPAD_LEFT,
        DPAD_RIGHT,
        START,
        SELECT,
        WHAMMY,
        KICK,
        // For RB Pro Drums and not GH's cymbals
        CYMBAL_YELLOW,
        CYMBAL_BLUE,
        CYMBAL_GREEN
    };

    enum class InputSource : uint8_t {
        INVALID,
        SDL_JOYSTICK,
        SDL_GAMEPAD,
        KEYBOARD,
        MIDI,
        REPLAY
    };

    struct ControllerIdentity {
        InputSource source = InputSource::INVALID;
        union {
            SDL_JoystickID joystickID;
            uint8_t replaySlot;
        };
        bool operator==(const ControllerIdentity & controller) const {
            if (source == InputSource::INVALID || controller.source == InputSource::INVALID) return false;
            if (source == InputSource::KEYBOARD && controller.source == InputSource::KEYBOARD) return true;
            return source == controller.source && joystickID == controller.joystickID;
        }
        bool IsValid() const {
            return source != InputSource::INVALID;
        }
        std::string GetName() const {
            switch (source) {
            case InputSource::INVALID:
                return "Invalid";
            case InputSource::SDL_JOYSTICK:
            case InputSource::SDL_GAMEPAD:
                return SDL_GetJoystickNameForID(joystickID);
            case InputSource::KEYBOARD:
                return "Keyboard";
            case InputSource::MIDI:
                return "A MIDI Device (TODO)";
            case InputSource::REPLAY:
                return "Replay";
            }
            return "Unknown";
        }
    };

    enum class InputMatchType : uint8_t {
        BUTTON,
        AXIS
    };

    struct InputMatch {
        InputSource source = InputSource::SDL_JOYSTICK;
        InputMatchType type = InputMatchType::BUTTON;
        union {
            uint8_t buttonId;
            uint8_t axisId;
            SDL_Scancode scancode;
            uint32_t midiNote = 0; // Temporary, don't know how we'll be consuming midi events yet
        };
        // Axis calibration data
        int16_t min = -32768;
        int16_t max = 32767;
        // For analog -> digital mappings
        float deadzone = 0.1f;

        float mapAnalog(int16_t rawAxis) const {
            return ((float)rawAxis - (float)min)/((float)max - (float)min);
        }
    };

    class ControllerProfile {
        void MapPhysical(PhysicalDeviceInput, InputChannel);
    public:
        std::string name;
        PhysicalDeviceType controllerType;
        std::unordered_map<InputChannel, std::vector<InputMatch>> channelMappings;
        std::unordered_map<PhysicalDeviceInput, std::vector<InputMatch>> physicalMappings;

        // Generates mappings from physicalMappings if we're an instrument controller
        void Update();
    };

    class ControllerState {
    public:
        ControllerIdentity controller;
        std::shared_ptr<ControllerProfile> controllerProfile;
    };

    NLOHMANN_JSON_SERIALIZE_ENUM( PhysicalDeviceType, {
        {GUITAR, "guitar"},
        {PAD, "pad"},
        {DRUMS, "drums"},
        {DRUMS_5L, "drums_5l"}
    })

    NLOHMANN_JSON_SERIALIZE_ENUM( InputSource, {
        {InputSource::SDL_JOYSTICK, "sdl_joystick"},
        {InputSource::SDL_GAMEPAD, "sdl_gamepad"},
        {InputSource::KEYBOARD, "keyboard"},
        {InputSource::MIDI, "midi"}
    })

    NLOHMANN_JSON_SERIALIZE_ENUM( InputMatchType, {
        {InputMatchType::BUTTON, "button"},
        {InputMatchType::AXIS, "axis"}
    })

    NLOHMANN_JSON_SERIALIZE_ENUM( InputChannel, {
        {InputChannel::LANE_1, "LANE_1"},
        {InputChannel::LANE_2, "LANE_2"},
        {InputChannel::LANE_3, "LANE_3"},
        {InputChannel::LANE_4, "LANE_4"},
        {InputChannel::LANE_5, "LANE_5"},
        {InputChannel::LANE_6, "LANE_6"},
        {InputChannel::LANE_7, "LANE_7"},
        {InputChannel::LANE_8, "LANE_8"},
        {InputChannel::STRUM_UP, "STRUM_UP"},
        {InputChannel::STRUM_DOWN, "STRUM_DOWN"},
        {InputChannel::OVERDRIVE, "OVERDRIVE"},
        {InputChannel::WHAMMY, "WHAMMY"},
        {InputChannel::PAUSE, "PAUSE"},
        {InputChannel::UI_ACCEPT, "UI_ACCEPT"},
        {InputChannel::UI_CANCEL, "UI_CANCEL"},
        {InputChannel::UI_VIEW, "UI_VIEW"},
        {InputChannel::UI_OPTIONS, "UI_OPTIONS"},
        {InputChannel::UI_MENU, "UI_MENU"},
        {InputChannel::UI_UP, "UI_UP"},
        {InputChannel::UI_DOWN, "UI_DOWN"},
        {InputChannel::UI_LEFT, "UI_LEFT"},
        {InputChannel::UI_RIGHT, "UI_RIGHT"}
    })

    NLOHMANN_JSON_SERIALIZE_ENUM( PhysicalDeviceInput, {
        {PhysicalDeviceInput::LANE_GREEN, "GREEN"},
        {PhysicalDeviceInput::LANE_RED, "RED"},
        {PhysicalDeviceInput::LANE_YELLOW, "YELLOW"},
        {PhysicalDeviceInput::LANE_BLUE, "BLUE"},
        {PhysicalDeviceInput::LANE_ORANGE, "ORANGE"},
        {PhysicalDeviceInput::DPAD_UP, "DPAD_UP"},
        {PhysicalDeviceInput::DPAD_DOWN, "DPAD_DOWN"},
        {PhysicalDeviceInput::DPAD_LEFT, "DPAD_LEFT"},
        {PhysicalDeviceInput::DPAD_RIGHT, "DPAD_RIGHT"},
        {PhysicalDeviceInput::START, "START"},
        {PhysicalDeviceInput::SELECT, "SELECT"},
        {PhysicalDeviceInput::WHAMMY, "WHAMMY"},
        {PhysicalDeviceInput::KICK, "KICK"},
        {PhysicalDeviceInput::CYMBAL_YELLOW, "CYMBAL_YELLOW"},
        {PhysicalDeviceInput::CYMBAL_BLUE, "CYMBAL_BLUE"},
        {PhysicalDeviceInput::CYMBAL_GREEN, "CYMBAL_GREEN"}
    })

    void to_json(nlohmann::json& j, const InputMatch& m);

    void from_json(const nlohmann::json& j, InputMatch& m);

    void to_json(nlohmann::json& j, const ControllerProfile& p);

    void from_json(const nlohmann::json& j, ControllerProfile& p);
}

