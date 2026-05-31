//
// Created by marie on 14/09/2024.
//

#include "resultsMenu.h"
#include "../main/MainMenu.h"
#include "raygui.h"
#include "../styles.h"
#include "../uiUnits.h"
#include "users/playerManager.h"
#include "../overshell/OvershellHelper.h"
#include "../MenuManager.h"
#include "menus/locale/Locale.h"
#include "menus/main/SongSelectMenu.h"
#include "song/ArtLoader.h"
#include "song/OpenSource.h"

void resultsMenu::ControllerInputCallback(Encore::RhythmEngine::ControllerEvent event) {
    buttReg.HandleInput(event);
}
void resultsMenu::KeyboardInputCallback(int key, int scancode, int action, int mods) {}

resultsMenu::~resultsMenu() {
    // for (int playerNum = 0; playerNum < ThePlayerManager.PlayersActive; playerNum++) {
    //     delete ThePlayerManager.GetActivePlayer(playerNum).stats;
    // }
}
resultsMenu::resultsMenu() {}

int FinalScore = 0;

void resultsMenu::Load() {
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg, LANE_1, "resultsMenu.back", {
        if (_action != Encore::RhythmEngine::Action::PRESS) return;
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (ThePlayerManager.ActivePlayers[i] == -1) continue;
            Player &player = ThePlayerManager.GetActivePlayer(i);
            player.engine->stats.reset();
            player.engine->chart.reset();
            player.engine.reset();
        }
        TheMenuManager.CreateAndSwitchMenu<SongSelectMenu>();
    })

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
    diffList = { "diff.easy", "diff.medium", "diff.hard", "diff.expert" };
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

    for (int playerInt = 0; playerInt < MAX_PLAYERS; playerInt++) {
        if (ThePlayerManager.ActivePlayers[playerInt] == -1) continue;
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
    GameMenu::DrawAlbumArtBackground();
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (ThePlayerManager.ActivePlayers[i] == -1) continue;
        drawPlayerResults(ThePlayerManager.GetActivePlayer(i),  i);
    }
    encOS::DrawTopOvershell(0.2f);
    GameMenu::DrawVersion();
    // Draw album cover to the left of track info
    float albumX = u.LeftSide;
    float albumY = u.hpct(0.0225f);
    float albumSize = u.hpct(0.158f) - albumY;
    DrawTexturePro(
        TheArtLoader.loadedArt,
        { 0, 0, (float)TheArtLoader.loadedArt.width, (float)TheArtLoader.loadedArt.height },
        { albumX, albumY, albumSize, albumSize },
        { 0, 0 },
        0,
        WHITE
    );
    // Shift track info text to the right of the album cover
    float textX = u.LeftSide + albumSize + u.winpct(0.01f);
    float TitleFontOffset = u.hpct(0.045f);
    float TitleFontSize = u.hinpct(0.04f);
    float SecondaryFontSize = TitleFontSize * 0.75f;
    float SecondaryFontOffset = TitleFontSize * 1.2f;
    float TitleSize = MeasureTextEx(ASSET(rubikBold), TheSongList.curSong->title.c_str(), TitleFontSize, 0).x;
    Encore::Text::DrawText(ASSET(rubikBold), TheSongList.curSong->title,
                         { textX, TitleFontOffset },
                         TitleFontSize,
                         WHITE,
                         LEFT
    );
    Encore::Text::DrawText(ASSET(josefinSansBoldItalic), TheSongList.curSong->artist,
                         { textX + TitleSize + u.hinpct(0.02f), TitleFontOffset + u.hinpct(0.008f) },
                         SecondaryFontSize,
                         LIGHTGRAY,
                         LEFT
    );
    auto sourceTex = TheSourceIcons[TheSongList.curSong->source]->GetTexture();
    DrawTexturePro(sourceTex, {0,0, (float)sourceTex.width, (float)sourceTex.height},
        {textX, TitleFontOffset + TitleFontSize, TitleFontSize, TitleFontSize}, {0,0}, 0, WHITE
    );
    Encore::Text::DrawText(ASSET(josefinSansBoldItalic), TheSongList.curSong->charters[0],
                         { textX + ( TitleFontSize * 1.125f), TitleFontOffset + SecondaryFontOffset },
                         SecondaryFontSize,
                         LIGHTGRAY,
                         LEFT
    );
    Color accentColor =
        ColorBrightness(ColorContrast(RED, -0.125f), -0.25f);
    float warningTextSize = u.hinpct(0.038f);
    auto warning = LOCALIZE("resultsMenu.indevWarning");
    float textWidth = MeasureTextEx(assets.josefinSansItalic, warning, warningTextSize, 0).x;
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
    Encore::Text::lDrawText(
        assets.josefinSansItalic,
        "resultsMenu.indevWarning",
        { TopLeft.x, TopLeft.y + u.hinpct(0.008f) },
        u.hinpct(0.025f),
        WHITE,
        LEFT
    );

    // renderStars(ThePlayerManager.BandStats, u.wpct(0.5f), u.hpct(0.1f),
    // u.hinpct(0.05f), false);
    float ScoreFontSize = u.hinpct(0.075f);
    Encore::Text::DrawText(
        assets.redHatDisplayItalic,
        GameMenu::scoreCommaFormatter(FinalScore).c_str(),
        { u.RightSide, u.hpct(0.02125f) },
        ScoreFontSize,
        GetColor(0x00adffFF),
        RIGHT
    );
    GameMenu::DrawBottomOvershell();
    buttReg.DrawPrompts(isOSOpen());
    //if (GuiButton({ 0, 0, 60, 60 }, "<")) {
    //    // delete ThePlayerManager.BandStats;
    //    for (int i = 0; i < MAX_PLAYERS; i++) {
    //        if (ThePlayerManager.ActivePlayers[i] == -1) continue;
    //        Player &player = ThePlayerManager.GetActivePlayer(i);
    //        player.engine->stats.reset();
    //        player.engine->chart.reset();
    //        player.engine.reset();
    //    }
    //    //for (int PlayersToReset = 0; PlayersToReset < ThePlayerManager.PlayersActive; PlayersToReset++) {
    //    //    Player &player = ThePlayerManager.GetActivePlayer(PlayersToReset);
    //    //    player.ResetGameplayStats();
    //        // TheSongList.curSong->parts[player.Instrument]
    //            // ->charts[player.Difficulty]
    //            // .resetNotes();
    //    //}
    //    TheMenuManager.SwitchScreen(SONG_SELECT);
    //}
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

    Encore::Text::DrawText(
        assets.redHatDisplayItalic,
        scoreString,
        { scorePos, (float)GetRenderHeight() / 2 },
        u.hinpct(0.065f),
        GetColor(0x00adffFF),
        CENTER
    );

    renderPlayerStars(
    player,
        (cardPos + cardHalfWidth),
        (float)GetRenderHeight() / 2 - u.hinpct(0.06f),
        u.hinpct(0.055f),
        false
    );

    if (rendAsFC) {
        Encore::Text::lDrawText(
            assets.rubikBold,
            "resultsMenu.infoText.fc",
            {cardPos + cardHalfWidth, u.hpct(0.32f)},
            SmallHeader,
            WHITE,
            CENTER
        );
    }
    //if (player.stats->Quit && !player.Bot) {
    //    ImportantInfoText = "Quit";
    //    ImportantInfoTextColor = RED;
   // }
    if (player.Bot) {
        Encore::Text::lDrawText(
            assets.rubikBold,
            "resultsMenu.infoText.autoplay",
            {cardPos + cardHalfWidth, u.hpct(0.32f)},
            SmallHeader,
            SKYBLUE,
            CENTER
        );
    }

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
        LOCALIZE(diffList[player.Difficulty]).toChar(),
        LOCALIZE(songPartsList[player.Instrument]).toChar()
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

    DrawTextEx(assets.rubik, LOCALISE("resultsMenu.statline.perfect"), { statsLeft, statsHeight }, u.hinpct(0.03f), 0,
    WHITE); DrawTextEx( assets.rubik, LOCALISE("resultsMenu.statline.good"), { statsLeft, statsHeight +
    u.hinpct(0.035f) }, u.hinpct(0.03f), 0, WHITE
    );
    DrawTextEx(
        assets.rubik,
        LOCALISE("resultsMenu.statline.missed"),
        { statsLeft, statsHeight + u.hinpct(0.07f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        LOCALISE("resultsMenu.statline.overhit"),
        { statsLeft, statsHeight + u.hinpct(0.105f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        LOCALISE("resultsMenu.statline.streak"),
        { statsLeft, statsHeight + u.hinpct(0.14f) },
        u.hinpct(0.03f),
        0,
        WHITE
    );
    DrawTextEx(
        assets.rubik,
        LOCALISE("resultsMenu.statline.noteCount"),
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