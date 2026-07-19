
#include "ParticleSystem.h"

#include "assets.h"
#include "gameplay/trackRenderer/Track.h"

void Encore::Particle::Update(ParticleSystem* system) {
    time += GetFrameTime();
    switch (type) {
    case FLARE:
    case KICKFLARE:
    case OPENFLARE:
        if (time > FLARE_LIFETIME) {
            active = false;
        }
        break;
    case SHOCKWAVE:
        if (time > SHOCKWAVE_LIFETIME) {
            active = false;
        }
        break;
    case MARKIPLIER_FLASH:
        if (time > MARKIPLIER_LIFETIME) {
            active = false;
        }
        break;
    default:
        break;
    }
}
void Encore::Particle::Render(ParticleSystem* system) {
    switch (type) {
        case FLARE: {
            float lifetime = FLARE_LIFETIME-time;
            float frac = (lifetime/FLARE_LIFETIME);
            color.a = (frac*frac)*250;
            Color white = {255, 255, 255, color.a};
            Rectangle source = { 0.0f, 0.0f, (float)ASSET(hitFlareTex).width, (float)ASSET(hitFlareTex).height };
            float size = Size-time*0.5f;
            BeginBlendMode(BlendMode::BLEND_ADDITIVE);
            DrawBillboardRec(system->billboardCamera, ASSET(hitFlareTex), source, position, {size*0.9f, size*1.2f}, ColorBrightness(ColorLerp(color, white, 0), -0.1));
            DrawBillboardRec(system->billboardCamera, ASSET(hitFlareInnerTex), source, position, {size*0.7f, size}, ColorBrightness(ColorLerp(color, white, 0.9), -0.1));
            EndBlendMode();
            break;
        }
        case KICKFLARE: {
            float lifetime = FLARE_LIFETIME-time;
            float frac = (lifetime/FLARE_LIFETIME);
            color.a = (frac*frac)*200;
            Color white = {255, 255, 255, color.a};
            // Rectangle source = { 0.0f, 0.0f, (float)ASSET(hitFlareTex).width, (float)ASSET(hitFlareTex).height };
            float size = Size-time*0.5f;
            BeginBlendMode(BlendMode::BLEND_ADDITIVE);
            //DrawBillboardRec(system->billboardCamera, ASSET(hitFlareTex), source, position, {size*0.9f, size*1.2f}, ColorLerp(color, white, 0));
            //DrawBillboardRec(system->billboardCamera, ASSET(hitFlareInnerTex), source, position, {size*0.7f, size}, ColorLerp(color, white, 0.9));
            ASSET(kickFlareModel).Fetch().materials[0].maps[0].texture = ASSET(kickFlareTex);
            DrawModelEx(ASSET(kickFlareModel), position, {-1, 0, 0}, 50, {1.4f, 1.2f*size, 1.2f*size}, ColorLerp(color, white, 0));
            ASSET(kickFlareModel).Fetch().materials[0].maps[0].texture = ASSET(kickFlareInnerTex);
            DrawModelEx(ASSET(kickFlareModel), position, {-1, 0, 0}, 50, {1.4f, 1.2f*size, 1.2f*size}, ColorLerp(color, white, 0.9));
            EndBlendMode();
            break;
        }
        case OPENFLARE: {
            float lifetime = FLARE_LIFETIME-time;
            float frac = (lifetime/FLARE_LIFETIME);
            color.a = (frac*frac)*200;
            Color white = {255, 255, 255, color.a};
            float size = Size-time*0.5f;
            BeginBlendMode(BlendMode::BLEND_ADDITIVE);
            ASSET(kickFlareModel).Fetch().materials[0].maps[0].texture = ASSET(kickFlareTex);
            DrawModelEx(ASSET(kickFlareModel), position, {-1, 0, 0}, 50, {1.4f, 1.2f*size, 2.0f*size}, ColorLerp(color, white, 0));
            ASSET(kickFlareModel).Fetch().materials[0].maps[0].texture = ASSET(kickFlareInnerTex);
            DrawModelEx(ASSET(kickFlareModel), position, {-1, 0, 0}, 50, {1.4f, 1.2f*size, 2.0f*size}, ColorLerp(color, white, 0.9));
            EndBlendMode();
            break;
        }
        case SHOCKWAVE: {
            float lifetime = SHOCKWAVE_LIFETIME-time;
            float frac = (lifetime/SHOCKWAVE_LIFETIME);
            // color.a = (frac*frac)*color.a;
            Color white = {color.r, color.g, color.b, (unsigned char)((frac*frac)*color.a)};
            Rectangle source = { 0.0f, 0.0f, (float)ASSET(shockwaveTex).width, (float)ASSET(shockwaveTex).height };
            float size = Size+(time * 16.0f);
            BeginBlendMode(BlendMode::BLEND_ADDITIVE);
            DrawBillboardRec(system->billboardCamera, ASSET(shockwaveTex), source, position, {size, size/2},  white);
            EndBlendMode();
            break;
        }
        case MARKIPLIER_FLASH: {
            float lifetime = MARKIPLIER_LIFETIME-time;
            float frac = (lifetime/MARKIPLIER_LIFETIME);
            color.a = (frac*frac)*255;
            Rectangle source = { 0.0f, 0.0f, (float)ASSET(markiplierFlash).width, (float)ASSET(markiplierFlash).height };
            float size = Size-time;
            BeginBlendMode(BlendMode::BLEND_ADDITIVE);
            DrawBillboardRec(system->billboardCamera, ASSET(markiplierFlash), source, position, {size, size}, color);
            EndBlendMode();
            break;
        }
        default:
            break;
    }
}


void Encore::ParticleSystem::Render() {

    billboardCamera = track->AnimCamera;
    billboardCamera.target.z -= 10;

    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle& particle = particles[i];
        if (!particle.active) {
            continue;
        }
        particle.Render(this);
        particle.Update(this);
    }
}