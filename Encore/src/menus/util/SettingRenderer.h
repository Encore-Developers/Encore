//
// Created by maria on 23/05/2026.
//

#ifndef ENCORE_SETTINGRENDERER_H
#define ENCORE_SETTINGRENDERER_H
#include <functional>
#include <string>
#include <vector>
#include "raylib.h"

namespace Encore {
    class SettingDoohickey {
    public:
        enum class settingType {
            INVALID,
            BOOL_SETTING,
            FLOAT_SETTING,
            INT_SETTING,
            BUTTON_SETTING,
            SEPARATOR
        };

        struct settingObject {
            std::string name;
            int min;
            int max;
            int increment;
            virtual settingType GetType() const { return settingType::INVALID; }
            virtual ~settingObject() {};
            virtual void Draw(Rectangle pos, bool hovered, bool Clickable) {};
            virtual void Action(bool Invert) {};
            // settingObject(const std::string &_name,
            //                      int* _value,
            //                      int _min,
            //                      int _max,
            //                      int _increment)
            // {
            //     name = _name;
            //     value = _value;
            //     min = _min;
            //     max = _max;
            //     increment = _increment;
            // }
        };

        struct boolSettingObject : settingObject {
            bool *value;
            settingType GetType() const override { return settingType::BOOL_SETTING; }
            void Draw(Rectangle pos, bool hovered, bool Clickable) override;
            void Action(bool invert) override;
            ~boolSettingObject() override {value = nullptr;};
            boolSettingObject(const std::string &_name,
                              bool *_value,
                              int _min,
                              int _max,
                              int _increment) {
                name = _name;
                value = _value;
                min = _min;
                max = _max;
                increment = _increment;
            }
        };

        struct floatSettingObject : settingObject {
            float *value;
            float increment;
            settingType GetType() const override { return settingType::FLOAT_SETTING; }

            void Draw(Rectangle pos, bool hovered, bool Clickable) override;
            void Action(bool invert) override;
            ~floatSettingObject() override {value = nullptr;};
            floatSettingObject(const std::string &_name,
                               float *_value,
                               int _min,
                               int _max,
                               float _increment) {
                name = _name;
                value = _value;
                min = _min;
                max = _max;
                increment = _increment;
            }
        };

        struct intSettingObject : settingObject {
            int *value;
            settingType GetType() const override { return settingType::INT_SETTING; }

            void Draw(Rectangle pos, bool hovered, bool Clickable) override;
            void Action(bool invert) override;
            ~intSettingObject() override {value = nullptr;};
            intSettingObject(const std::string &_name,
                             int *_value,
                             int _min,
                             int _max,
                             int _increment) {
                name = _name;
                value = _value;
                min = _min;
                max = _max;
                increment = _increment;
            }
            ;
        };

        struct buttonSettingObject : settingObject {
            std::function<void()> *value;
            settingType GetType() const override { return settingType::BUTTON_SETTING; }

            void Draw(Rectangle pos, bool hovered, bool Clickable) override;
            void Action(bool invert) override;
            ~buttonSettingObject() override {value = nullptr;};
            buttonSettingObject(const std::string &_name,
                             std::function<void()> *_value) {
                name = _name;
                value = _value;
                min = 0;
                max = 0;
                increment = 0;
            };
        };

        struct separatorObject : settingObject {
            settingType GetType() const override { return settingType::SEPARATOR; }

            void Draw(Rectangle pos, bool hovered, bool Clickable) override;
            void Action(bool invert) override;
            ~separatorObject() override {};
            separatorObject(const std::string &_name) {
                name = _name;
                min = 0;
                max = 0;
                increment = 0;
            };
        };
        bool isOSOpen = false;
        std::vector<settingObject*> settingsArray;
        ~SettingDoohickey() {
            for (const auto* setting : settingsArray) {
                delete setting;
            }
            settingsArray.clear();
        }
        int selectedIndex = 0;
        void Action(bool remove);
        void IncrementSelected(bool up);

        void Add(settingObject* object) {
            settingsArray.emplace_back(object);
        };
        void Draw(float top = 0.15f);
    };
}

#endif //ENCORE_SETTINGRENDERER_H