//
// Created by maria on 29/05/2026.
//

#ifndef ENCORE_JUKEBOX_H
#define ENCORE_JUKEBOX_H

namespace Encore {
    class Jukebox
    {
        void AdjustVolume();
        void PickRandomSong();
        void StartStreams();
        void LoadStreams();

    public:
        bool playing = false;
        bool streamsLoaded = false;
        void UnloadStreams();
        void FirstLoad();
        void Update();
        void TogglePlayback();
        void SkipSong();
        void StartPlayback();
    };
}

extern Encore::Jukebox TheGameJukebox;
#endif //ENCORE_JUKEBOX_H