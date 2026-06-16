#include "local_session.h"

#include "session_events.h"
void Encore::LocalSession::AddPlayer(ClientID id, ControllerIdentity controller) {
    auto slot = FindOrCreateEmptyPlayerSlot();
    std::shared_ptr<Player> newPlayer = std::make_shared<Player>();
    newPlayer->index = slot;
    newPlayer->controller = controller;
    players[slot] = newPlayer;
}
void Encore::LocalSession::RemovePlayer(unsigned int playerId) {
    players[playerId] = nullptr;
}
void Encore::LocalSession::SetPlayerProfile(
    PlayerID player, std::shared_ptr<Profile> profile
) {
    players[player]->profile = profile;
}
void Encore::LocalSession::PushPlaylistSong(SyncedSongRef song) {
    playlist.push_back(song);
}
void Encore::LocalSession::PopPlaylistSong() {
    playlist.pop_back();
}
void Encore::LocalSession::ClearPlaylist() {
    playlist.clear();
}
void Encore::LocalSession::ToSongSelect() {
    FireEventTemp(SongSelectEvent());
}
void Encore::LocalSession::ReadyUpForSong(SyncedSongRef song) {
    FireEventTemp(ReadyUpEvent(song));
}
void Encore::LocalSession::LoadSong(SyncedSongRef song) {
    FireEventTemp(LoadSongEvent(song));
}
void Encore::LocalSession::PlaySong(SyncedSongRef song) {
    FireEventTemp(PlaySongEvent(song));
}
void Encore::LocalSession::ShowResults() {
    FireEventTemp(ShowResultsEvent());
}
void Encore::LocalSession::ResetSignals() {
    for (auto& player : players) {
        player->signal = SignalState::UNSIGNALED;
    }
}
void Encore::LocalSession::SignalPlayer(PlayerID player, SignalState state) {
    players[player]->signal = state;
    CheckSignals();
}
void Encore::LocalSession::SignalClient() {
    for (auto& player : players) {
        player->signal = SignalState::SIGNALED;
    }
    CheckSignals();
}
void Encore::LocalSession::CheckSignals() {
    bool all = true;
    for (auto& player: players) {
        switch (player->signal) {
        case SignalState::UNSIGNALED:
            all = false;
        case SignalState::SIGNALED:
            break;
        case SignalState::CANCELLED:
            currentWait.signal_result = false;
            currentFlow.handle.resume();
            break;
        }
    }
    if (all) {
        currentWait.signal_result = true;
        currentFlow.handle.resume();
    }
}
Encore::Session::SignalWait Encore::LocalSession::WaitForSignal() {
    for (auto& player : players) {
        player->signal = SignalState::UNSIGNALED;
    }
    currentWait = SignalWait();
    return currentWait;
}