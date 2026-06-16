#include "session.h"
size_t Encore::Session::FindOrCreateEmptyPlayerSlot() {
    for (size_t i = 0; i < players.size(); ++i) {
        if (!players[i]) {
            return i;
        }
    }
    players.emplace_back();
    return players.size() - 1;
}