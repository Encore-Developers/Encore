//
// Created by marie on 18/09/2024.
//

#include "enclog.h"
// #include "raylib.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <cstring>
#include <vector>
#define FMT_UNICODE 0
#include "spdlog/spdlog.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

static bool active = false;

bool Encore::Log::InitializeLogger(std::string prefsPath) {
    try {
        std::vector<spdlog::sink_ptr> sinks;
        auto rotatingSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(prefsPath + "/log", 1048576 * 1024, 3);
        auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sinks.push_back(rotatingSink);
        sinks.push_back(stdoutSink);
        auto encoreLogger = std::make_shared<spdlog::logger>("Encore", begin(sinks), end(sinks));
        auto raylibLogger = std::make_shared<spdlog::logger>("raylib", begin(sinks), end(sinks));
        spdlog::register_logger(raylibLogger);
        spdlog::initialize_logger(encoreLogger);
        spdlog::set_default_logger(encoreLogger);
        spdlog::set_level(spdlog::level::trace);
        encoreLogger->info("Logger initialized");
        active = true;
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Failed to start logging service: " << e.what() << std::endl;
        return false;
    }
}

void Encore::Log::Trace(const std::string& msg) {
    spdlog::trace(msg);
}
void Encore::Log::Debug(const std::string& msg) {
    spdlog::debug(msg);
}
void Encore::Log::Info(const std::string& msg) {
    spdlog::info(msg);
}

void Encore::Log::Warn(const std::string& msg) {
    spdlog::warn(msg);
}
void Encore::Log::Error(const std::string& msg) {
    spdlog::error(msg);
}

void Encore::Log::Exit() {
    active = false;
    spdlog::shutdown();
}

// stolen from Raylib, dont want include issues here
typedef enum {
    LOG_ALL = 0,        // Display all logs
    LOG_TRACE,          // Trace logging, intended for internal use only
    LOG_DEBUG,          // Debug logging, used for internal debugging, it should be disabled on release builds
    LOG_INFO,           // Info logging, used for program execution info
    LOG_WARNING,        // Warning logging, used on recoverable failures
    LOG_ERROR,          // Error logging, used on unrecoverable failures
    LOG_FATAL,          // Fatal logging, used to abort program: exit(EXIT_FAILURE)
    LOG_NONE            // Disable logging
} TraceLogLevel;

// I want raylib to DIE.
void Encore::RaylibLogWrapper(int msgType, const char *text, va_list args) {
    if (!active) {
        // raylib SUCKS and is not important ANYWAYS
        return;
    }
    auto logger = spdlog::get("raylib");

    std::stringstream outputString;
    char buffer[256] = { 0 };
    char outputbuf[256] = { 0 };
    unsigned int textSize = (unsigned int)strlen(text);
    memcpy(buffer + strlen(buffer), text, (textSize < (256 - 12))? textSize : (256 - 12));
    vsprintf(outputbuf, buffer, args);
    outputString << outputbuf;
    switch (msgType)
    {
    case LOG_TRACE:
        logger->trace(outputString.str());
        break;
    case LOG_DEBUG:
        logger->debug(outputString.str());
        break;
    case LOG_INFO:
        logger->info(outputString.str());
        break;
    case LOG_WARNING:
        logger->warn(outputString.str());
        break;
    case LOG_FATAL:
    case LOG_ERROR:
        logger->error(outputString.str());
        break;
    default: break;
    }
}
