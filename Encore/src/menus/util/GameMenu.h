#pragma once
#include <iostream>
#include <locale>
#include <sstream>

// Just some commonly used utilities

template <typename CharT>
struct Separators : public std::numpunct<CharT> {
    virtual std::string do_grouping() const { return "\003"; }
};


namespace GameMenu {

    inline std::string scoreCommaFormatter(double value) {
        std::stringstream ss;
        ss.imbue(std::locale(std::cout.getloc(), new Separators<char>()));
        ss << std::fixed << value;
        return ss.str().substr(0, ss.str().size() - 7);
    }
    void DrawTopOvershell(float TopOvershell);
    void DrawBottomOvershell();
    static bool FirstMainMenuBoot = true;

    static bool streamsLoaded = false;
    void DrawFPS(int posX, int posY);
    void DrawTopBarText(bool ShowFPS = false, bool ShowLogo = true);

    void DrawAlbumArtBackground();
}