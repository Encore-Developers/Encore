#include "SimpleMenu.h"
#include "raylib.h"
using namespace SimpleMenu;
using namespace Encore;

Option::Option(Instance* instance, const std::string &text) : instance(instance), text(text) {}
bool Option::Input(ControllerEvent event) {
    return false;
}
void Option::Draw(TextDisplay display, size_t index) {
    auto bgColor = instance->bgColor;
    if (index >= instance->rangeStart && index <= instance->rangeEnd) {
        bgColor = {80, 0, 80, 255};
    }
    if (instance->selectedIndex == index) {
        auto swap = display.color;
        display.color = bgColor;
        bgColor = swap;
    }
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
        SetRangeEnd(selectedIndex);
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
        break;
    }
}
void Instance::Draw() {
    int itemHeight = ItemHeight();

    if (autoScroll) {
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

    float cursor = -scroll;
    BeginScissorMode(displayParams.pos.x, displayParams.pos.y, displayParams.width, displayParams.height);
    for (size_t i = 0; i < options.size(); ++i) {
        auto& option = options[i];
        TextDisplay thisDisplay = displayParams;
        thisDisplay.pos.y = displayParams.pos.y + cursor;
        thisDisplay.height = itemHeight;
        option->Draw(thisDisplay, i);
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
void Instance::StartRangeSelect() {
    selectingRange = true;
    rangeStart = selectedIndex;
    rangeEnd = selectedIndex;
}
void Instance::EndRangeSelect() {
    selectingRange = false;
}