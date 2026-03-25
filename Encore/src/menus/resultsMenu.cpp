//
// Created by marie on 14/09/2024.
//

#include "resultsMenu.h"
#include "gameMenu.h"
#include "raygui.h"
#include "styles.h"
#include "uiUnits.h"
#include "song/audio.h"
#include "users/playerManager.h"
#include "OvershellHelper.h"
#include "MenuManager.h"

void resultsMenu::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {}
void resultsMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {}

resultsMenu::~resultsMenu() {
    // for (int playerNum = 0; playerNum < ThePlayerManager.PlayersActive; playerNum++) {
    //     delete ThePlayerManager.GetActivePlayer(playerNum).stats;
    // }
}
resultsMenu::resultsMenu() {}

int FinalScore = 0;

void resultsMenu::Load() {
    FinalScore = 0;
    std::filesystem::path assetsdir = GetApplicationDirectory();
    assetsdir /= "Assets";
    /*
    redHatDisplayItalic = GameMenu::LoadFontFilter(
        assetsdir / "fonts/RedHatDisplay-BlackItalic.ttf"
    ); // large song text
    RubikItalic = GameMenu::LoadFontFilter(assetsdir / "fonts/assets.rubik-Italic.ttf"); // artist
                                                                                  // name
                                                                                  // text
    RubikBoldItalic =
        GameMenu::LoadFontFilter(assetsdir / "fonts/assets.rubik-BoldItalic.ttf"); // praise text
    assets.rubik = GameMenu::LoadFontFilter(assetsdir / "fonts/assets.rubik-Regular.ttf"); // performance
                                                                             // text
    RubikBold = GameMenu::LoadFontFilter(assetsdir / "fonts/assets.rubik-Bold.ttf"); // instrument/difficulty
                                                                              // text
    assets.josefinSansItalic =
        GameMenu::LoadFontFilter(assetsdir / "fonts/JosefinSans-Italic.ttf"); // extra
                                                                              // information
                                                                              // text
                                                                              */
    GoldStar = GameMenu::LoadTextureFilter(assetsdir / "ui/gold-star.png");
    Star = GameMenu::LoadTextureFilter(assetsdir / "ui/star.png");
    EmptyStar = GameMenu::LoadTextureFilter(assetsdir / "ui/empty-star.png");
    diffList = { "Easy", "Medium", "Hard", "Expert" };
    sdfShader = LoadShader(0, (assetsdir / "fonts/sdf.fs").string().c_str());
    // std::cout << "Band Score: " << ThePlayerManager.BandStats->Score << std::endl;
    // std::cout << "Band Sustain Score: " << ThePlayerManager.BandStats->SustainScore <<
    // std::endl;
    // std::cout << "Band Multiplier Score: " <<
    // ThePlayerManager.BandStats->MultiplierScore << std::endl;
    // std::cout << "Band Overdrive Score: " << ThePlayerManager.BandStats->OverdriveScore
    // << std::endl; std::cout << "Band Perfect Score: " <<
    // ThePlayerManager.BandStats->PerfectScore << std::endl; std::cout << "Band Note
    // Score: " << ThePlayerManager.BandStats->NoteScore << std::endl;

    for (int playerInt = 0; playerInt < 4; playerInt++) {
        if (ThePlayerManager.ActivePlayers[playerInt] == -1)
            continue;
        Player &player = ThePlayerManager.GetActivePlayer(playerInt);
        FinalScore += player.engine->stats->Score;
        //   PlayerGameplayStats *&stats =
        //   ThePlayerManager.GetActivePlayer(playerNum).stats; std::cout << player.Name
        //   << " Score: " << stats->Score << std::endl; std::cout << player.Name << "
        //   Sustain Score: " << stats->SustainScore << std::endl; std::cout <<
        //   player.Name << " Multiplier Score: " << stats->MultiplierScore << std::endl;
        //   std::cout << player.Name << " Overdrive Score: " << stats->OverdriveScore <<
        //   std::endl; std::cout << player.Name << " Perfect Score: " <<
        //   stats->PerfectScore << std::endl; std::cout << player.Name << " Note Score: "
        //   << stats->NoteScore << std::endl;
    }
}

void resultsMenu::Draw() {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    GameMenu::DrawAlbumArtBackground(TheSongList.curSong->albumArtBlur);
    for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
        drawPlayerResults(ThePlayerManager.GetActivePlayer(i),  i);
    }
    encOS::DrawTopOvershell(0.2f);
    GameMenu::DrawVersion();
    // Draw album cover to the left of track info
    float albumSize = u.hinpct(0.15f);
    float albumX = u.LeftSide;
    float albumY = u.hpct(0.0225f);
    DrawTexturePro(
        TheSongList.curSong->albumArt,
        { 0, 0, (float)TheSongList.curSong->albumArt.width, (float)TheSongList.curSong->albumArt.height },
        { albumX, albumY, albumSize, albumSize },
        { 0, 0 },
        0,
        WHITE
    );
    // Shift track info text to the right of the album cover
    float textX = u.LeftSide + albumSize + u.winpct(0.01f);
    DrawTextEx(
        assets.redHatDisplayItalic,
        TheSongList.curSong->title.c_str(),
        { textX, u.hpct(0.0225f) },
        u.hinpct(0.05f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubikItalic,
        TheSongList.curSong->artist.c_str(),
        { textX, u.hpct(0.075f) },
        u.hinpct(0.035f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubikItalic,
        TheSongList.curSong->charters[0].c_str(),
        { textX, u.hpct(0.115f) },
        u.hinpct(0.035f),
        0,
        WHITE
    );
    Color accentColor =
        ColorBrightness(ColorContrast(RED, -0.125f), -0.25f);
    std::string indevWarnText = "Scoring is disabled in indev builds";
    float warningTextSize = u.hinpct(0.038f);
    float textWidth = MeasureTextEx(assets.josefinSansItalic, indevWarnText.c_str(), warningTextSize, 0).x;
    Vector2 TopLeft = { u.LeftSide, u.hpct(0.158f) };
    DrawRectangle(0, TopLeft.y, u.LeftSide, warningTextSize, accentColor);

    DrawRectangleGradientH(
        TopLeft.x,
        TopLeft.y,
        textWidth + u.winpct(0.1f),
        warningTextSize,
        accentColor,
        Color { 0, 0, 0, 0 }
    );
    GameMenu::mhDrawText(
        assets.josefinSansItalic,
        indevWarnText.c_str(),
        { TopLeft.x, TopLeft.y + u.hinpct(0.008f) },
        u.hinpct(0.025f),
        WHITE,
        sdfShader,
        0
    );

    // renderStars(ThePlayerManager.BandStats, u.wpct(0.5f), u.hpct(0.1f),
    // u.hinpct(0.05f), false);
    float ScoreFontSize = u.hinpct(0.075f);
    std::string ScoreText =
    GameMenu::scoreCommaFormatter(ThePlayerManager.GetActivePlayer(0).engine->stats->Score).c_str();
    GameMenu::mhDrawText(
        assets.redHatDisplayItalic,
        GameMenu::scoreCommaFormatter(FinalScore).c_str(),
         { u.wpct(0.5), u.hpct(0.02125f) },
         ScoreFontSize,
        GetColor(0x00adffFF),
        sdfShader,
        CENTER
    );

    if (GuiButton({ 0, 0, 60, 60 }, "<")) {
        // delete ThePlayerManager.BandStats;
        for (int i = 0; i < ThePlayerManager.PlayersActive; i++) {
            Player &player = ThePlayerManager.GetActivePlayer(i);
            player.engine->stats.reset();
            player.engine->chart.reset();
            player.engine.reset();
        }
        //for (int PlayersToReset = 0; PlayersToReset < ThePlayerManager.PlayersActive; PlayersToReset++) {
        //    Player &player = ThePlayerManager.GetActivePlayer(PlayersToReset);
        //    player.ResetGameplayStats();
            // TheSongList.curSong->parts[player.Instrument]
                // ->charts[player.Difficulty]
                // .resetNotes();
        //}
        TheMenuManager.SwitchScreen(SONG_SELECT);
    }
    DrawOvershell();
}

void resultsMenu::drawPlayerResults(Player &player, int playerslot) {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    auto& stats = player.engine->stats;
    float cardPos =
            (u.wpct(0.125) + (u.winpct(0.25) * playerslot)) - u.winpct(0.11);
    // float cardPos = u.LeftSide + (u.winpct(0.26f) * ((float)playerslot));
    float cardWidth = u.winpct(0.22f);
    float cardHalfWidth = cardWidth/2;
    float cardTop = u.hpct(0.2f);
    DrawRectangle(cardPos - 6, cardTop, cardWidth + 12, u.hpct(0.85f), WHITE);
    DrawRectangle(
        cardPos, cardTop, cardWidth, u.hpct(0.85f), backgroundColor
    );

    DrawRectangleGradientV(
        cardPos,
        cardTop,
        cardWidth,
        u.hinpct(0.2f),
        ColorBrightness(player.AccentColor, -0.5f),
        backgroundColor
    );


    Color bottomColorForStatus = backgroundColor;
    bool rendAsFC = stats->AttemptedNotes == stats->NotesHit && !player.Bot && stats->Overhits == 0;
    if (player.Bot) {
        bottomColorForStatus = ColorContrast(ColorBrightness(SKYBLUE, -0.5f), -0.25f);
    }
    //if (player.stats->Quit && !player.Bot) {
    //    bottomColorForStatus = ColorBrightness(RED, -0.5f);
    //}
    if (rendAsFC && !player.Bot) {
        bottomColorForStatus = ColorContrast(ColorBrightness(GOLD, -0.5f), -0.25f);
    }
    if (stats->PerfectHits == stats->AttemptedNotes && rendAsFC && !player.Bot) {
        bottomColorForStatus = ColorBrightness(WHITE, -0.5f);
    }
    DrawRectangleGradientV(
                cardPos,
                cardTop + u.hinpct(0.2f),
                cardWidth,
                u.hinpct(0.63f),
                backgroundColor,
                bottomColorForStatus
            );

    DrawLine(
        cardPos,
        cardTop + u.hinpct(0.2f),
        cardPos + cardWidth,
        cardTop + u.hinpct(0.2f),
        WHITE
    );
    DrawLine(
        cardPos,
        cardTop + u.hinpct(0.4f),
        cardPos + cardWidth,
        cardTop + u.hinpct(0.4f),
        WHITE
    );
    std::string scoreString = GameMenu::scoreCommaFormatter(stats->Score);
    float scorePos = (cardPos + cardHalfWidth);
    float Percent =
       floorf(((float)stats->NotesHit / (float)stats->AttemptedNotes) * 100.0f);

    GameMenu::mhDrawText(
        assets.redHatDisplayItalic,
        scoreString,
        { scorePos, (float)GetRenderHeight() / 2 },
        u.hinpct(0.065f),
        GetColor(0x00adffFF),
        sdfShader,
        CENTER
    );

    renderPlayerStars(
    player,
        (cardPos + cardHalfWidth),
        (float)GetRenderHeight() / 2 - u.hinpct(0.06f),
        u.hinpct(0.055f),
        false
    );

    std::string ImportantInfoText;
    Color ImportantInfoTextColor = WHITE;
    if (rendAsFC) {
        ImportantInfoText = "Flawless!";
    }
    //if (player.stats->Quit && !player.Bot) {
    //    ImportantInfoText = "Quit";
    //    ImportantInfoTextColor = RED;
   // }
    if (player.Bot) {
        ImportantInfoText = "BOT";
        ImportantInfoTextColor = SKYBLUE;
    }
    GameMenu::mhDrawText(
        assets.rubikBold,
        ImportantInfoText,
        {cardPos + cardHalfWidth, u.hpct(0.32f)},
        SmallHeader,
        ImportantInfoTextColor,
        sdfShader,
        CENTER
    );
    DrawTextEx(
        assets.redHatDisplayItalic,
        TextFormat("%3.0f%%", Percent),
        { (cardPos + cardHalfWidth)
              - (MeasureTextEx(
                     assets.redHatDisplayItalic,
                     rendAsFC ? TextFormat("%3.0f", Percent)
                              : TextFormat("%3.0f%%", Percent),
                     u.hinpct(0.1f),
                     0
                 )
                     .x
                 / (rendAsFC ? 1.5f : 2.0f)),
          u.hpct(0.22f) },
        u.hinpct(0.1f),
        0,
        rendAsFC ? YELLOW : WHITE
    );

    std::string InstDiffName = TextFormat(
        "%s %s",
        diffList[player.Difficulty].c_str(),
        songPartsList[player.Instrument].c_str()
    );
    float InstDiffPos =
        MeasureTextEx(assets.rubikBold, InstDiffName.c_str(), u.hinpct(0.03f), 0).x;
    float pctSize =
        MeasureTextEx(assets.rubikBold, TextFormat("%3.0f%%", Percent), u.hinpct(0.1f),
    0).y;

    DrawTextEx(
        assets.rubikBold,
        InstDiffName.c_str(),
        { cardPos + cardHalfWidth - (InstDiffPos / 2), u.hpct(0.24f) + (pctSize / 2) },
        u.hinpct(0.03f),
        0,
        WHITE
    );

    float statsHeight = cardTop + u.hinpct(0.415f);
    float statsLeft = cardPos + u.winpct(0.01f);
    float statsRight = cardPos + u.winpct(0.21f);

    DrawTextEx(assets.rubik, "Perfects:", { statsLeft, statsHeight }, u.hinpct(0.03f), 0,
    WHITE); DrawTextEx( assets.rubik, "Goods:", { statsLeft, statsHeight +
    u.hinpct(0.035f) }, u.hinpct(0.03f), 0, WHITE
    );
    DrawTextEx(
        assets.rubik,
        "Missed:",
        { statsLeft, statsHeight + u.hinpct(0.07f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        "Strikes:",
        { statsLeft, statsHeight + u.hinpct(0.105f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        "Max Streak:",
        { statsLeft, statsHeight + u.hinpct(0.14f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        "Notes:",
        { statsLeft, statsHeight + u.hinpct(0.175f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );

    float hitpct = ((float)stats->PerfectHits / (float)stats->AttemptedNotes);
    float pHitPercent = floorf(hitpct * 100.0f);
    std::string PerfectDisplay =
        TextFormat("%01i (%3.0f%%)", stats->PerfectHits, pHitPercent);

    float gpct =
        ((float)(stats->NotesHit - stats->PerfectHits)
         / (float)stats->AttemptedNotes);
    float gHitPercent = floorf(gpct * 100.0f);
    std::string GoodDisplay = TextFormat(
        "%01i (%3.0f%%)", stats->NotesHit - stats->PerfectHits, gHitPercent
    );

    float mpct = ((float)stats->Misses / (float)stats->AttemptedNotes);
    float mHitPercent = floorf(mpct * 100.0f);
    std::string MissDisplay =
        TextFormat("%01i (%3.0f%%)", stats->Misses, mHitPercent);

    std::string NotesDisplay = TextFormat("%01i", stats->AttemptedNotes);

    // int MaxNotes =
    //    song.parts[player.Instrument]->charts[player.Difficulty].notes.size();
    float FontSize = u.hinpct(0.03f);
    DrawTextEx(
        assets.rubik,
        PerfectDisplay.c_str(),
        { statsRight - MeasureTextEx(assets.rubik, PerfectDisplay.c_str(), FontSize, 0).x,
          statsHeight },
        FontSize,
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        GoodDisplay.c_str(),
        { statsRight - MeasureTextEx(assets.rubik, GoodDisplay.c_str(), FontSize, 0).x,
          statsHeight + u.hinpct(0.035f) },
        FontSize,
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        MissDisplay.c_str(),
        { statsRight - MeasureTextEx(assets.rubik, MissDisplay.c_str(), FontSize, 0).x,
          statsHeight + u.hinpct(0.07f) },
        FontSize,
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        TextFormat("%01i", stats->Overhits, stats->AttemptedNotes),
        { statsRight
              - MeasureTextEx(
                    assets.rubik, TextFormat("%01i", stats->Overhits),
    u.hinpct(0.03f), 0
              )
                    .x,
          statsHeight + u.hinpct(0.105f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        TextFormat("%01i/%01i", stats->MaxCombo, stats->AttemptedNotes),
        { statsRight
              - MeasureTextEx(
                    assets.rubik,
                    TextFormat("%01i/%01i", stats->MaxCombo, stats->AttemptedNotes),
                    u.hinpct(0.03f),
                    0
              )
                    .x,
          statsHeight + u.hinpct(0.14f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        NotesDisplay.c_str(),
        { statsRight - MeasureTextEx(assets.rubik, NotesDisplay.c_str(), u.hinpct(0.03f),
    0).x, statsHeight + u.hinpct(0.17f) }, u.hinpct(0.03f), 0, WHITE
    );
    // DrawTextEx(rubik, TextFormat("%2.2f", player.totalOffset / player.notesHit),
    // {statsRight - MeasureTextEx(rubik, TextFormat("%2.2f", player.totalOffset /
    // player.notesHit), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.17f)},
    // u.hinpct(0.03f),0,WHITE);
};


void resultsMenu::renderPlayerStars(
    Player &player, float xPos, float yPos, float scale, bool left
) {
    int starsval = player.engine->stats->Stars;

    float starX = left ? 0 : scale * 2.5f;
    for (int i = 0; i < 5; i++) {
        DrawTexturePro(
            EmptyStar,
            { 0, 0, (float)EmptyStar.width, (float)EmptyStar.height },
            { (xPos + (i * scale) - starX), yPos, scale, scale },
            { 0, 0 },
            0,
            WHITE
        );
    }
    for (int i = 0; i < starsval; i++) {

        DrawTexturePro(
            starsval > 4 ? GoldStar : Star,
            { 0, 0, (float)EmptyStar.width, (float)EmptyStar.height },
            { (xPos + ((i) * scale) - starX), yPos, scale, scale },
            { 0, 0 },
            0,
            WHITE
        );
    }
};
/*
void resultsMenu::renderStars(
    BandGameplayStats *&stats, float xPos, float yPos, float scale, bool left
) {
    int starsval = stats->Stars();

    float starX = left ? 0 : scale * 2.5f;
    for (int i = 0; i < 5; i++) {
        DrawTexturePro(
            EmptyStar,
            { 0, 0, (float)EmptyStar.width, (float)EmptyStar.height },
            { (xPos + (i * scale) - starX), yPos, scale, scale },
            { 0, 0 },
            0,
            WHITE
        );
    }
    for (int i = 0; i < starsval; i++) {
        DrawTexturePro(
            stats->GoldStars() ? GoldStar : Star,
            { 0, 0, (float)EmptyStar.width, (float)EmptyStar.height },
            { (xPos + (i * scale) - starX), yPos, scale, scale },
            { 0, 0 },
            0,
            WHITE
        );
    }
};
*/