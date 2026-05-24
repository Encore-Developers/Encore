//
// Created by Student on 5/17/2026.
// Hopefully this is being done correctly.
//

#ifndef VENUEHANDLER_H
#define VENUEHANDLER_H
#include "raylib.h"

#include <vector>

namespace Encore {
    enum class VenueEventType {
        LIGHTSANIMATION_STATIC,
        LIGHTSANIMATION_ONBEAT,
        LIGHTSANIMATION_ROAMING,
        LIGHTSANIMATION_FLASH,
        LIGHTSANIMATION_BLACKOUT

    };
    class VenueEvent {
    public:
        float musicPosition = 0;
        VenueEventType eventType = VenueEventType::LIGHTSANIMATION_BLACKOUT;
    };

    class VenueHandler {
        public:

        int beatCount = 0;
        float previousDeltaBeat = 0.0f;

        float StageLightsAnimationTime = 0.0f;

        std::vector<VenueEvent> events;
        VenueEvent currentVenueEvent = VenueEvent();

        float StageLightsAnimationRandom = 0.0f;
        float StageLightsAnimationStatic = 0.0f;
        float StageLightsAnimationBeatSync = 0.0f;

        float StageLightGlobalRotation = 0.0f;

        float StageLightsBrightness = 0.0f;

        static void DrawVenueBackground();
        static void UpdateVenue();

        static void GenerateVenueEvents();
    };
}

extern Encore::VenueHandler TheVenueHandler;
#endif
