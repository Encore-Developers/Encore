#include "controller.h"

using json = nlohmann::json;

void Encore::ControllerProfile::MapPhysical(PhysicalDeviceInput phys, InputChannel chan) {
    if (physicalMappings.contains(phys)) {
        channelMappings[chan] = physicalMappings[phys];
    }
}
void Encore::ControllerProfile::Update() {
    if (controllerType == PAD) {
        physicalMappings.clear();
    } else {
        channelMappings.clear();

        switch (controllerType) {
        case GUITAR:
            MapPhysical(PhysicalDeviceInput::LANE_GREEN, InputChannel::LANE_1);
            MapPhysical(PhysicalDeviceInput::LANE_RED, InputChannel::LANE_2);
            MapPhysical(PhysicalDeviceInput::LANE_YELLOW, InputChannel::LANE_3);
            MapPhysical(PhysicalDeviceInput::LANE_BLUE, InputChannel::LANE_4);
            MapPhysical(PhysicalDeviceInput::LANE_ORANGE, InputChannel::LANE_5);
            MapPhysical(PhysicalDeviceInput::DPAD_UP, InputChannel::STRUM_UP);
            MapPhysical(PhysicalDeviceInput::DPAD_DOWN, InputChannel::STRUM_DOWN);
            MapPhysical(PhysicalDeviceInput::SELECT, InputChannel::OVERDRIVE);
            MapPhysical(PhysicalDeviceInput::WHAMMY, InputChannel::WHAMMY);

            MapPhysical(PhysicalDeviceInput::LANE_GREEN, InputChannel::UI_ACCEPT);
            MapPhysical(PhysicalDeviceInput::LANE_RED, InputChannel::UI_CANCEL);
            MapPhysical(PhysicalDeviceInput::LANE_YELLOW, InputChannel::UI_VIEW);
            MapPhysical(PhysicalDeviceInput::LANE_BLUE, InputChannel::UI_OPTIONS);
            MapPhysical(PhysicalDeviceInput::LANE_ORANGE, InputChannel::UI_MODIFIER);

            MapPhysical(PhysicalDeviceInput::START, InputChannel::PAUSE);
            MapPhysical(PhysicalDeviceInput::SELECT, InputChannel::UI_MENU);

            MapPhysical(PhysicalDeviceInput::DPAD_UP, InputChannel::UI_UP);
            MapPhysical(PhysicalDeviceInput::DPAD_DOWN, InputChannel::UI_DOWN);
            MapPhysical(PhysicalDeviceInput::DPAD_LEFT, InputChannel::UI_LEFT);
            MapPhysical(PhysicalDeviceInput::DPAD_RIGHT, InputChannel::UI_RIGHT);
            break;
        case DRUMS:
            MapPhysical(PhysicalDeviceInput::KICK, InputChannel::LANE_1);
            MapPhysical(PhysicalDeviceInput::LANE_RED, InputChannel::LANE_2);
            MapPhysical(PhysicalDeviceInput::LANE_YELLOW, InputChannel::LANE_3);
            MapPhysical(PhysicalDeviceInput::LANE_BLUE, InputChannel::LANE_4);
            MapPhysical(PhysicalDeviceInput::LANE_GREEN, InputChannel::LANE_5);
            MapPhysical(PhysicalDeviceInput::CYMBAL_YELLOW, InputChannel::LANE_6);
            MapPhysical(PhysicalDeviceInput::CYMBAL_BLUE, InputChannel::LANE_7);
            MapPhysical(PhysicalDeviceInput::CYMBAL_GREEN, InputChannel::LANE_8);

            MapPhysical(PhysicalDeviceInput::LANE_GREEN, InputChannel::UI_ACCEPT);
            MapPhysical(PhysicalDeviceInput::LANE_RED, InputChannel::UI_CANCEL);
            MapPhysical(PhysicalDeviceInput::CYMBAL_YELLOW, InputChannel::UI_VIEW);
            MapPhysical(PhysicalDeviceInput::CYMBAL_BLUE, InputChannel::UI_OPTIONS);
            MapPhysical(PhysicalDeviceInput::KICK, InputChannel::UI_MODIFIER);

            MapPhysical(PhysicalDeviceInput::START, InputChannel::PAUSE);
            MapPhysical(PhysicalDeviceInput::SELECT, InputChannel::UI_MENU);

            MapPhysical(PhysicalDeviceInput::DPAD_UP, InputChannel::UI_UP);
            MapPhysical(PhysicalDeviceInput::DPAD_DOWN, InputChannel::UI_DOWN);
            MapPhysical(PhysicalDeviceInput::LANE_YELLOW, InputChannel::UI_UP);
            MapPhysical(PhysicalDeviceInput::LANE_BLUE, InputChannel::UI_DOWN);
            MapPhysical(PhysicalDeviceInput::DPAD_LEFT, InputChannel::UI_LEFT);
            MapPhysical(PhysicalDeviceInput::DPAD_RIGHT, InputChannel::UI_RIGHT);
            break;
        case DRUMS_5L:
            MapPhysical(PhysicalDeviceInput::KICK, InputChannel::LANE_1);
            MapPhysical(PhysicalDeviceInput::LANE_RED, InputChannel::LANE_2);
            MapPhysical(PhysicalDeviceInput::LANE_YELLOW, InputChannel::LANE_3);
            MapPhysical(PhysicalDeviceInput::LANE_BLUE, InputChannel::LANE_4);
            MapPhysical(PhysicalDeviceInput::LANE_ORANGE, InputChannel::LANE_5);
            MapPhysical(PhysicalDeviceInput::LANE_GREEN, InputChannel::LANE_6);

            MapPhysical(PhysicalDeviceInput::LANE_GREEN, InputChannel::UI_ACCEPT);
            MapPhysical(PhysicalDeviceInput::LANE_RED, InputChannel::UI_CANCEL);
            MapPhysical(PhysicalDeviceInput::LANE_YELLOW, InputChannel::UI_VIEW);
            MapPhysical(PhysicalDeviceInput::LANE_ORANGE, InputChannel::UI_OPTIONS);
            MapPhysical(PhysicalDeviceInput::KICK, InputChannel::UI_MODIFIER);

            MapPhysical(PhysicalDeviceInput::START, InputChannel::PAUSE);
            MapPhysical(PhysicalDeviceInput::SELECT, InputChannel::UI_MENU);

            MapPhysical(PhysicalDeviceInput::DPAD_UP, InputChannel::UI_UP);
            MapPhysical(PhysicalDeviceInput::DPAD_DOWN, InputChannel::UI_DOWN);
            //MapPhysical(PhysicalDeviceInput::YELLOW, InputChannel::UI_UP);
            //MapPhysical(PhysicalDeviceInput::BLUE, InputChannel::UI_DOWN);
            MapPhysical(PhysicalDeviceInput::DPAD_LEFT, InputChannel::UI_LEFT);
            MapPhysical(PhysicalDeviceInput::DPAD_RIGHT, InputChannel::UI_RIGHT);
            break;
        default:
            break; // should be unreachable
        }

    }
}
void Encore::to_json(json &j, const InputMatch &m) {
    j = json { { "source", m.source }, { "type", m.type } };
    switch (m.source) {
    case InputSource::SDL_JOYSTICK:
    case InputSource::SDL_GAMEPAD:
        if (m.type == InputMatchType::BUTTON) {
            j["buttonId"] = m.buttonId;
        } else {
            j["axisId"] = m.axisId;
        }
        break;
    case InputSource::KEYBOARD:
        j["scancode"] = m.scancode;
        break;
    case InputSource::MIDI:
        j["note"] = m.midiNote;
        break;
    }
    if (m.type == InputMatchType::AXIS) {
        j["min"] = m.min;
        j["max"] = m.max;
        j["deadzone"] = m.deadzone;
    }
}
void Encore::from_json(const json &j, InputMatch &m) {
    j.at("source").get_to(m.source);
    j.at("type").get_to(m.type);
    switch (m.source) {
    case InputSource::SDL_JOYSTICK:
    case InputSource::SDL_GAMEPAD:
        if (m.type == InputMatchType::BUTTON) {
            j.at("buttonId").get_to(m.buttonId);
        } else {
            j.at("axisId").get_to(m.axisId);
        }
        break;
    case InputSource::KEYBOARD:
        j.at("scancode").get_to(m.scancode);
        break;
    case InputSource::MIDI:
        j.at("note").get_to(m.midiNote);
        break;
    }
    if (m.type == InputMatchType::AXIS) {
        j.at("min").get_to(m.min);
        j.at("max").get_to(m.max);
        j.at("deadzone").get_to(m.deadzone);
    }
}

void Encore::to_json(json &j, const ControllerProfile &p) {
    j["name"] = p.name;
    if (p.controllerType == PAD) {
        j["channelMappings"] = p.channelMappings;
    } else {
        j["physicalMappings"] = p.physicalMappings;
    }
}
void Encore::from_json(const json &j, ControllerProfile &p) {
    j.at("name").get_to(p.name);
    if (p.controllerType == PAD) {
        j.at("channelMappings").get_to(p.channelMappings);
    } else {
        j.at("physicalMappings").get_to(p.physicalMappings);
    }
    p.Update();
}