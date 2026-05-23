//
// Created by maria on 23/05/2026.
//

#ifndef ENCORE_SETTINGRENDERER_H
#define ENCORE_SETTINGRENDERER_H
#include <string>
#include <vector>

namespace Encore {
    class SettingDoohickey {
    public:
        enum class settingType {
            INVALID,
            BOOL_SETTING,
            FLOAT_SETTING,
            INT_SETTING
        };

        struct settingObject {
            std::string name;
            void *value;
            int min;
            int max;
            int increment;
            virtual settingType GetType() const { return settingType::INVALID; }
            virtual ~settingObject() {value = nullptr;};
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
            std::string name;
            bool *value;
            int min;
            int max;
            int increment;
            virtual settingType GetType() const override { return settingType::BOOL_SETTING; }

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
            std::string name;
            float *value;
            int min;
            int max;
            float increment;
            settingType GetType() const override { return settingType::FLOAT_SETTING; }
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
            std::string name;
            int *value;
            int min;
            int max;
            int increment;
            settingType GetType() const override { return settingType::INT_SETTING; }
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
        std::vector<settingObject*> settingsArray;
        ~SettingDoohickey() {
            for (const auto* setting : settingsArray) {
                delete setting;
            }
            settingsArray.clear();
        }
        void Action(int selectedIndex, bool remove);

        void Add(settingObject* object) {
            settingsArray.emplace_back(object);
        };
        void Draw(int selectedIndex, float top = 0.15f);
    };
}

#endif //ENCORE_SETTINGRENDERER_H