#include "SimpleMenu.h"
#include "raylib.h"
using namespace SimpleMenu;
using namespace Encore;

Option::Option(Instance* instance, const std::string &text) : instance(instance), text(text) {}
bool Option::Input(ControllerEvent event) {
    return false;
}
void Option::Draw(TextDisplay display, size_t index, MouseState mouseState) {
    auto bgColor = instance->bgColor;
    if (index >= instance->rangeStart && index <= instance->rangeEnd) {
        bgColor = {80, 0, 80, 255};
    }
    if (mouseState == HOVERED) {
        bgColor = ColorLerp(bgColor, display.color, 0.1);
        if (mouseClicked) {
            ControllerEvent fakeEvent;
            fakeEvent.channel = InputChannel::LANE_1;
            fakeEvent.action = Action::PRESS;
            fakeEvent.slot = -1;
            fakeEvent.timestamp = 0;
            fakeEvent.axis = 0;

            instance->Select((int)index - instance->selectedIndex);
            Input(fakeEvent);
        }
    }
    if ((!instance->lastInteractionWasMouse && instance->selectedIndex == index) || mouseState == ACTIVE) {
        auto swap = display.color;
        display.color = bgColor;
        bgColor = swap;
    }
    mouseClicked = mouseState == ACTIVE;
    DrawRectangle(display.pos.x, display.pos.y, display.width, display.height, bgColor);
    display.DrawText(text);
}
FuncOption::FuncOption(Instance* instance, const std::string &text, std::function<void()> func)
    : Option(instance, text), func(func) {}
bool FuncOption::Input(ControllerEvent event) {
    if (event.IsAccept()) {
        func();
        return true;
    }
    return false;
}

void Instance::Select(int amount) {
    if (options.empty()) {
        return;
    }
    int selectedIndexSigned = selectedIndex;
    selectedIndexSigned += amount;
    if (inputWrapping) {
        if (selectedIndexSigned < 0) {
            selectedIndexSigned = options.size() + selectedIndexSigned;
        }
        if (selectedIndexSigned >= options.size()) {
            selectedIndexSigned = selectedIndexSigned - options.size();
        }
    } else {
        if (selectedIndexSigned < 0) {
            selectedIndexSigned = 0;
        }
        if (selectedIndexSigned >= options.size()) {
            selectedIndexSigned = options.size() - 1;
        }
    }

    selectedIndex = selectedIndexSigned;
    if (selectingRange) {
        ExpandRange(selectedIndex);
    }
}
void Instance::Input(ControllerEvent event) {
    if (options.empty()) {
        return;
    }
    if (options[selectedIndex]->Input(event)) {
        return;
    }
    if (event.action == Action::RELEASE) {
        return;
    }
    switch (event.channel) {
    case InputChannel::STRUM_DOWN:
        Select(1);
        break;
    case InputChannel::STRUM_UP:
        Select(-1);
        break;
    case InputChannel::INPUT_LEFT:
        Select(-4);
        break;
    case InputChannel::INPUT_RIGHT:
        Select(4);
        break;
    case InputChannel::LANE_2:
        for (auto& option : options) {
            if (option->isBackOption) {
                event.channel = InputChannel::LANE_1;
                option->Input(event);
            }
        }
    default:
        return;
    }
    lastInteractionWasMouse = false;
}
void Instance::Draw() {
    int itemHeight = ItemHeight();

    if (autoScroll && !lastInteractionWasMouse) {
        float selectedPos = selectedIndex * itemHeight;
        float marginSize = scrollMargin * itemHeight;

        float safeAreaStart = scroll + marginSize;
        float safeAreaEnd = scroll + displayParams.height - marginSize;

        if (selectedPos > safeAreaEnd) {
            scroll = selectedPos + marginSize - displayParams.height;
        }
        if (selectedPos < safeAreaStart) {
            scroll = selectedPos - marginSize;
        }

        if (scroll > options.size()*itemHeight - displayParams.height) {
            scroll = options.size()*itemHeight - displayParams.height;
        }
        if (scroll < 0) {
            scroll = 0;
        }

    }

    bool isInstanceHovered = displayParams.CollidesPoint(GetMousePosition());
    if (isInstanceHovered) {
        if (GetMouseWheelMove() != 0) {
            lastInteractionWasMouse = true;
        }
        scroll -= GetMouseWheelMove() * itemHeight;

        if (scroll > options.size()*itemHeight - displayParams.height) {
            scroll = options.size()*itemHeight - displayParams.height;
        }
        if (scroll < 0) {
            scroll = 0;
        }
    }

    float cursor = -scroll;
    BeginScissorMode(displayParams.pos.x, displayParams.pos.y, displayParams.width, displayParams.height);
    for (size_t i = 0; i < options.size(); ++i) {
        if (cursor > -itemHeight && cursor <= options.size() * itemHeight) {
            auto option = options[i];
            TextDisplay thisDisplay = displayParams;
            thisDisplay.pos.y = displayParams.pos.y + cursor;
            thisDisplay.height = itemHeight;
            MouseState mouseState = NONE;
            if (isInstanceHovered && thisDisplay.CollidesPoint(GetMousePosition())) {
                if (lastInteractionWasMouse) {
                    if (selectedIndex != i) {
                        Select(i - selectedIndex);
                        // Known issue: range selection will act odd because of the mouse hover
                        // Hovering items will expand the range but never shrink it
                    }
                }
                if (IsMouseButtonDown(0)) {
                    mouseState = ACTIVE;
                    lastInteractionWasMouse = true;
                } else {
                    mouseState = HOVERED;
                }
            }
            option->Draw(thisDisplay, i, mouseState);
        }
        cursor += itemHeight + itemSpacing;
    }
    EndScissorMode();
}
float Instance::ItemHeight() {
    return displayParams.fontSize + displayParams.padding.y*2;
}
void Instance::SetRangeStart(int start) {
    if (rangeEnd == -1) {
        rangeEnd = start;
    }
    rangeStart = start;
    if (rangeStart > rangeEnd) {
        rangeEnd = rangeStart;
    }
}
void Instance::SetRangeEnd(int end) {
    if (rangeStart == -1) {
        rangeStart = end;
    }
    rangeEnd = end;
    if (rangeEnd < rangeStart) {
        rangeStart = rangeEnd;
    }
}
void Instance::ExpandRange(int target) {
    if (rangeStart == -1 || rangeEnd == -1) {
        rangeStart = target;
        rangeEnd = target;
    }
    if (target < rangeStart) {
        rangeStart = target;
    }
    if (target > rangeEnd) {
        rangeEnd = target;
    }
}
void Instance::StartRangeSelect() {
    selectingRange = true;
    rangeStart = selectedIndex;
    rangeEnd = selectedIndex;
}
void Instance::EndRangeSelect() {
    selectingRange = false;
}