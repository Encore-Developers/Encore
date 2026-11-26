//
// Created by maria on 23/02/2026.
//

#include "OpenTrackSlot.h"

#include "assets.h"
#include "Track.h"
#include "rlgl.h"

void Encore::OpenTrackSlot::DrawNote(RhythmEngine::NoteEvent *note, bool missed) {
    auto pos = track->GetNotePos3D(note->start.sec);
    Vector3 position = { xPos, 0.0, pos };
    Color color = track->player.QueryColorProfile(colorSlot, track->ColorProfileType);


    if (track->player.engine->chart->overdrive.TimeIsInODPhrase(note->start.sec)) {
        color = track->player.QueryColorProfile(SLOT_OVERDRIVE, track->ColorProfileType);
        ASSET(noteShader).SetUniform("frameColor", track->player.QueryColorProfile(SLOT_FRAME_OVERDRIVE, track->ColorProfileType));
    } else {
        ASSET(noteShader).SetUniform("frameColor", track->player.QueryColorProfile(SLOT_FRAME, track->ColorProfileType));
    }
    if (missed) {
        ASSET(noteShader).SetUniform("frameColor", Color{120, 120, 120, 255});
        color = { 140, 140, 140, 255 };
    }
    ASSET(noteShader).SetUniform("noteColor", color);

    if (note->secLen() > 0) {
        Color sustainColor = missed ? ColorBrightness(color, -0.75) : color;
        DrawSustainTail(note->start.sec, note->end.sec, sustainColor, 0, false);
    }
    rlDrawRenderBatchActive();

    if (note->type == RhythmEngine::NoteEvent::HOPO || note->type == RhythmEngine::NoteEvent::TAP) {
        DrawModelEx(ASSET(openHopoNote),
                    position,
                    { 0 },
                    0,
                    {width/5.0f, track->NoteHeight, length},
                    WHITE);

    } else {
        DrawModelEx(ASSET(openNote),
                    position,
                    { 0 },
                    0,
                    {width/5.0f, track->NoteHeight, length},
                    WHITE);
    }
}

void Encore::OpenTrackSlot::DrawSustainTail(double startTime, double endTime, Color color, float whammy, bool active) {
    if (endTime <= startTime) {
        return;
    }
    float startPos = track->GetNotePos3D(startTime);
    float endPos = track->GetNotePos3D(endTime);
    float additiveTime = 0;
    if (startTime <= TheSongTime.GetElapsedTime() + 0.01) {
        additiveTime = TheSongTime.GetElapsedTime() * 10;
    }
    if (active) {
        additiveTime += sustainShift;
        sustainShift += whammy * GetFrameTime() * 15;
    }
    float tailPos = endPos;
    endPos -= 0.1;
    if (endPos < startPos) {
        endPos = startPos;
    }

    float widthMult = 24;
    Color dimmed = ColorBrightness(color, 0);
    if (active == false) {
        dimmed = ColorAlpha(dimmed, 0.8);
    } else {
        widthMult = 4 / ((whammy / 2) + 0.75f);
    }
    //
    Color bright = ColorBrightness(color, 0.2);
    rlSetBlendMode(RL_BLEND_ADDITIVE);
    rlDisableDepthTest();
    rlBegin(RL_QUADS);
    float regWidthMult = 0.25;
    float w = width * 1.5f;
    float whammyOffset = whammy * (w * regWidthMult);
    rlColor4ub(color.r, color.g, color.b, 0); // sets vertex color
    rlVertex3f(xPos -  w*regWidthMult - whammyOffset, 0.03, startPos); // places vertex
    rlVertex3f(xPos -  w*regWidthMult - whammyOffset, 0.03, endPos);
    rlColor4ub(bright.r, bright.g, bright.b, 64);
    rlVertex3f(xPos, 0.03, endPos);
    rlVertex3f(xPos, 0.03, startPos);
    // since thats all four it stops???? i guess because rlBegin(RL_QUADS) is being used
    // i fucking hate implicit behaviours FUCK implicit behaviours

    rlVertex3f(xPos, 0.03, startPos);
    rlVertex3f(xPos, 0.03, endPos);
    rlColor4ub(color.r, color.g, color.b, 0);
    rlVertex3f(xPos +  w*regWidthMult + whammyOffset, 0.03, endPos);
    rlVertex3f(xPos +  w*regWidthMult + whammyOffset, 0.03, startPos);


    rlColor4ub(color.r, color.g, color.b, 0);
    rlVertex3f(xPos - w*regWidthMult - whammyOffset, 0.03, endPos);
    rlVertex3f(xPos - w*regWidthMult - whammyOffset, 0.03, tailPos);
    rlVertex3f(xPos, 0.03, tailPos);
    rlColor4ub(bright.r, bright.g, bright.b, 64);
    rlVertex3f(xPos, 0.03, endPos);

    rlColor4ub(color.r, color.g, color.b, 0);
    rlVertex3f(xPos + w*regWidthMult + whammyOffset, 0.03, endPos);
    rlColor4ub(bright.r, bright.g, bright.b, 64);
    rlVertex3f(xPos, 0.03, endPos);
    rlColor4ub(color.r, color.g, color.b, 0);
    rlVertex3f(xPos, 0.03, tailPos);
    rlVertex3f(xPos + w*regWidthMult + whammyOffset, 0.03, tailPos);
    // seggs
    { // left side?
        Segment leftSeg;
        float left = (xPos-(width/3));
        float wham = regWidthMult * whammyOffset;
        leftSeg.front.l = { left-wham, 0.3, startPos };
        leftSeg.front.r = { (left+1)+wham, 0.3, startPos };
        leftSeg.back.l = { left-wham, 0.3, endPos };
        leftSeg.back.r = { (left+1)+wham, 0.3, endPos };

        DrawSegment(leftSeg, dimmed, 1, 1);
        DrawSegment(leftSeg, dimmed, 1, 1);

        Segment rightSeg;

        float right = (xPos+(width/3));
        rightSeg.front.l = { (right-1)-wham, 0.3, startPos };
        rightSeg.front.r = { right+wham, 0.3, startPos };
        rightSeg.back.l = { (right-1)-wham, 0.3, endPos };
        rightSeg.back.r = { right+wham, 0.3, endPos };

        DrawSegment(rightSeg, dimmed, 1, 1);
        DrawSegment(rightSeg, dimmed, 1, 1);
    }
    // constexpr float segStep = 0.5f;
    // for (float seg = startPos; seg < endPos; seg += segStep) {
    //     if (seg >= track->GetNotePos3D(track->GetViewEndTime())) break;
    //     if (seg < -3) continue;
    //     float endFade = Clamp(Remap(seg+segStep, endPos - 2, endPos, 1, 0), 0, 1);
    //     float endFadeFront = Clamp(Remap(seg, endPos - 2, endPos, 1, 0), 0, 1);
    //     float laneWidth = width/widthMult;
    //     float halfWidth = width/8;
    //     float dist = (seg - startPos) - additiveTime;
    //     float xEnd = std::sin((dist+segStep)/2) * laneWidth;
    //     float xStart = std::sin((dist)/2) * laneWidth;
    //     auto col = dimmed;
    //     newSeg.front.l = { (xPos-halfWidth)+xStart, 0.3, seg };
    //     newSeg.front.r = { (xPos+halfWidth)+xStart, 0.3, seg };
    //
    //     newSeg.back.l = { (xPos-halfWidth)+xEnd, 0.3, seg+segStep };
    //     newSeg.back.r = { (xPos+halfWidth)+xEnd, 0.3, seg+segStep };
    //     //fuck it we drawin it twice
    //     DrawSegment(newSeg, col, endFadeFront, endFade);
    //     DrawSegment(newSeg, col, endFadeFront, endFade);
    //
    //     newSeg.front.l = { (xPos-halfWidth)-xStart, 0.3, seg };
    //     newSeg.front.r = { (xPos+halfWidth)-xStart, 0.3, seg };
    //
    //     newSeg.back.l = { (xPos-halfWidth)-xEnd, 0.3, seg+segStep };
    //     newSeg.back.r = { (xPos+halfWidth)-xEnd, 0.3, seg+segStep };
    //     DrawSegment(newSeg, col, endFadeFront, endFade);
    //     DrawSegment(newSeg, col, endFadeFront, endFade);
    //
    // }
}

void Encore::OpenTrackSlot::DrawSmasher(bool held) {
    if (animTimer < 1) {
        animTimer += GetFrameTime() * 3.5;
    } else {
        animTimer = 1;
    }
    for (auto& sustain : track->engine->ActiveSustains) {
        auto note = sustain.note;
        if (note->end.sec < TheSongTime.GetElapsedTime()) {
            continue;
        }
        bool matches = false;
        if (note->lane == 0) {
            matches = true;
        }
        Color color = track->player.QueryColorProfile(colorSlot, track->ColorProfileType);
        if (track->player.engine->chart->overdrive.TimeIsInODPhrase(note->start.sec)) {
            color = track->player.QueryColorProfile(SLOT_OVERDRIVE, track->ColorProfileType);
        }
        if (matches) {
            DrawSustainTail(TheSongTime.GetElapsedTime(),
                            note->end.sec, color, track->player.engine->whammy, true);
            if (hitFlare) {
                if (hitFlare->id == hitFlareId) {
                    hitFlare->time = (std::sin(TheSongTime.GetElapsedTime() * 100) + 1) *
                        0.5 * FLARE_LIFETIME * 0.1 + 0.02;
                    //hitFlare->time = 0;
                }
            }
            break;
        }
    }
}

void Encore::OpenTrackSlot::AnimateHit(bool perfect, Color color) {
    float size = 2.4f;
    if (!perfect) {
        color = ColorBrightness(color, -0.7);
    } else {
        color = ColorBrightness(GOLD, -0.3);
        size = 2.9f;
    }
    animTimer = 0;
    overhitTimer = 0;

    Particle part;
    part.setActive(true)
        .setType(OPENFLARE)
        .pos(xPos, 0.15, 0)
        .size(size)
        .col(color);
    hitFlare = track->particleSystem->SpawnParticle(part);
    hitFlareId = hitFlare->id;
}