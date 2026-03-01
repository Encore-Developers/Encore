
#include "ParticleSystem.h"

#include "assets.h"
void Encore::Particle::Update(ParticleSystem* system) {
    time += GetFrameTime();
    switch (type) {
    case FLARE:
        if (time > FLARE_LIFETIME) {
            active = false;
        }
    }
}
void Encore::Particle::Render(ParticleSystem* system) {
    switch (type) {
        case FLARE:
        float lifetime = FLARE_LIFETIME-time;
        float frac = (lifetime/FLARE_LIFETIME);
        color.a = (frac*frac)*255;
        Color white = {255, 255, 255, color.a};
        Rectangle source = { 0.0f, 0.0f, (float)ASSET(hitFlareTex).width, (float)ASSET(hitFlareTex).height };
        float size = 2.4-time*0.5;
        BeginBlendMode(BlendMode::BLEND_ADDITIVE);
        DrawBillboardRec(system->billboardCamera, ASSET(hitFlareTex), source, position, {size*0.9f, size*1.2f}, ColorLerp(color, white, 0));
        DrawBillboardRec(system->billboardCamera, ASSET(hitFlareInnerTex), source, position, {size*0.7f, size}, ColorLerp(color, white, 0.9));
        EndBlendMode();
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
        particle.Update(this);
        if (!particle.active) {
            continue;
        }
        particle.Render(this);
    }
}