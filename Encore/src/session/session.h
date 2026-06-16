#pragma once
#include "client.h"
#include "users/player.h"

#include <coroutine>
#include <memory>
#include <vector>

namespace Encore {

    // Unified interface for both local and online
    class Session : public EventSource {
    public:
        Session() = default;
        ~Session() override = default;
        std::vector<std::shared_ptr<Player>> players;
        std::vector<std::shared_ptr<Client>> clients;
        std::vector<SyncedSongRef> playlist;

        struct GameplayFlow {
            struct promise_type {
                GameplayFlow get_return_object() {return {.handle = std::coroutine_handle<promise_type>::from_promise(*this)};}
                std::suspend_always initial_suspend() {return {};}
                std::suspend_always final_suspend() noexcept {return {};}
                void return_void() {}
                void unhandled_exception() {}
            };

            std::coroutine_handle<promise_type> handle;
            operator std::coroutine_handle<promise_type>() const { return handle; }
            operator std::coroutine_handle<>() const { return handle; }
        };

        struct SignalWait {
            bool signal_result = false;

            constexpr bool await_ready() { return false; }
            bool await_suspend(std::coroutine_handle<> handle) {

                return false;
            }
            constexpr bool await_resume() {return signal_result;}
        };

        virtual bool IsOnline();

        virtual void AddPlayer(ClientID id, ControllerIdentity controller);
        virtual void RemovePlayer(unsigned int playerId);
        virtual size_t FindOrCreateEmptyPlayerSlot();

        virtual void SetPlayerProfile(PlayerID player, std::shared_ptr<Profile> profile);

        virtual void PushPlaylistSong(SyncedSongRef song);
        virtual void PopPlaylistSong();
        virtual void ClearPlaylist();

        virtual void ToSongSelect();

        GameplayFlow StartGameplayFlow() {
            for (auto& song : playlist) {
                ReadyUpForSong(song);
                if (!co_await WaitForSignal()) {
                    break;
                }

                LoadSong(song);
                if (!co_await WaitForSignal()) {
                    break;
                }

                PlaySong(song);
                if (!co_await WaitForSignal()) {
                    break;
                }

                ShowResults();
                co_await WaitForSignal();
            }
            ClearPlaylist();

            ToSongSelect();
            co_return;
        }

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
        virtual SignalWait WaitForSignal();
    };
}
