#pragma once
#include "client.h"
#include "users/player.h"

#include <memory>
#include <vector>

namespace Encore {

    // Unified interface for both local and online
    class Session {
    public:
        Session() = default;
        virtual ~Session() = default;
        std::vector<std::shared_ptr<Player>> players;
        std::vector<std::shared_ptr<Client>> clients;
        std::vector<SyncedSongRef> playlist;

        virtual bool IsOnline();

        virtual void AddPlayer(ClientID id, ControllerIdentity controller);
        virtual void RemovePlayer(unsigned int playerId);

        virtual void SetPlayerProfile(PlayerID player, std::shared_ptr<Profile> profile);

        virtual void PushPlaylistSong(SyncedSongRef song);
        virtual void PopPlaylistSong();
        virtual void ClearPlaylist();


        virtual TEMP_coroutine GameplayFlow();

        virtual void ReadyUpForSong(SyncedSongRef song);
        virtual void LoadSong(SyncedSongRef song);
        virtual void PlaySong(SyncedSongRef song);
        virtual void ShowResults();


        /*
        Signaling: Very commonly we need to wait for all players to be done with some
        task: readying up, loading a song, reaching the end of the song, and finishing
        looking at results. Instead of having unique logic for things like readying up,
        this will unify the logic for every instance of this being needed.
        This also can handle fail or cancel states.
        */
        virtual void ResetSignals();
        virtual void SignalPlayer(PlayerID player, SignalState state);
        virtual void SignalClient();
        virtual void CheckSignals();

        // Returns true once all players have signaled, returns false if any player cancels
        virtual TEMP_coroutine<bool> WaitForSignal();
    };
}
