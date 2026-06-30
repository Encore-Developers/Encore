#include "QuickOpen.h"

#include "../MenuManager.h"
#include "../gameplay/ReadyUpMenu.h"
#include "RhythmEngine/Replay.h"
#include "song/song.h"
#include "song/songlist.h"
void QuickOpenSongDir(std::filesystem::path dir) {
    if (std::filesystem::is_regular_file(dir)) {
        if (dir.extension() == ".encrReplay") {
            std::shared_ptr<Encore::RhythmEngine::Replay> replay = std::make_shared<Encore::RhythmEngine::Replay>();
            encore::bin_ifstream_le stream(dir, std::ios::binary);
            replay->Load(stream);
            stream.close();
            if (!replay->loaded) return;
            for (auto& part : replay->participants) {
                std::shared_ptr<Player> fakePlayer = std::make_shared<Player>();
                fakePlayer->Name = part.name;
                fakePlayer->Instrument = part.instrument;
                fakePlayer->Difficulty = part.difficulty;
                fakePlayer->bindingType = part.bindingType;
                fakePlayer->NoteSpeed = part.noteSpeed;
                fakePlayer->HighwayLength = part.trackLength;
                fakePlayer->ReplaySlot = part.activeSlot;
                fakePlayer->PlaybackReplay = replay;
                fakePlayer->ReplayPlayer = std::make_shared<Encore::RhythmEngine::ReplayPlayer>(*replay);
                fakePlayer->ReplayPlayer->slotFilter = part.activeSlot;
                fakePlayer->joypadID = -3;

                for (size_t i = 0; i < MAX_PLAYERS; i++) {
                    if (!ThePlayerManager.ActivePlayers[i]) {
                        ThePlayerManager.AddActivePlayer(fakePlayer, i);
                        break;
                    }
                }
            }
            auto song = TheSongList.songHashIndex[replay->song];

            if (!song) {
                Encore::Log::Error("Error: Replay song not found!");
                return;
            }

            TheMenuManager.CreateAndSwitchMenu<ReadyUpMenu>(song);
            return;
        }
        dir = dir.parent_path();
    }
    if (std::filesystem::exists(dir / "song.ini") && (std::filesystem::exists(dir / "notes.mid") || std::filesystem::exists(dir / "notes.chart"))) {
        Encore::Log::Info("Running game from Quick Open");
        Encore::Log::Info("Path: {}", dir.string());
        static std::shared_ptr<Song> song;
        song = std::make_shared<Song>();
        song->songInfoPath = dir / "song.ini";
        song->songDir = dir;
        song->LoadSongIni(dir);
        //song->LoadAlbumArt();

        TheMenuManager.CreateAndSwitchMenu<ReadyUpMenu>(song.get());
    }
}