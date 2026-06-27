#pragma once

#include <format>
#include <string>
#include <vector>

namespace Encore {

    namespace Log {
        bool InitializeLogger(std::string prefsPath);


        void Trace(const std::string& msg);
        void Debug(const std::string& msg);
        void Info(const std::string& msg);
        void Warn(const std::string& msg);
        void Error(const std::string& msg);
        std::vector<std::string> GetRecentMessages();

        void Exit();

        template<typename... _Args>
        void Trace(const std::string& fmt, _Args&&... args) {
            Log::Trace(std::vformat(fmt, std::make_format_args(args ...)));
        };

        template<typename... _Args>
        void Debug(const std::string& fmt, _Args&&... args) {
            Log::Debug(std::vformat(fmt, std::make_format_args(args ...)));
        };

        template<typename... _Args>
        void Info(const std::string& fmt, _Args&&... args) {
            Log::Info(std::vformat(fmt, std::make_format_args(args ...)));
        };

        template<typename... _Args>
        void Warn(const std::string& fmt, _Args&&... args){
            Log::Warn(std::vformat(fmt, std::make_format_args(args ...)));
        };

        template<typename... _Args>
        void Error(const std::string& fmt, _Args&&... args) {
            Log::Error(std::vformat(fmt, std::make_format_args(args ...)));
        };

    }
    void RaylibLogWrapper(int msgType, const char *text, va_list args);
}

