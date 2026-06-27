#include "TheGame.h"
#include "SDL3/SDL_filesystem.h"
#include "SDL3/SDL_messagebox.h"
#include "util/enclog.h"

// you just lost...
TheGame game;

int main(int argc, char *argv[]) {
    return game.Run(argc, argv);
}

    // note: what was commented out here was goofy shit.
    // do not uncomment it. there are better ways to handle this type of thing.
    /*
    int result;
#ifdef NDEBUG
    try {
        result = game.Run(argc, argv);
    } catch (const std::exception &e) {
        Encore::Log::Error("Encore crashed! {}", e.what());
        std::string exception = e.what();
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Encore has crashed!", ("Reason:\n" + exception).c_str(), NULL);
        return 1;
    }
#else
    result = game.Run(argc, argv);
// #endif
    return result;
}
*/
