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
        LIGHTSANIMATION_ROAMING,
        LIGHTSANIMATION_FLASH,
        LIGHTSANIMATION_STROBE,
        LIGHTSANIMATION_BLACKOUT,
        LIGHTSANIMATION_FRENZY,

    };
    class VenueEvent {
    public:
        float musicPosition = 0;
        VenueEventType eventType = VenueEventType::LIGHTSANIMATION_BLACKOUT;
    };

    enum class VenueObjectType {
        VENUEOBJECT_LIGHTS,
        VENUEOBJECT_EMITTER,
    };

    class VenueObject {
        public:
        Vector3 Position;
        Vector3 Rotation;

        bool RotateOnX;
        bool RotateOnY;
        bool RotateOnZ;

        int AnimationSlot;
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
        float StageLightGlobalAnimationSpeed;

        float StageLightsBrightness = 0.0f;

        Camera3D VenueCamera = Camera3D();

        static void DrawVenueBackground();
        static void UpdateVenue();

        static void GenerateVenueEvents();
    };
}

extern Encore::VenueHandler TheVenueHandler;
#endif
