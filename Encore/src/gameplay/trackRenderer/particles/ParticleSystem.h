
#ifndef ENCORE_PARTICLESYSTEM_H
#define ENCORE_PARTICLESYSTEM_H

#define MAX_PARTICLES 300
#define FLARE_LIFETIME 0.18f
#define SHOCKWAVE_LIFETIME 0.3f

#include "raylib.h"
#include "../Track.h"

namespace Encore {

    class ParticleSystem;
    class Track;

    enum ParticleType {
        FLARE,
        SMOKE,
        SHOCKWAVE
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

        Particle& pos(Vector3 _position) {
            position = _position;
            return *this;
        }

        Particle& vel(Vector3 _velocity) {
            velocity = _velocity;
            return *this;
        }

        Particle& col(Color _color) {
            color = _color;
            return *this;
        }

        Particle& setActive(bool _active) {
            active = _active;
            return *this;
        }

        Particle& setType(ParticleType _type) {
            type = _type;
            return *this;
        }

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
