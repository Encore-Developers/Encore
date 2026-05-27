#pragma once
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <format>
#include "tracy/Tracy.hpp"

#include <unordered_set>
#include <nlohmann/json.hpp>

namespace Encore {

    class LocalizedString {
        LocalizedString() {}
    public:
        const char* ref;
        std::string ownedData;

        LocalizedString(const std::string& in) {
            ref = in.c_str();
        }
        static LocalizedString FromStringPtr(const std::string* in) {
            LocalizedString output;
            output.ref = in->c_str();
            return output;
        }

        static LocalizedString MoveStr(std::string data) {
            LocalizedString output;
            output.ownedData = std::move(data);
            output.ref = nullptr;
            return output;
        }
        std::string toString() const {
            if (!ref) {
                return ownedData;
            } else {
                return ref;
            }
        }

        const char* toChar() const {
            if (!ref) {
                return ownedData.c_str();
            } else {
                return ref;
            }
        }

        operator std::string() const {
            if (!ref) {
                return ownedData;
            } else {
                return ref;
            }
        }

        operator const char*() const {
            if (!ref) {
                return ownedData.c_str();
            } else {
                return ref;
            }
        }
    };

    class LocaleLayer {
        void AddEntries(const std::string& name, nlohmann::basic_json<>& json);
    public:
        std::string name;
        std::unordered_map<std::string, std::string> entries;
        bool fallback;

        LocaleLayer(const std::string& name, bool fallback);

        const std::string* FetchValue(const std::string& token) const;
    };

    class Locale {
    public:
        static std::vector<LocaleLayer> layers;
        static std::unordered_set<std::string> unlocalizedTokens;

        static void Init();
        static void AddLayer(const std::string& name, bool fallback);
        static LocalizedString Localize(const std::string& token);

        static LocalizedString LocalizeFormat(const std::string& token, std::format_args args) {
            ZoneScoped
            std::string value = Localize(token);
            return LocalizedString::MoveStr(std::vformat((const std::string &)value, args));
        }
    };
}

#define LOCALIZE(token) Encore::Locale::Localize(token)
#define LOCALIZE_STR(token) std::string(Encore::Locale::Localize(token))
#define LOCALIZE_FMT(token, ...) Encore::Locale::LocalizeFormat(token, std::make_format_args(__VA_ARGS__))

// i keep typing it wrong   -marie
#define LOCALISE(token) LOCALIZE(token)
#define LOCALISE_STR(token) LOCALIZE_STR(token)
#define LOCALISE_FMT(token, ...) LOCALIZE_FMT(token, __VA_ARGS__)
