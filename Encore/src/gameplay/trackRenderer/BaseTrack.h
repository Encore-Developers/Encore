//
// Created by maria on 14/01/2026.
//

#ifndef ENCORE_BASETRACK_H
#define ENCORE_BASETRACK_H
#include "raylib.h"
#include "users/player.h"

namespace Encore {
    class BaseTrack {
    public:
        virtual ~BaseTrack() = default;
        void Draw();
        virtual void Load();
        virtual void DrawNotes() = 0;
        virtual void DrawStrikeline() = 0;
        virtual void DrawBeatlines() = 0;
        BaseTrack(Player &player_)
            : player(player_) {
        };
    protected:
        Player& player;
        Camera3D camera;
        RenderTexture2D GameplayRenderTexture;
    };
}

#endif //ENCORE_BASETRACK_H