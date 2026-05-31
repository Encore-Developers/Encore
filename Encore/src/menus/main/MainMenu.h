#pragma once
//
// Created by marie on 02/05/2024.
//

#include "GLFW/glfw3.h"
#include "song/songlist.h"
#include "assets.h"
#include "users/player.h"
#include "../overshell/OvershellMenu.h"
#include "menus/locale/Locale.h"
#include "menus/util/ButtonActionRegistry.h"
#include "song/ArtLoader.h"

enum TextAlign {
    LEFT,
    CENTER,
    RIGHT
};

template <typename CharT>
struct Separators : public std::numpunct<CharT> {
    virtual std::string do_grouping() const { return "\003"; }
};

struct TextDisplay {
    Font font = ASSET(rubik);
    Vector2 pos {0,0};
    float fontSize = 0;
    Color color{255, 255, 255, 255};
    TextAlign align = LEFT;

    TextDisplay& Fnt(const Font &_font) {
        this->font = _font;
        return *this;
    }
    TextDisplay& Pos(const Vector2 _pos) {
        this->pos = _pos;
        return *this;
    }
    TextDisplay& Pos(float x, float y) {
        this->pos = {x,y};
        return *this;
    }
    TextDisplay& Size(float _fontSize) {
        this->fontSize = _fontSize;
        return *this;
    }
    TextDisplay& Col(const Color _color) {
        this->color = _color;
        return *this;
    }
    TextDisplay& Align(const TextAlign _align) {
        this->align = _align;
        return *this;
    }

    void lDrawText(const std::string &localizeKey) const;
    void DrawText(const std::string &text) const;

    float lTextWidth(const std::string &localeKey) {
        return TextWidth(LOCALISE(localeKey));
    }
    float TextWidth(const std::string &text) {
        return MeasureTextEx(font, text.c_str(), fontSize, 0).x;
    }
};

namespace GameMenu {

    inline std::string scoreCommaFormatter(int value) {
        std::stringstream ss;
        ss.imbue(std::locale(std::cout.getloc(), new Separators<char>()));
        ss << std::fixed << value;
        return ss.str();
    }
    void DrawTopOvershell(float TopOvershell);
    void DrawBottomOvershell();
    Texture2D LoadTextureFilter(const std::filesystem::path &texturePath);
    Font LoadFontFilter(const std::filesystem::path &fontPath);
    void mhDrawText(
        const Font &font,
        const std::string &text,
        Vector2 pos,
        float fontSize,
        Color color,
        TextAlign align
    );
    void lDrawText(
        const Font &font,
        const std::string &localizationKey,
        Vector2 pos,
        float fontSize,
        Color color,
        TextAlign align
    );
    // for text that is reusing rendering data
    void mhDrawText(
        const TextDisplay &data,
        const std::string &text
    );
    void lDrawText(
        const TextDisplay &data,
        const std::string &localizationKey
    );
    float TextWidth(const TextDisplay &data, const std::string& text);
    float lTextWidth(const TextDisplay &data, const std::string& localeKey);
    static bool FirstMainMenuBoot = true;

    static bool streamsLoaded = false;
    void DrawFPS(int posX, int posY);
    void DrawVersion();

    void DrawAlbumArtBackground();
}

class MainMenu : public OvershellMenu {
    
    void ChooseSplashText(std::filesystem::path directory);
    void AttractScreen();
    void GotoSongSelect();
    void MainMenuScreen();
public:
    // MainMenu() {};
    // drawing helper functions for other menus

    bool hehe = false;
    bool shouldBreak = false;
    Encore::ButtonActionRegistry buttReg;
    MainMenu() {}
    ~MainMenu() {}
    void Draw();
    void Load();
    void KeyboardInputCallback(int key, int scancode, int action, int mods);
    void ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event);
    std::string SplashString;
    float TitleAnimTimer = 1;
    int ControllerSelected = 0;
    bool songsLoaded = false;
    bool streamsPaused = false;
    bool stringChosen = false;
    bool albumArtLoaded = false;
    void loadMainMenu();
    inline void loadTitleScreen() {};

    bool songChosen = false;
};


