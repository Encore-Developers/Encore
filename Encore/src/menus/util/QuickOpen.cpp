#include "QuickOpen.h"

#include "../MenuManager.h"
#include "../gameplay/ReadyUpMenu.h"
#include "song/song.h"
void QuickOpenSongDir(std::filesystem::path dir) {
    if (std::filesystem::is_regular_file(dir)) {
        dir = dir.parent_path();
    }
    if (std::filesystem::exists(dir / "song.ini") && std::filesystem::exists(dir / "notes.mid")) {
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