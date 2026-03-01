
#ifndef ENCORE_PARTICLESYSTEM_H
#define ENCORE_PARTICLESYSTEM_H

#define MAX_PARTICLES 300
#define FLARE_LIFETIME 0.2f

#include "raylib.h"
#include "../Track.h"

namespace Encore {

    class ParticleSystem;
    class Track;

    enum ParticleType {
        FLARE,
        SMOKE
    };

    class Particle {
    public:
        bool active : 1 = false; // The bit fields here save 4 * MAX_PARTICLES bytes
        unsigned int id = 0;
        ParticleType type : 31;
        float time = 0.0;
        Vector3 position;
        Vector3 velocity;
        Color color;

        void Render(ParticleSystem*);
        void Update(ParticleSystem*);
    };

    class ParticleSystem {
        Particle particles[MAX_PARTICLES] = {};
        unsigned int nextSpawnIndex = 0;
        unsigned int nextId = 0;

    public:
        Camera3D billboardCamera;
        Track* track;
        void Render();
        Particle* SpawnParticle(Particle& particle) {
            Particle& newPart = particles[nextSpawnIndex];
            newPart = particle;
            newPart.id = nextId;
            nextId++;
            nextSpawnIndex = (nextSpawnIndex + 1) % MAX_PARTICLES;
            return &newPart;
        }
    };
}

#endif // ENCORE_PARTICLESYSTEM_H
