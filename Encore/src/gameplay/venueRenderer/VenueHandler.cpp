//
// Created by Student on 5/17/2026.
// This is rushed code and will need way more time to work on. This is not final.
// There are alot of bugs within the drawing code (for example, resizing/dragging the window some how breaks the highway and the venue)
// Please expect many weird bugs.
//


#include "VenueHandler.h"

#include "assets.h"
#include "raygui.h"
#include "raymath.h"
#include "rlgl.h"
#include "external/glad.h"
#include "gameplay/enctime.h"
#include <algorithm>

void Encore::VenueHandler::GenerateVenueEvents() {
    events.clear();

    // Add default event
    events.push_back(VenueEvent {0, VenueEventType::LIGHTSANIMATION_BLACKOUT});


    for (const auto &[name, start] : TheSongTime.Sections) {
        auto event = VenueEvent();

        std::string sectionName = name;
        std::transform(sectionName.begin(), sectionName.end(), sectionName.begin(), tolower);

        event.musicPosition = static_cast<float>(start);


        // Generates events based off of section names
        // This is here to test out venue effects before loading midi files.
        // There's probably a better way to do this...

        if (sectionName.find("verse") != std::string::npos && sectionName.find("pre") == std::string::npos) {
            event.eventType = VenueEventType::LIGHTSANIMATION_ROAMING;
        }
        else if (sectionName.find("chorus") != std::string::npos && sectionName.find("pre") == std::string::npos) {
            event.eventType = VenueEventType::LIGHTSANIMATION_ONBEAT;
        }
        else {
            if (sectionName.find("bridge") != std::string::npos || sectionName.find("breakdown") != std::string::npos || sectionName.find("pre") != std::string::npos) {
                event.eventType = VenueEventType::LIGHTSANIMATION_FLASH;
            }
            else {
                event.eventType = VenueEventType::LIGHTSANIMATION_STATIC;
            }
        }

        events.push_back(event);
    }
}

void Encore::VenueHandler::UpdateVenue() {

    // Venue Animation (for random animation value)
    StageLightsAnimationTime += static_cast<float>((((TheSongTime.BPMChanges[TheSongTime.CurrentBPM].bpm / 60.0f) * TheSongTime.TimeSigChanges[TheSongTime.CurrentTimeSig].numer) / 2000.0f) * 3.14f);

    // Assuming the events are already sorted, find the latest event crossed by the music
    for (const VenueEvent e : events) {
        if (static_cast<float>(TheAudioManager.GetMusicTimePlayed()) >= e.musicPosition) {
            currentVenueEvent = e;
        }
    }

    if (TheSongTime.GetBeatlineDelta() < previousDeltaBeat) {
        beatCount++;
    }
    previousDeltaBeat = static_cast<float>(TheSongTime.GetBeatlineDelta());

    switch (currentVenueEvent.eventType) {
    case VenueEventType::LIGHTSANIMATION_FLASH:
    case VenueEventType::LIGHTSANIMATION_ROAMING:
        StageLightsAnimationStatic -= 0.125f;
        StageLightsAnimationRandom += 0.1f;
        break;
    case VenueEventType::LIGHTSANIMATION_BLACKOUT:
    case VenueEventType::LIGHTSANIMATION_STATIC:
    case VenueEventType::LIGHTSANIMATION_ONBEAT:
        StageLightsAnimationStatic += 0.1f;
        StageLightsAnimationRandom -= 0.1f;
        break;
    }

    StageLightsAnimationStatic = Clamp(StageLightsAnimationStatic, 0.0f, 1.0f);
    StageLightsAnimationRandom = Clamp(StageLightsAnimationRandom, 0.0f, 1.0f);

    // WE NEED MORE!!!
    // MORE SHIT,
    // spaghetti,
    // CODE!!!

    if (currentVenueEvent.eventType == VenueEventType::LIGHTSANIMATION_FLASH || currentVenueEvent.eventType == VenueEventType::LIGHTSANIMATION_ONBEAT) {
        StageLightsBrightness = 0.5f + (0.5f-(TheSongTime.GetBeatlineDelta() * 0.5f));
    }
    else if (currentVenueEvent.eventType == VenueEventType::LIGHTSANIMATION_BLACKOUT) {
        StageLightsBrightness -= 0.1f;
        StageLightsBrightness = std::max(StageLightsBrightness, 0.0f);
        std::cout << "blackout" << std::endl;
    }
    else {
        StageLightsBrightness = Lerp(StageLightsBrightness, 1.0f, 0.1f);
    }

    if (currentVenueEvent.eventType == VenueEventType::LIGHTSANIMATION_ONBEAT) {
        if (beatCount % 2 == 1) {
            StageLightGlobalRotation += 0.25f;
            StageLightGlobalRotation = std::min(StageLightGlobalRotation, 1.0f);
        }
        else {
            StageLightGlobalRotation -= 0.25f;
            StageLightGlobalRotation = std::max(StageLightGlobalRotation, -1.0f);
        }
    }

}


// This might be worse...
void Encore::VenueHandler::DrawVenueBackground() {
    Camera3D BaseCamera = VenueCamera;

    rlEnableDepthMask();
    rlEnableDepthTest();
    rlEnableBackfaceCulling();

    Vector2 CameraShake = { (float)sin(TheAudioManager.GetMusicTimePlayed()) * 2.0f, (float)cos(TheAudioManager.GetMusicTimePlayed() * 0.5f) * 1.5f};

    BaseCamera = {
        { -7.0f + CameraShake.x, 10.0f + CameraShake.y, 57.5f + CameraShake.x },
        { 0.0f, 6.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        60.0f,
    };

    Color PrimaryLightsColor = { 223, 128, 242 };
    Color SecondaryLightsColor = { 219, 129, 255};

    Color CombinedLighting = ColorLerp(PrimaryLightsColor, SecondaryLightsColor, 0.5f);

    float spotlightIntensity = StageLightsBrightness / 2.0f;

    Model StageModel = ASSET(VenueStage);
    Model CrowdModel = ASSET(CrowdTestModel); // Wish this would also use the animations...
    Model StageLampHookModel = ASSET(StageLampHook);

    Material StageLampHookGlareMaterial = StageLampHookModel.materials[2];
    SetMaterialTexture(&StageLampHookGlareMaterial, MATERIAL_MAP_DIFFUSE, ASSET(StageLampLightTex));

    Color StageModelColor = ColorBrightness(CombinedLighting, -1.0f + (StageLightsBrightness * 2.0f));
    StageModelColor.a = 255; // Just incase mixing colors goes wrong


    UpdateVenue();

    BeginMode3D(BaseCamera);
    BeginBlendMode(BLEND_ALPHA);
    DrawModelEx(StageModel,{ 0 }, { 0 }, 0, {1, 1, 1}, StageModelColor);

    // Draw crowd
    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            Vector2 CrowdPersonPosition = Vector2();
            CrowdPersonPosition.x = 12.0f * x;
            CrowdPersonPosition.y = 12.0f * y;

            //DrawModelEx(CrowdModel, {-47.5f + CrowdPersonPosition.x, 0, 90.0f + CrowdPersonPosition.y}, {0}, 0, {1, 1, 1}, {255, 255, 255, 255});
            //DrawModelEx(CrowdModel, {0.0f + CrowdPersonPosition.x, 0, 0.0f + CrowdPersonPosition.y}, {0}, 0, {1, 1, 1}, {255, 255, 255, 255});

        }
    }

    rlDisableDepthTest();
    rlDisableDepthMask();

    /// Draw base lamps, first
    for (int sl = 0; sl < 10; sl++) {
        StageLampHookModel = ASSET(StageLampHook);
        auto StageLampPosition = Vector3(-16, 15.0f, -12.0f);
        StageLampPosition.x += 3.5f*sl;

        float randomRotationAngle = 15.0f * sin(StageLightsAnimationTime - (sl*24.5f));
        float staticRotationAngle = 7.5f * StageLightGlobalRotation;

        float finalRotationAngle = 0.0f;

        finalRotationAngle = Lerp(finalRotationAngle, randomRotationAngle, StageLightsAnimationRandom);
        finalRotationAngle = Lerp(finalRotationAngle, staticRotationAngle, StageLightsAnimationStatic);

        if (sl % 2 == 1) {
            finalRotationAngle = -finalRotationAngle;
        }

        StageLampHookModel.transform *= MatrixRotateXYZ({0, -90 * DEG2RAD, finalRotationAngle * DEG2RAD});
        StageLampHookModel.transform *= MatrixTranslate(StageLampPosition.x, StageLampPosition.y, StageLampPosition.z);

        for (int m = 0; m < StageLampHookModel.meshCount; m++) {
            Mesh LampMesh = StageLampHookModel.meshes[m];

            if (m != 2 && m != 3) {
                DrawMesh(LampMesh, StageLampHookModel.materials[1], StageLampHookModel.transform);
            }
        }
    }
    EndBlendMode();
    /// Draw the light part of the lamps, second
    BeginBlendMode(BLEND_ADDITIVE);
    for (int sl = 0; sl < 10; sl++) {
        StageLampHookModel = ASSET(StageLampHook);
        Color CurrentLightColor = PrimaryLightsColor;

        auto StageLampPosition = Vector3(-16, 15.0f, -12.0f);
        StageLampPosition.x += 3.5f*sl;

        float randomRotationAngle = 15.0f * sin(StageLightsAnimationTime - (sl*24.5f));
        float staticRotationAngle = 7.5f * StageLightGlobalRotation;

        float finalRotationAngle = 0.0f;

        finalRotationAngle = Lerp(finalRotationAngle, randomRotationAngle, StageLightsAnimationRandom);
        finalRotationAngle = Lerp(finalRotationAngle, staticRotationAngle, StageLightsAnimationStatic);

        if (sl % 2 == 1) {
            CurrentLightColor = SecondaryLightsColor;
            finalRotationAngle = -finalRotationAngle;
        }

        StageLampHookModel.transform *= MatrixRotateXYZ({0, -90 * DEG2RAD, finalRotationAngle * DEG2RAD});
        StageLampHookModel.transform *= MatrixTranslate(StageLampPosition.x, StageLampPosition.y, StageLampPosition.z);

        for (int m = 0; m < StageLampHookModel.meshCount; m++) {
            Mesh LampMesh = StageLampHookModel.meshes[m];

            if (m == 2) {
                StageLampHookGlareMaterial.maps[MATERIAL_MAP_DIFFUSE].color = CurrentLightColor;

                if (currentVenueEvent.eventType == VenueEventType::LIGHTSANIMATION_ONBEAT) {
                    if (beatCount % 2 == sl % 2) {
                        StageLampHookGlareMaterial.maps[MATERIAL_MAP_DIFFUSE].color.a = (int)(150 * spotlightIntensity);
                    }
                    else {
                        StageLampHookGlareMaterial.maps[MATERIAL_MAP_DIFFUSE].color.a = (int)(120 * spotlightIntensity);
                    }
                }
                else {
                    StageLampHookGlareMaterial.maps[MATERIAL_MAP_DIFFUSE].color.a = (int)(150 * spotlightIntensity);
                }

                DrawMesh(LampMesh, StageLampHookGlareMaterial, StageLampHookModel.transform);
            }
            else if (m == 3) {
                StageLampHookGlareMaterial.maps[MATERIAL_MAP_DIFFUSE].color = CurrentLightColor;
                StageLampHookGlareMaterial.maps[MATERIAL_MAP_DIFFUSE].color.a = (int)(255 * spotlightIntensity);
                DrawMesh(LampMesh, StageLampHookModel.materials[0], StageLampHookModel.transform);
            }
        }

        Vector3 StageLightGlarePosition = StageLampPosition;

        StageLightGlarePosition.y += (sin(finalRotationAngle * DEG2RAD) * 3.0f) - 2.0f;
        StageLightGlarePosition.z += cos(finalRotationAngle * DEG2RAD) * 1.832f;

        DrawBillboard(BaseCamera, ASSET(StageLampLightGlow), StageLightGlarePosition, 7.0f, ColorAlpha(CurrentLightColor, 0.9f * spotlightIntensity));
        DrawBillboard(BaseCamera, ASSET(StageLampLightGlare), StageLightGlarePosition, 13.5f, ColorAlpha(ColorBrightness(CurrentLightColor, 1.25f), 1.0f * spotlightIntensity));

    }
    EndBlendMode();

    EndMode3D();

    // Enable DepthMask so that the highway doesn't break while rendering
    rlEnableDepthMask();

};