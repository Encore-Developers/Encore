//
// Created by marie on 14/09/2024.
//

#ifndef RESULTSMENU_H
#define RESULTSMENU_H

#include "../main/MainMenu.h"
#include "../menu.h"

struct Grade {
    struct Range {
        double bottom;
        double top;
    };
    Color color = WHITE;
    const char *Letter;
    Range range = {0,0};
    explicit Grade(const Color _color, const char _letter[1], const Range _range)
        : color(_color), Letter(_letter), range(_range) {}

    // 0 is lower, 1 is normal, 2 is upper
    [[nodiscard]] int GetSubdiv(double acc) const {
        if (acc >= range.top - (range.top - range.bottom) * 0.3) return 2;
        if (acc < range.bottom + (range.bottom - range.top) * 0.3) return 0;
        return 1;
    }
};

class resultsMenu : public OvershellMenu {
    enum ResultsState {
        GENERAL,
        SECTIONS,
        HISTOGRAM
    };

    // ok assets go here
    // i forgot which assets i need.
    Texture2D GoldStar;
    Texture2D Star;
    Texture2D EmptyStar;
    Shader sdfShader;
    std::vector<std::string> diffList;
    std::array<ResultsState, MAX_PLAYERS> resultsState {GENERAL};
    std::array<int, MAX_PLAYERS> topSectList {0};
    void DrawSections(Player &player, Rectangle rect, float cardHeight, int playerslot);
    void DrawStatistics(std::shared_ptr<Encore::RhythmEngine::BaseStats> &stats, Grade curGrade, Rectangle rect, float cardHeight);
    void DrawHistogram(Player &player, Rectangle rect, float cardHeight, int playerslot);
    void drawPlayerResults(Player &player, int playerslot);
    void renderPlayerStars(
         Player &stats, float xPos, float yPos, float scale, bool left
    );
    //  void
    //  renderStars(BandGameplayStats *&stats, float xPos, float yPos, float scale, bool
    //  left);
    Encore::ButtonActionRegistry buttReg;
public:
    Song* curSong;

    resultsMenu(Song* song) : curSong(song) {}
    virtual void KeyboardInputCallback(SDL_KeyboardEvent* event);
    virtual void ControllerInputCallback(Encore::ControllerEvent event);
    //~resultsMenu() override;
    void Draw() override;
    void Load() override;

    bool AllowsTempPlayers() override { return true; };
};

#endif // RESULTSMENU_H