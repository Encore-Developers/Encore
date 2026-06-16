#pragma once
#include "session.h"
namespace Encore {

    // Unified interface for both local and online
    class LocalSession : public Session {
    public:

        virtual bool IsOnline() {return false;};

        virtual void AddPlayer(ClientID id, ControllerIdentity controller);
        virtual void RemovePlayer(unsigned int playerId);

        virtual void SetPlayerProfile(PlayerID player, std::shared_ptr<Profile> profile);

        virtual void PushPlaylistSong(SyncedSongRef song);
        virtual void PopPlaylistSong();
        virtual void ClearPlaylist();

        virtual void ToSongSelect();

        virtual void ReadyUpForSong(SyncedSongRef song);
        virtual void LoadSong(SyncedSongRef song);
        virtual void PlaySong(SyncedSongRef song);
        virtual void ShowResults();

        virtual void ResetSignals();
        virtual void SignalPlayer(PlayerID player, SignalState state);
        virtual void SignalClient();
        virtual void CheckSignals();

        virtual SignalWait WaitForSignal();
    };
}