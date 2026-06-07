#pragma once
#include "menus/util/TextDisplay.h"
#include "util/Input.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>
namespace SimpleMenu {
    class Instance;

    enum MouseState {
        NONE,
        HOVERED,
        ACTIVE
    };

    class Option {
        bool mouseClicked = false;
        friend class Instance;
    public:
        virtual ~Option() = default;
        Instance * instance;
        std::string text;
        bool isBackOption = false;

        Option(Instance* instance, const std::string& text);

        virtual bool Input(Encore::ControllerEvent event);
        virtual void Draw(Encore::TextDisplay display, size_t index, MouseState mouseState);
        void SetBackOption() {
            isBackOption = true;
        }
    };

    class FuncOption : public Option {
    public:
        std::function<void()> func;

        FuncOption(Instance*, const std::string& text, std::function<void()> func);

        bool Input(Encore::ControllerEvent event) override;
    };

    class Instance {
    public:
        std::vector<std::shared_ptr<Option>> options;
        Encore::TextDisplay displayParams;
        Color bgColor = BLACK;
        float itemSpacing = 0;
        float scroll = 0;
        int scrollMargin = 4;
        bool autoScroll = true;
        size_t selectedIndex = 0;
        bool inputWrapping = true;
        bool mouseInput = true;
        bool lastInteractionWasMouse = false;

        int rangeStart = -1;
        int rangeEnd = -1;
        bool selectingRange = false;

        void Select(int amount);
        void Input(Encore::ControllerEvent event);
        void Draw();
        float ItemHeight();

        void SetRangeStart(int start);
        void SetRangeEnd(int end);
        void ExpandRange(int target);
        void StartRangeSelect();
        void EndRangeSelect();

        template<typename T, typename... Args>
        T* CreateOption(Args&&... __args) {
            auto option = std::make_shared<T>(this, std::forward<Args>(__args)...);
            options.push_back(option);
            return option.get();
        }
    };
}

