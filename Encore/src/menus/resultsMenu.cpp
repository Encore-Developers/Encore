//
// Created by marie on 14/09/2024.
//

#include "resultsMenu.h"
#include "gameMenu.h"
#include "overshellRenderer.h"
#include "raygui.h"
#include "uiUnits.h"
#include "song/audio.h"

resultsMenu::~resultsMenu() {}
resultsMenu::resultsMenu() {}

void resultsMenu::Load() {
    std::filesystem::path assetsdir = GetApplicationDirectory();
    assetsdir /= "Assets";
    RedHatDisplayItalic = GameMenu::LoadFontFilter(
        assetsdir / "fonts/RedHatDisplay-BlackItalic.ttf"
    ); // large song text
    RubikItalic = GameMenu::LoadFontFilter(assetsdir / "fonts/Rubik-Italic.ttf"); // artist
                                                                                  // name
                                                                                  // text
    RubikBoldItalic =
        GameMenu::LoadFontFilter(assetsdir / "fonts/Rubik-BoldItalic.ttf"); // praise text
    Rubik = GameMenu::LoadFontFilter(assetsdir / "fonts/Rubik-Regular.ttf"); // performance
                                                                             // text
    RubikBold = GameMenu::LoadFontFilter(assetsdir / "fonts/Rubik-Bold.ttf"); // instrument/difficulty
                                                                              // text
    JosefinSansItalic =
        GameMenu::LoadFontFilter(assetsdir / "fonts/JosefinSans-Italic.ttf"); // extra
                                                                              // information
                                                                              // text
    GoldStar = GameMenu::LoadTextureFilter(assetsdir / "ui/gold-star.png");
    Star = GameMenu::LoadTextureFilter(assetsdir / "ui/star.png");
    ;
    EmptyStar = GameMenu::LoadTextureFilter(assetsdir / "ui/empty-star.png");
    ;
    songPartsList = { "Drums",         "Bass",         "Guitar",       "Vocals",
                      "Classic Drums", "Classic Bass", "Classic Lead", "Classic Vocals",
                      "Keys",          "Classic Keys" };
    diffList = { "Easy", "Medium", "Hard", "Expert" };
    sdfShader = LoadShader(0, (assetsdir / "fonts/sdf.fs").string().c_str());
}

void resultsMenu::Draw() {
    PlayerManager &player_manager = PlayerManager::getInstance();
    SongList &songList = SongList::getInstance();
    Units &u = Units::getInstance();

    GameMenu::DrawAlbumArtBackground(songList.curSong->albumArtBlur);

    for (int i = 0; i < player_manager.PlayersActive; i++) {
        drawPlayerResults(player_manager.GetActivePlayer(i), *songList.curSong, i);
    }

    GameMenu::DrawTopOvershell(0.2f);
    GameMenu::DrawBottomOvershell();
    GameMenu::DrawBottomBottomOvershell();
    OvershellRenderer osr;
    osr.DrawOvershell();
    GameMenu::DrawVersion();

    float scoreWidth =
        MeasureTextEx(
            RedHatDisplayItalic,
            GameMenu::scoreCommaFormatter(player_manager.BandStats.Score).c_str(),
            u.hinpct(0.06f),
            0
        )
            .x;

    DrawTextEx(
        RedHatDisplayItalic,
        songList.curSong->title.c_str(),
        { u.LeftSide, u.hpct(0.02125f) },
        u.hinpct(0.05f),
        0,
        WHITE
    );
    DrawTextEx(
        RubikItalic,
        songList.curSong->artist.c_str(),
        { u.LeftSide, u.hpct(0.07f) },
        u.hinpct(0.035f),
        0,
        WHITE
    );
    DrawTextEx(
        RedHatDisplayItalic,
        GameMenu::scoreCommaFormatter(player_manager.BandStats.Score).c_str(),
        { u.LeftSide, u.hpct(0.1f) },
        u.hinpct(0.06f),
        0,
        GetColor(0x00adffFF)
    );
    DrawTextEx(
        RedHatDisplayItalic, "!", { u.LeftSide, u.hpct(0.1525f) }, u.hinpct(0.05f), 0, RED
    );
    DrawTextEx(
        JosefinSansItalic,
        "  Scoring is disabled in indev builds",
        { u.LeftSide, u.hpct(0.158f) },
        u.hinpct(0.025f),
        0.125,
        WHITE
    );
    renderStars(
        &player_manager.BandStats,
        scoreWidth + u.LeftSide + u.winpct(0.01f),
        u.hpct(0.105f),
        u.hinpct(0.05f),
        true
    );
    // assets.DrawTextRHDI(player->songToBeJudged.title.c_str(),songNamePos, 50, WHITE);
    if (GuiButton({ 0, 0, 60, 60 }, "<")) {
        // player.quit = false;
        for (int PlayersToReset = 0; PlayersToReset < player_manager.PlayersActive;
             PlayersToReset++) {
            Player *player = player_manager.GetActivePlayer(PlayersToReset);
            player->ResetGameplayStats();
            songList.curSong->parts[player->Instrument]
                ->charts[player->Difficulty]
                .resetNotes();
        }
        player_manager.BandStats.ResetBandGameplayStats();
        TheGameMenu.SwitchScreen(SONG_SELECT);
    }
}

void resultsMenu::drawPlayerResults(Player *player, Song song, int playerslot) {
    Units &u = Units::getInstance();
    float cardPos = u.LeftSide + (u.winpct(0.26f) * ((float)playerslot));

    DrawRectangle(cardPos - 6, u.hpct(0.2f), u.winpct(0.22f) + 12, u.hpct(0.85f), WHITE);
    DrawRectangle(
        cardPos, u.hpct(0.2f), u.winpct(0.22f), u.hpct(0.85f), GetColor(0x181827FF)
    );

    DrawRectangleGradientV(
        cardPos,
        u.hpct(0.2f),
        u.winpct(0.22f),
        u.hinpct(0.2f),
        ColorBrightness(player->AccentColor, -0.5f),
        GetColor(0x181827FF)
    );

    bool rendAsFC = player->stats->FC && !player->stats->Quit && !player->Bot;
    if (player->Bot) {
        DrawRectangleGradientV(
            cardPos,
            u.hpct(0.2f) + u.hinpct(0.2f),
            u.winpct(0.22f),
            u.hinpct(0.63f),
            GetColor(0x181827FF),
            ColorContrast(ColorBrightness(SKYBLUE, -0.5f), -0.25f)
        );
    }
    if (player->stats->Quit && !player->Bot) {
        DrawRectangleGradientV(
            cardPos,
            u.hpct(0.2f) + u.hinpct(0.2f),
            u.winpct(0.22f),
            u.hinpct(0.63f),
            GetColor(0x181827FF),
            ColorBrightness(RED, -0.5f)
        );
    }
    if (rendAsFC && !player->Bot) {
        DrawRectangleGradientV(
            cardPos,
            u.hpct(0.2f) + u.hinpct(0.2f),
            u.winpct(0.22f),
            u.hinpct(0.63f),
            GetColor(0x181827FF),
            ColorContrast(ColorBrightness(GOLD, -0.5f), -0.25f)
        );
    }
    if (player->stats->PerfectHit == player->stats->Notes && rendAsFC && !player->Bot) {
        DrawRectangleGradientV(
            cardPos,
            u.hpct(0.2f) + u.hinpct(0.2f),
            u.winpct(0.22f),
            u.hinpct(0.63f),
            GetColor(0x181827FF),
            ColorBrightness(WHITE, -0.5f)
        );
    }

    DrawLine(
        cardPos,
        u.hpct(0.2f) + u.hinpct(0.2f),
        cardPos + u.winpct(0.22f),
        u.hpct(0.2f) + u.hinpct(0.2f),
        WHITE
    );
    DrawLine(
        cardPos,
        u.hpct(0.2f) + u.hinpct(0.4f),
        cardPos + u.winpct(0.22f),
        u.hpct(0.2f) + u.hinpct(0.4f),
        WHITE
    );

    float scorePos = (cardPos + u.winpct(0.11f))
        - (MeasureTextEx(
               RedHatDisplayItalic,
               GameMenu::scoreCommaFormatter(player->stats->Score).c_str(),
               u.hinpct(0.065f),
               0
           )
               .x
           / 2);
    float Percent =
        floorf(((float)player->stats->NotesHit / (float)player->stats->Notes) * 100.0f);

    DrawTextEx(
        RedHatDisplayItalic,
        GameMenu::scoreCommaFormatter(player->stats->Score).c_str(),
        { scorePos, (float)GetScreenHeight() / 2 },
        u.hinpct(0.065f),
        0,
        GetColor(0x00adffFF)
    );

    renderStars(
        player->stats,
        (cardPos + u.winpct(0.11f)),
        (float)GetScreenHeight() / 2 - u.hinpct(0.06f),
        u.hinpct(0.055f),
        false
    );

    if (rendAsFC) {
        DrawTextEx(
            RedHatDisplayItalic,
            TextFormat("%3.0f%%", Percent),
            { (cardPos + u.winpct(0.113f))
                  - (MeasureTextEx(
                         RedHatDisplayItalic,
                         TextFormat("%3.0f", Percent),
                         u.hinpct(0.1f),
                         0
                     )
                         .x
                     / 1.5f),
              u.hpct(0.243f) },
            u.hinpct(0.1f),
            0,
            ColorBrightness(GOLD, -0.5)
        );
        float flawlessFontSize = 0.03f;
        DrawTextEx(
            RubikBoldItalic,
            "Flawless!",
            { (cardPos + u.winpct(0.113f))
                  - (MeasureTextEx(
                         RubikBoldItalic, "Flawless!", u.hinpct(flawlessFontSize), 0.0f
                     )
                         .x
                     / 2),
              u.hpct(0.35f) },
            u.hinpct(flawlessFontSize),
            0.0f,
            WHITE
        );
    }
    if (player->stats->Quit && !player->Bot) {
        float flawlessFontSize = 0.05f;
        DrawTextEx(
            RubikBoldItalic,
            "Quit",
            { (cardPos + u.winpct(0.11f))
                  - (MeasureTextEx(
                         RubikBoldItalic, "Quit", u.hinpct(flawlessFontSize), 0.0f
                     )
                         .x
                     / 2),
              u.hpct(0.35f) },
            u.hinpct(flawlessFontSize),
            0.0f,
            RED
        );
    }
    if (player->Bot) {
        float flawlessFontSize = 0.05f;
        DrawTextEx(
            RubikBoldItalic,
            "BOT",
            { (cardPos + u.winpct(0.11f))
                  - (MeasureTextEx(
                         RubikBoldItalic, "BOT", u.hinpct(flawlessFontSize), 0.0f
                     )
                         .x
                     / 2),
              u.hpct(0.35f) },
            u.hinpct(flawlessFontSize),
            0.0f,
            SKYBLUE
        );
    }
    DrawTextEx(
        RedHatDisplayItalic,
        TextFormat("%3.0f%%", Percent),
        { (cardPos + u.winpct(0.11f))
              - (MeasureTextEx(
                     RedHatDisplayItalic,
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
        diffList[player->Difficulty].c_str(),
        songPartsList[player->Instrument].c_str()
    );
    float InstDiffPos =
        MeasureTextEx(RubikBold, InstDiffName.c_str(), u.hinpct(0.03f), 0).x;
    float pctSize =
        MeasureTextEx(RubikBold, TextFormat("%3.0f%%", Percent), u.hinpct(0.1f), 0).y;

    DrawTextEx(
        RubikBold,
        InstDiffName.c_str(),
        { cardPos + u.winpct(0.11f) - (InstDiffPos / 2), u.hpct(0.24f) + (pctSize / 2) },
        u.hinpct(0.03f),
        0,
        WHITE
    );

    float statsHeight = u.hpct(0.2f) + u.hinpct(0.415f);
    float statsLeft = cardPos + u.winpct(0.01f);
    float statsRight = cardPos + u.winpct(0.21f);

    DrawTextEx(Rubik, "Perfects:", { statsLeft, statsHeight }, u.hinpct(0.03f), 0, WHITE);
    DrawTextEx(
        Rubik,
        "Goods:",
        { statsLeft, statsHeight + u.hinpct(0.035f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        Rubik,
        "Missed:",
        { statsLeft, statsHeight + u.hinpct(0.07f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        Rubik,
        "Strikes:",
        { statsLeft, statsHeight + u.hinpct(0.105f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        Rubik,
        "Max Streak:",
        { statsLeft, statsHeight + u.hinpct(0.14f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        Rubik,
        "Notes:",
        { statsLeft, statsHeight + u.hinpct(0.175f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );

    float hitpct = ((float)player->stats->PerfectHit / (float)player->stats->Notes);
    float pHitPercent = floorf(hitpct * 100.0f);
    std::string PerfectDisplay =
        TextFormat("%01i (%3.0f%%)", player->stats->PerfectHit, pHitPercent);

    float gpct =
        ((float)(player->stats->NotesHit - player->stats->PerfectHit)
         / (float)player->stats->Notes);
    float gHitPercent = floorf(gpct * 100.0f);
    std::string GoodDisplay = TextFormat(
        "%01i (%3.0f%%)", player->stats->NotesHit - player->stats->PerfectHit, gHitPercent
    );

    float mpct = ((float)player->stats->NotesMissed / (float)player->stats->Notes);
    float mHitPercent = floorf(mpct * 100.0f);
    std::string MissDisplay =
        TextFormat("%01i (%3.0f%%)", player->stats->NotesMissed, mHitPercent);

    std::string NotesDisplay = TextFormat("%01i", player->stats->Notes);

    int MaxNotes =
        song.parts[player->Instrument]->charts[player->Difficulty].notes.size();
    float FontSize = u.hinpct(0.03f);
    DrawTextEx(
        Rubik,
        PerfectDisplay.c_str(),
        { statsRight - MeasureTextEx(Rubik, PerfectDisplay.c_str(), FontSize, 0).x,
          statsHeight },
        FontSize,
        0,
        WHITE
    );
    DrawTextEx(
        Rubik,
        GoodDisplay.c_str(),
        { statsRight - MeasureTextEx(Rubik, GoodDisplay.c_str(), FontSize, 0).x,
          statsHeight + u.hinpct(0.035f) },
        FontSize,
        0,
        WHITE
    );
    DrawTextEx(
        Rubik,
        MissDisplay.c_str(),
        { statsRight - MeasureTextEx(Rubik, MissDisplay.c_str(), FontSize, 0).x,
          statsHeight + u.hinpct(0.07f) },
        FontSize,
        0,
        WHITE
    );
    DrawTextEx(
        Rubik,
        TextFormat("%01i", player->stats->Overhits, player->stats->Notes),
        { statsRight
              - MeasureTextEx(
                    Rubik, TextFormat("%01i", player->stats->Overhits), u.hinpct(0.03f), 0
              )
                    .x,
          statsHeight + u.hinpct(0.105f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        Rubik,
        TextFormat("%01i/%01i", player->stats->MaxCombo, player->stats->Notes),
        { statsRight
              - MeasureTextEx(
                    Rubik,
                    TextFormat("%01i/%01i", player->stats->MaxCombo, player->stats->Notes),
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
        Rubik,
        NotesDisplay.c_str(),
        { statsRight - MeasureTextEx(Rubik, NotesDisplay.c_str(), u.hinpct(0.03f), 0).x,
          statsHeight + u.hinpct(0.17f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    // DrawTextEx(rubik, TextFormat("%2.2f", player->totalOffset / player->notesHit),
    // {statsRight - MeasureTextEx(rubik, TextFormat("%2.2f", player->totalOffset /
    // player->notesHit), u.hinpct(0.03f), 0).x, statsHeight+u.hinpct(0.17f)},
    // u.hinpct(0.03f),0,WHITE);
};

void resultsMenu::renderStars(
    PlayerGameplayStats *stats, float xPos, float yPos, float scale, bool left
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
            stats->GoldStars ? GoldStar : Star,
            { 0, 0, (float)EmptyStar.width, (float)EmptyStar.height },
            { (xPos + (i * scale) - starX), yPos, scale, scale },
            { 0, 0 },
            0,
            WHITE
        );
    }
};

void resultsMenu::renderStars(
    BandGameplayStats *stats, float xPos, float yPos, float scale, bool left
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
            stats->GoldStars ? GoldStar : Star,
            { 0, 0, (float)EmptyStar.width, (float)EmptyStar.height },
            { (xPos + (i * scale) - starX), yPos, scale, scale },
            { 0, 0 },
            0,
            WHITE
        );
    }
};