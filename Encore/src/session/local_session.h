#pragma once
#include "session.h"
namespace Encore {

    // Unified interface for both local and online
    class LocalSession : public Session {
    public:

        GameplayFlow currentFlow;
        SignalWait currentWait;

        bool IsOnline() override {return false;};

        void AddPlayer(ClientID id, ControllerIdentity controller) override;
        void RemovePlayer(unsigned int playerId) override;

        void SetPlayerProfile(PlayerID player, std::shared_ptr<Profile> profile) override;

        void PushPlaylistSong(SyncedSongRef song) override;
        void PopPlaylistSong() override;
        void ClearPlaylist() override;

        void ToSongSelect() override;

        void ReadyUpForSong(SyncedSongRef song) override;
        void LoadSong(SyncedSongRef song) override;
        void PlaySong(SyncedSongRef song) override;
        void ShowResults() override;

        void ResetSignals() override;
        void SignalPlayer(PlayerID player, SignalState state) override;
        void SignalClient() override;
        void CheckSignals() override;

        SignalWait WaitForSignal() override;
    };
}