//
// Created by jayde on 6/5/2025.
//

#ifndef ENCORE_LOG_H
#define ENCORE_LOG_H

#include "raylib.h"

namespace Encore {
    inline void EncoreLog(int logLevel, const char* format, ...) {
        va_list args;
        va_start(args, format);
        TraceLog(logLevel, format, args);
        va_end(args);
    }
}

#endif