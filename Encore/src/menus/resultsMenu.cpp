//
// Created by marie on 14/09/2024.
//

#include "resultsMenu.h"
#include "gameMenu.h"
#include "overshellRenderer.h"
#include "raygui.h"
#include "styles.h"
#include "uiUnits.h"
#include "song/audio.h"

resultsMenu::~resultsMenu() {}
resultsMenu::resultsMenu() {}

void resultsMenu::Load() {
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
    OvershellRenderer osr;
    PlayerManager &player_manager = PlayerManager::getInstance();
    SongList &songList = SongList::getInstance();
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    GameMenu::DrawAlbumArtBackground(songList.curSong->albumArtBlur);
    
    for (int i = 0; i < player_manager.PlayersActive; i++) {
        drawPlayerResults(player_manager.GetActivePlayer(i), *songList.curSong, i);
    }
    
    osr.DrawTopOvershell(0.2f);

    GameMenu::DrawVersion();

    float scoreWidth =
        MeasureTextEx(
            assets.redHatDisplayItalic,
            GameMenu::scoreCommaFormatter(player_manager.BandStats.Score).c_str(),
            u.hinpct(0.06f),
            0
        )
            .x;

    DrawTextEx(
        assets.redHatDisplayItalic,
        songList.curSong->title.c_str(),
        { u.LeftSide, u.hpct(0.02125f) },
        u.hinpct(0.05f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubikItalic,
        songList.curSong->artist.c_str(),
        { u.LeftSide, u.hpct(0.07f) },
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
        {TopLeft.x, TopLeft.y + u.hinpct(0.008f)},
        u.hinpct(0.025f),
        WHITE,
        sdfShader,
        0
    );

    renderStars(
        &player_manager.BandStats,
        u.wpct(0.5f),
        u.hpct(0.1f),
        u.hinpct(0.05f),
        false
    );
    float ScoreFontSize = u.hinpct(0.05f);
    std::string ScoreText = GameMenu::scoreCommaFormatter(player_manager.BandStats.Score).c_str();
    GameMenu::mhDrawText(
        assets.redHatDisplayItalic,
        GameMenu::scoreCommaFormatter(player_manager.BandStats.Score).c_str(),
        { u.wpct(0.5), u.hpct(0.02125f) },
        ScoreFontSize,
        GetColor(0x00adffFF),
        sdfShader,
        CENTER
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
        songList.curSong->midiParsed = false;
        player_manager.BandStats.ResetBandGameplayStats();
        TheGameMenu.SwitchScreen(SONG_SELECT);
    }
    GameMenu::DrawBottomOvershell();

    osr.DrawBottomOvershell();
}

void resultsMenu::drawPlayerResults(Player *player, Song song, int playerslot) {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
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
        ColorBrightness(player->AccentColor, -0.5f),
        backgroundColor
    );


    Color bottomColorForStatus = backgroundColor;
    bool rendAsFC = player->stats->FC && !player->stats->Quit && !player->Bot;
    if (player->Bot) {
        bottomColorForStatus = ColorContrast(ColorBrightness(SKYBLUE, -0.5f), -0.25f);
    }
    if (player->stats->Quit && !player->Bot) {
        bottomColorForStatus = ColorBrightness(RED, -0.5f);
    }
    if (rendAsFC && !player->Bot) {
        bottomColorForStatus = ColorContrast(ColorBrightness(GOLD, -0.5f), -0.25f);
    }
    if (player->stats->PerfectHit == player->stats->Notes && rendAsFC && !player->Bot) {
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
    std::string scoreString = GameMenu::scoreCommaFormatter(player->stats->Score);
    float scorePos = (cardPos + cardHalfWidth);
    float Percent =
        floorf(((float)player->stats->NotesHit / (float)player->stats->Notes) * 100.0f);

    GameMenu::mhDrawText(
        assets.redHatDisplayItalic,
        scoreString,
        { scorePos, (float)GetScreenHeight() / 2 },
        u.hinpct(0.065f),
        GetColor(0x00adffFF),
        sdfShader,
        CENTER
    );

    renderStars(
        player->stats,
        (cardPos + cardHalfWidth),
        (float)GetScreenHeight() / 2 - u.hinpct(0.06f),
        u.hinpct(0.055f),
        false
    );

    std::string ImportantInfoText;
    Color ImportantInfoTextColor = WHITE;
    if (rendAsFC) {
        ImportantInfoText = "Flawless!";
    }
    if (player->stats->Quit && !player->Bot) {
        ImportantInfoText = "Quit";
        ImportantInfoTextColor = RED;
    }
    if (player->Bot) {
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
        diffList[player->Difficulty].c_str(),
        songPartsList[player->Instrument].c_str()
    );
    float InstDiffPos =
        MeasureTextEx(assets.rubikBold, InstDiffName.c_str(), u.hinpct(0.03f), 0).x;
    float pctSize =
        MeasureTextEx(assets.rubikBold, TextFormat("%3.0f%%", Percent), u.hinpct(0.1f), 0).y;

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

    DrawTextEx(assets.rubik, "Perfects:", { statsLeft, statsHeight }, u.hinpct(0.03f), 0, WHITE);
    DrawTextEx(
        assets.rubik,
        "Goods:",
        { statsLeft, statsHeight + u.hinpct(0.035f) },
        u.hinpct(0.03f),
        0,
        WHITE
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
        TextFormat("%01i", player->stats->Overhits, player->stats->Notes),
        { statsRight
              - MeasureTextEx(
                    assets.rubik, TextFormat("%01i", player->stats->Overhits), u.hinpct(0.03f), 0
              )
                    .x,
          statsHeight + u.hinpct(0.105f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        TextFormat("%01i/%01i", player->stats->MaxCombo, player->stats->Notes),
        { statsRight
              - MeasureTextEx(
                    assets.rubik,
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
        assets.rubik,
        NotesDisplay.c_str(),
        { statsRight - MeasureTextEx(assets.rubik, NotesDisplay.c_str(), u.hinpct(0.03f), 0).x,
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