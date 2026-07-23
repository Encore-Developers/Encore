//
// Created by marie on 14/09/2024.
//

#include "resultsMenu.h"

#include "ChartLoadingMenu.h"
#include "../main/MainMenu.h"
#include "raygui.h"
#include "ReadyUpMenu.h"
#include "../util/styles.h"
#include "../util/uiUnits.h"
#include "users/playerManager.h"
#include "../overshell/OvershellHelper.h"
#include "../MenuManager.h"
#include "menus/util/locale/Locale.h"
#include "menus/main/SongSelectMenu.h"
#include "song/ArtLoader.h"
#include "song/OpenSource.h"

void resultsMenu::ControllerInputCallback(Encore::ControllerEvent event) {
    buttReg.HandleInput(event);
}


inline int MAX_LIST_LENGTH = 7;

void resultsMenu::KeyboardInputCallback(SDL_KeyboardEvent *event) {
    if (event->down == false)
        return;
    if (event->key == SDLK_UP) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!ThePlayerManager.ActivePlayers[i])
                continue;
            switch (resultsState.at(i)) {
            case (GENERAL): {
                break;
            }
            case (SECTIONS): {
                topSectList.at(i)--;
                if (topSectList.at(i) < 0)
                    topSectList.at(i) = 0;
            }
            break;
            }
        }
    } else if (event->key == SDLK_DOWN) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!ThePlayerManager.ActivePlayers[i])
                continue;
            switch (resultsState.at(i)) {
            case (GENERAL): {
                break;
            }
            case (SECTIONS): {
                topSectList.at(i)++;
                if (topSectList.at(i) > ThePlayerManager.GetActivePlayer(i).engine->chart
                                                        ->sections.size() - 1 -
                    MAX_LIST_LENGTH + 2)
                    topSectList.at(i) = ThePlayerManager.GetActivePlayer(i).engine->chart
                                                        ->sections.size() - 1 -
                        MAX_LIST_LENGTH + 2;
            }
            break;
            }
        }
    }
}


int FinalScore = 0;

void resultsMenu::Load() {
    buttReg.buttMap.clear();
    std::string l1key = "resultsMenu.back";
    if (TheSongList.playlist.size() > 1 && TheSongList.PlaylistMode) {
        l1key = "resultsMenu.next";
    }
    NEWBUTTONACTION2(buttReg,
                     LANE_1,
                     l1key,
                     {
                     if (_action != Encore::Action::PRESS) return;
                     for (int i = 0; i < MAX_PLAYERS; i++) {
                     if (!ThePlayerManager.ActivePlayers[i]) continue;
                     Player &player = ThePlayerManager.GetActivePlayer(i);
                     player.engine->stats.reset();
                     player.engine->chart.reset();
                     player.engine.reset();
                     }
                     if (TheSongList.PlaylistMode && TheSongList.playlist.size() > 1) {
                     TheSongList.playlist.pop_front();
                     TheSongList.PlaylistIndex++;
                     TheMenuManager.CreateAndSwitchMenu<ReadyUpMenu>(TheSongList.playlist.
                         front());
                     return;
                     }

        if (TheSongList.playlist.size() == 1) {
            TheSongList.PlaylistMode = false;
            TheSongList.PlaylistSize = 0;
            TheSongList.PlaylistIndex = 0;
            TheSongList.playlist.pop_front();
        }
        TheMenuManager.CreateAndSwitchMenu<SongSelectMenu>();
    })
    NEWBUTTONACTION2(buttReg, LANE_4, "resultsMenu.sections", {
        if (_action != Encore::Action::PRESS) return;
        if (slot == -1) {
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (!ThePlayerManager.ActivePlayers[i]) continue;
                switch (resultsState.at(i)) {
                case GENERAL: resultsState.at(i) = SECTIONS; break;
                case SECTIONS: resultsState.at(i) = HISTOGRAM; break;
                case HISTOGRAM: resultsState.at(i) = GENERAL; break;
                }
            }
            return;
        }
        switch (resultsState.at(slot)) {
        case GENERAL: resultsState.at(slot) = SECTIONS; break;
        case SECTIONS: resultsState.at(slot) = HISTOGRAM; break;
        case HISTOGRAM: resultsState.at(slot) = GENERAL; break;
        }
    })
    NEWBUTTONACTION2(buttReg, LANE_3, "generic.restart", {
        if (_action != Encore::Action::PRESS) return;
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!ThePlayerManager.ActivePlayers[i]) continue;
            Player &player = ThePlayerManager.GetActivePlayer(i);
            player.engine->stats.reset();
            player.engine->chart.reset();
            player.engine.reset();
        }
        if (TheSongList.PlaylistMode) {
            TheMenuManager.CreateAndSwitchMenu<ReadyUpMenu>(TheSongList.playlist.front());
            return;
        }
        TheMenuManager.CreateAndSwitchMenu<ChartLoadingMenu>(curSong);
    })
      NEWBUTTONACTION2(buttReg, STRUM_UP, "sectionsup", {
        if (_action != Encore::Action::PRESS) return;
        switch (resultsState.at(slot)) {
        case (HISTOGRAM):
        case (GENERAL): {
            break;
        }
        case (SECTIONS): {
            topSectList.at(slot)--;
            if (topSectList.at(slot) < 0)
                topSectList.at(slot) = 0;
            }
            break;
        }
    }, false)
    NEWBUTTONACTION2(buttReg, STRUM_DOWN, "sectionsdown", {
        if (_action != Encore::Action::PRESS) return;
        switch (resultsState.at(slot)) {
        case (HISTOGRAM):
        case (GENERAL): {
            break;
        }
        case (SECTIONS): {
            topSectList.at(slot)++;
            if (topSectList.at(slot) > ThePlayerManager.GetActivePlayer(slot).engine->chart->sections.size() - 1 - MAX_LIST_LENGTH + 2)
                topSectList.at(slot) = ThePlayerManager.GetActivePlayer(slot).engine->chart->sections.size() - 1 - MAX_LIST_LENGTH + 2;
            }
            break;
        }
    }, false)

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
    GoldStar = ASSET(goldStar);
    Star = ASSET(star);
    EmptyStar = ASSET(emptyStar);
    diffList = { "diff.easy", "diff.medium", "diff.hard", "diff.expert" };
    sdfShader = ASSET(sdfShader);
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
        if (!ThePlayerManager.ActivePlayers[playerInt])
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
    GameMenu::DrawAlbumArtBackground();

    GameMenu::DrawTopOvershell(0.18f);
    GameMenu::DrawTopBarText();
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!ThePlayerManager.ActivePlayers[i])
            continue;
        drawPlayerResults(ThePlayerManager.GetActivePlayer(i), i);
    }
    // Shift track info text to the right of the album cover
    float textX = u.LeftSide + u.winpct(0.01f);
    float TitleY = u.hpct(0.05125f);
    float TitleFontSize = u.hinpct(0.04f);
    float SecondaryFontSize = TitleFontSize * 0.75f;
    float SecondaryFontOffset = TitleFontSize * 1.2f;
    Encore::TextDisplay SongTitle;
    auto sourceTex = TheSourceIcons[curSong->source]->GetTexture();
    DrawTexturePro(sourceTex,
                   { 0, 0, (float)sourceTex.width, (float)sourceTex.height },
                   { textX, TitleY, TitleFontSize, TitleFontSize },
                   { 0, 0 },
                   0,
                   WHITE
    );
    float TitleSize =
        SongTitle.Pos(textX + SecondaryFontOffset, TitleY)
                 .Fnt(ASSET(rubikBold))
                 .Size(TitleFontSize)
                 .DrawText(curSong->title)
                 .TextWidth(curSong->title);

    Encore::TextDisplay SongArtist;
    SongArtist.Pos(textX + SecondaryFontOffset + TitleSize + u.hinpct(0.02f),
                   TitleY + u.hinpct(0.008f))
              .Fnt(ASSET(josefinSansBoldItalic))
              .Size(SecondaryFontSize)
              .Col(LIGHTGRAY)
              .DrawText(curSong->artist);

    // renderStars(ThePlayerManager.BandStats, u.wpct(0.5f), u.hpct(0.1f),
    // u.hinpct(0.05f), false);
    float ScoreFontSize = u.hinpct(0.08f);
    Encore::Text::DrawText(
        assets.redHatMono,
        GameMenu::scoreCommaFormatter(FinalScore).c_str(),
        { textX, TitleY + TitleFontSize },
        ScoreFontSize,
        GetColor(0x4FC6F9FF),
        LEFT
    );
    buttReg.DrawPrompts(isOSOpen());
    //if (GuiButton({ 0, 0, 60, 60 }, "<")) {
    //    // delete ThePlayerManager.BandStats;
    //    for (int i = 0; i < MAX_PLAYERS; i++) {
    //        if (!ThePlayerManager.ActivePlayers[i]) continue;
    //        Player &player = ThePlayerManager.GetActivePlayer(i);
    //        player.engine->stats.reset();
    //        player.engine->chart.reset();
    //        player.engine.reset();
    //    }
    //    //for (int PlayersToReset = 0; PlayersToReset < ThePlayerManager.PlayersActive; PlayersToReset++) {
    //    //    Player &player = ThePlayerManager.GetActivePlayer(PlayersToReset);
    //    //    player.ResetGameplayStats();
    //        // curSong->parts[player.Instrument]
    //            // ->charts[player.Difficulty]
    //            // .resetNotes();
    //    //}
    //    TheMenuManager.SwitchScreen(SONG_SELECT);
    //}
    DrawOvershell();
}


std::array<Grade, 7> Grades{
    {
        Grade(MAGENTA, "P", { 1.00, 1.00 }),
        Grade(GOLD, "S", { 0.96, 1.00 }),
        Grade(GREEN, "A", { 0.90, 0.96 }),
        Grade(SKYBLUE, "B", { 0.85, 0.90 }),
        Grade(ORANGE, "C", { 0.77, 0.85 }),
        Grade(RED, "D", { 0.60, 0.77 }),
        Grade(backgroundColor, "F", { 0.00, 0.60 }),
    }
};

int GetGrade(double acc, Grade &grade) {
    int i = 0;
    for (const auto &g : Grades) {
        if (acc >= g.range.bottom) {
            grade = g;
            return i;
        }
        i++;
    }
    return 0;
}

void resultsMenu::DrawStatistics(std::shared_ptr<Encore::RhythmEngine::BaseStats> &stats,
                                 Grade curGrade,
                                 Rectangle rect,
                                 float cardHeight) {
    std::string grade = curGrade.Letter;
    switch (curGrade.GetSubdiv(stats->Accuracy / stats->AttemptedNotes)) {
    case 0:
        grade += "-";
        break;
    case 2:
        grade += "+";
        break;
    default:
        break;
    }
    Units &u = Units::getInstance();
    float ActualStatsHeight = u.hinpct(0.03f);
    float statsHeight = rect.y;
    float statsLeft = rect.x + u.winpct(0.01f);
    float statsRight = rect.x + rect.width - u.winpct(0.01f);

    Encore::TextDisplay Header;
    Header.Pos(rect.x + (rect.width / 2), rect.y - (cardHeight * 0.07f)).Align(CENTER).
           Fnt(ASSET(josefinSansBold)).Col(LIGHTGRAY).Size(cardHeight * 0.05f);

    Encore::TextDisplay LeftStatData;
    Encore::TextDisplay RightStatData;

    LeftStatData.Pos(statsLeft, statsHeight).Size(u.hinpct(0.025f));
    RightStatData.Pos(statsRight, statsHeight).Size(u.hinpct(0.025f)).Align(RIGHT);

    Header.lDrawText("resultsMenu.statisticsHeader");
    RightStatData.Fnt(ASSET(rubik));
    LeftStatData.lDrawText("resultsMenu.statline.perfect");
    LeftStatData.pos.y += ActualStatsHeight;
    LeftStatData.lDrawText("resultsMenu.statline.good");
    LeftStatData.pos.y += ActualStatsHeight;
    LeftStatData.lDrawText("resultsMenu.statline.missed");
    LeftStatData.pos.y += ActualStatsHeight;
    LeftStatData.lDrawText("resultsMenu.statline.overhit");
    LeftStatData.pos.y += ActualStatsHeight;
    LeftStatData.lDrawText("resultsMenu.statline.streak");
    LeftStatData.pos.y += ActualStatsHeight;
    LeftStatData.lDrawText("resultsMenu.statline.noteCount");
    LeftStatData.pos.y += ActualStatsHeight;
    LeftStatData.lDrawText("resultsMenu.statline.accRating");

    float hitpct = ((float)stats->PerfectHits / (float)stats->AttemptedNotes);
    float pHitPercent = floorf(hitpct * 100.0f);
    std::string PerfectDisplay =
        TextFormat("%01i (%3.0f%%)", stats->PerfectHits, pHitPercent);

    float gpct =
    ((float)(stats->NotesHit - stats->PerfectHits)
        / (float)stats->AttemptedNotes);
    float gHitPercent = floorf(gpct * 100.0f);
    std::string GoodDisplay = TextFormat(
        "%01i (%3.0f%%)",
        stats->NotesHit - stats->PerfectHits,
        gHitPercent
    );

    float mpct = ((float)stats->Misses / (float)stats->AttemptedNotes);
    float mHitPercent = floorf(mpct * 100.0f);
    std::string MissDisplay =
        TextFormat("%01i (%3.0f%%)", stats->Misses, mHitPercent);

    std::string NotesDisplay = TextFormat("%01i", stats->AttemptedNotes);
    std::string AccRatingDisplay = TextFormat("%2.2f",
                                              (stats->Accuracy / stats->AttemptedNotes) *
                                              100);
    // int MaxNotes =
    //    song.parts[player.Instrument]->charts[player.Difficulty].notes.size();
    RightStatData.DrawText(PerfectDisplay);
    RightStatData.pos.y += ActualStatsHeight;
    RightStatData.DrawText(GoodDisplay);
    RightStatData.pos.y += ActualStatsHeight;
    RightStatData.DrawText(MissDisplay);
    RightStatData.pos.y += ActualStatsHeight;
    RightStatData.DrawText(TextFormat("%01i", stats->Overhits, stats->AttemptedNotes));
    RightStatData.pos.y += ActualStatsHeight;
    RightStatData.DrawText(
        TextFormat("%01i/%01i", stats->MaxCombo, stats->AttemptedNotes));
    RightStatData.pos.y += ActualStatsHeight;
    RightStatData.DrawText(NotesDisplay);
    RightStatData.pos.y += ActualStatsHeight;
    RightStatData.DrawText(AccRatingDisplay);
    RightStatData.pos.x -= RightStatData.TextWidth(AccRatingDisplay) * 1.25f;
    RightStatData.Fnt(ASSET(redHatDisplayItalic)).Col(curGrade.color).DrawText(grade);
}

void resultsMenu::DrawSections(Player &player, Rectangle rect, float cardHeight, int playerslot) {
    Units &u = Units::getInstance();
    float ActualStatsHeight = u.hinpct(0.03f);
    float statsHeight = rect.y;
    float statsLeft = rect.x + u.winpct(0.01f);
    float statsRight = rect.x + rect.width - u.winpct(0.01f);

    Encore::TextDisplay Header;
    Header.Pos(rect.x + (rect.width / 2), rect.y - (cardHeight * 0.07f)).Align(CENTER).
           Fnt(ASSET(josefinSansBold)).Col(LIGHTGRAY).Size(cardHeight * 0.05f);

    Encore::TextDisplay LeftStatData;
    Encore::TextDisplay RightStatData;

    LeftStatData.Pos(statsLeft, statsHeight).Size(u.hinpct(0.025f));
    RightStatData.Pos(statsRight, statsHeight).Size(u.hinpct(0.025f)).Align(RIGHT);
    Header.lDrawText("resultsMenu.sectionHeader");
    RightStatData.Fnt(ASSET(JetBrainsMono));
    int bottom = topSectList.at(playerslot) + MAX_LIST_LENGTH > player.engine->chart->
        sections.size()
        ? player.engine->chart->sections.size()
        : topSectList.at(playerslot) + MAX_LIST_LENGTH;
    for (int i = topSectList.at(playerslot); i < bottom; i++) {
        Section &sect = player.engine->chart->sections.at(i);

        RightStatData.Col(WHITE).pos.x = statsRight;
        std::string percentage;
        if (sect.notes == 0) {
            percentage = "";
        } else {
            percentage = std::to_string(
                int((float(sect.hit) / float(sect.notes)) * 100.0f)) + "%";
        }
        RightStatData.DrawText(percentage);
        float percentWidth = RightStatData.TextWidth("0000%");
        RightStatData.pos.x -= percentWidth;

        float perfectWidth = RightStatData.TextWidth("000");
        RightStatData.Col(GOLD).DrawText(std::to_string(sect.perfects));
        RightStatData.pos.x -= perfectWidth * 1.2;

        float goodWidth = RightStatData.TextWidth("000");
        RightStatData.Col(LIGHTGRAY).DrawText(std::to_string(sect.hit - sect.perfects));
        RightStatData.pos.x -= goodWidth * 1.125;

        float overhitWidth = 0;
        if (sect.hit == sect.notes && sect.overhits > 0) {
            RightStatData.Col(GRAY).DrawText("+" + std::to_string(sect.overhits));
            overhitWidth = RightStatData.TextWidth("+" + std::to_string(sect.overhits));
        } else if (sect.hit != sect.notes) {
            RightStatData.Col(RED).DrawText("-" + std::to_string(sect.notes - sect.hit));
            overhitWidth = RightStatData.TextWidth(
                "-" + std::to_string(sect.notes - sect.hit));
        }
        LeftStatData.Bounds(RightStatData.pos.x - (overhitWidth * 1.5) - statsLeft,
                            ActualStatsHeight);
        LeftStatData.DrawText(sect.name);
        LeftStatData.pos.y += u.hinpct(0.03f);
        RightStatData.pos.y += u.hinpct(0.03f);
    }
}

void resultsMenu::DrawHistogram(Player &player, Rectangle rect, float cardHeight, int playerslot) {
    Units &u = Units::getInstance();
    float ActualStatsHeight = u.hinpct(0.03f);
    float statsHeight = rect.y;
    float statsLeft = rect.x + u.winpct(0.01f);
    float statsRight = rect.x + rect.width - u.winpct(0.01f);

    Encore::TextDisplay Header;
    Header.Pos(rect.x + (rect.width / 2), rect.y - (cardHeight * 0.07f)).Align(CENTER).
           Fnt(ASSET(josefinSansBold)).Col(LIGHTGRAY).Size(cardHeight * 0.05f);

    Encore::TextDisplay LeftStatData;
    Encore::TextDisplay RightStatData;

    LeftStatData.Pos(statsLeft, statsHeight).Size(u.hinpct(0.025f));
    RightStatData.Pos(statsRight, statsHeight).Size(u.hinpct(0.025f)).Align(RIGHT);
    Header.lDrawText("resultsMenu.histogramHeader");
    RightStatData.Fnt(ASSET(JetBrainsMono));
    int bottom = topSectList.at(playerslot) + MAX_LIST_LENGTH > player.engine->chart->
        sections.size()
        ? player.engine->chart->sections.size()
        : topSectList.at(playerslot) + MAX_LIST_LENGTH;

    float xPad = (rect.width * 0.05f);
    float yPad = (rect.height * 0.05f);
    Rectangle HistogramBox {rect.x + xPad, rect.y, rect.width - (xPad*2), (rect.height/2) - (yPad*2)};
    NPatchInfo scoreBoxPatch;
    scoreBoxPatch.source = { 0, 0, 128, 128 };
    scoreBoxPatch.top = HistogramBox.width * 0.1f;
    scoreBoxPatch.bottom = HistogramBox.width * 0.1f;
    scoreBoxPatch.left = HistogramBox.width * 0.1f;
    scoreBoxPatch.right = HistogramBox.width * 0.1f;
    scoreBoxPatch.layout = 0;
    BeginBlendMode(BLEND_MULTIPLIED);
    DrawRectangleRec(HistogramBox, LIGHTGRAY);
    EndBlendMode();

    float xOffset = HistogramBox.width / player.engine->stats->accuracies.size();
    float boxHalf = HistogramBox.height/2;
    float middle = HistogramBox.y+boxHalf;
    Vector2 pos {HistogramBox.x, middle};
    // augh i gotta figure out how to make this time dependant instead of.
    // you know. count dependant
    float songLength = curSong->end;
    float perfectNormalized = (perfectBackend/goodBackend)*1;

    for (auto& solo : player.engine->chart->solos) {
        float leftPos = HistogramBox.x + (HistogramBox.width * (solo.start.sec / songLength));
        float rightPos = HistogramBox.width * (solo.secLen() / songLength);
        Rectangle solorec {leftPos, HistogramBox.y, rightPos, HistogramBox.height};

        DrawRectangleRec(solorec, ColorAlpha(SKYBLUE, 0.5));

        BeginBlendMode(BLEND_MULTIPLIED);
        DrawTextureNPatch(ASSET(borderShadow),
                          scoreBoxPatch,
                          solorec,
                          { 0 },
                          0,
                          WHITE);
        EndBlendMode();
    }

    for (auto &sect : player.engine->chart->sections) {
        float leftPos = HistogramBox.x + (HistogramBox.width * (sect.start / songLength));
        float fontSize = cardHeight * 0.04;
        Vector2 top {leftPos, HistogramBox.y};
        Vector2 bot {leftPos, HistogramBox.y+HistogramBox.height + (fontSize/2)};
        DrawLineEx(top, bot, 1.5, ColorAlpha(LIGHTGRAY, 0.5));
        DrawTextPro(ASSET(rubik), sect.name.c_str(), bot, {0,fontSize/2}, 90, fontSize, 0, LIGHTGRAY);
    }
    DrawRectangle(pos.x, middle - (boxHalf * perfectNormalized), HistogramBox.width, (boxHalf * perfectNormalized) * 2, ColorAlpha(GOLD, 0.25f));
    DrawLineEx(pos, {pos.x + HistogramBox.width, middle}, 2, LIGHTGRAY);
    // I apologise.

    for (auto& acc : player.engine->stats->accuracies) {
        if (acc.miss) {
            float leftPos = HistogramBox.x + (HistogramBox.width * (acc.time / songLength));
            Vector2 top {leftPos, HistogramBox.y};
            Vector2 bot {leftPos, HistogramBox.y+HistogramBox.height};
            DrawLineEx(top, bot, 3, ColorAlpha(RED, 0.25));
        }
    }
    for (auto& acc : player.engine->stats->accuracies) {
        if (acc.miss) continue;
        float normalized = acc.offset/goodBackend;
        float leftPos = HistogramBox.x + (HistogramBox.width * (acc.time / songLength));
        pos.y = middle - (boxHalf * normalized);
        pos.x = leftPos;
        Color accColor = LIGHTGRAY;
        if (acc.offset < perfectBackend && acc.offset > -perfectBackend) accColor = GOLD;
        DrawRectangle(pos.x - 1, pos.y - 1, 2, 2, accColor);
        // pos.x += xOffset;
    }

    BeginBlendMode(BLEND_MULTIPLIED);
    DrawTextureNPatch(ASSET(borderShadow),
                      scoreBoxPatch,
                      HistogramBox,
                      { 0 },
                      0,
                      WHITE);
    EndBlendMode();
}

void resultsMenu::drawPlayerResults(Player &player, int playerslot) {
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    auto &stats = player.engine->stats;
    float cardPos = GetOvershellSlotLeft(playerslot);
    float cardWidth = GetOvershellSlotWidth();
    float cardHalfWidth = cardWidth / 2;
    float cardTop = u.hpct(0.18f);
    float topCardHeight = (float(assets.GradeBackgrounds[0]->height) / float(
        assets.GradeBackgrounds[0]->width)) * cardWidth;
    auto bgToDraw = ASSETPTR(cheese);

    float scorePos = (cardPos + cardHalfWidth);
    int Percent =
        floorf(((float)stats->NotesHit / (float)stats->AttemptedNotes) * 100.0f);
    double accuracy = stats->Accuracy / stats->AttemptedNotes;

    std::string ResultsShit = "F";
    Grade curGrade(WHITE, "", { 0, 0 });
    if (!stats->Bot) {
        bgToDraw = assets.GradeBackgrounds[GetGrade(accuracy, curGrade)];
        ResultsShit = curGrade.Letter;
    } else {
        curGrade = Grades.back();
    }

    bgToDraw->Draw({ cardPos, cardTop, cardWidth, topCardHeight }, WHITE);

    bool rendAsFC = stats->AttemptedNotes == stats->NotesHit && !stats->Bot && stats->
        Overhits == 0;

    float PercentSize = topCardHeight * 0.4;
    float InstIconSize = topCardHeight * 0.25;
    float DiffSize = topCardHeight * 0.2;
    float BannerSize = topCardHeight * 0.19;
    float left = cardPos + (cardWidth * 0.03);
    float cursor = cardTop;

    std::string percentText = std::to_string(Percent) + "%";
    std::string accText = std::to_string(accuracy * 100) + "%";
    int inst = player.Instrument < PlasticDrums
        ? player.Instrument
        : player.Instrument - PlasticDrums;
    assets.InstIcons[inst]->Draw({ cardPos + (cardWidth * 0.02f),
                                   cursor + (topCardHeight * 0.015f), InstIconSize,
                                   InstIconSize },
                                 WHITE);

    int diffNumber = curSong->Difficulties[player.Instrument];

    assets.BaseRingTexture.Draw({ cardPos + (cardWidth * 0.02f),
                                   cursor + (topCardHeight * 0.015f), InstIconSize,
                                   InstIconSize },
                                 WHITE);

    if (diffNumber > 0) {
        if (diffNumber > 6) {
            diffNumber = 6;
        }
        auto ring = TheAssets.YargRings[diffNumber - 1];
        ring->Draw({ cardPos + (cardWidth * 0.02f),
                                   cursor + (topCardHeight * 0.015f), InstIconSize,
                                   InstIconSize }, WHITE);
    }

    float asda = InstIconSize / 4;
    Encore::TextDisplay a;
    a.Pos(left + InstIconSize, cursor + asda + (topCardHeight * 0.0175f)).
      Fnt(ASSET(rubik)).Size(asda * 2).lDrawText(songPartsList[player.Instrument]);

    cursor += InstIconSize * 0.85;
    Encore::TextDisplay notesHitPercent;
    std::string diffString = LOCALISE(diffList[player.Difficulty]);
    std::ranges::transform(diffString, diffString.begin(), ::toupper);
    notesHitPercent.Fnt(ASSET(redHatDisplayBlack))
                   .Size(PercentSize)
                   .Pos(left, cursor)
                   .DrawText(percentText);
    cursor += PercentSize * 0.85;
    notesHitPercent.Fnt(ASSET(rubik))
                   .Size(DiffSize)
                   .Pos(left, cursor)
                   .DrawText(diffString);
    cursor += DiffSize;

    DrawRectangleGradientH(cardPos,
                           cursor,
                           cardWidth,
                           BannerSize,
                           ColorAlpha(player.AccentColor, 0.25),
                           ColorAlpha(player.AccentColor, 0));
    float totalSpace = u.hpct(1.0f) - u.hpct(0.36f);
    float lowerCardBottom = totalSpace - topCardHeight;
    DrawRectangleGradientV(cardPos,
                           cardTop + topCardHeight,
                           cardWidth,
                           lowerCardBottom,
                           GetColor(0x202033FF),
                           GetColor(0x181827FF));

    float gsodif = BannerSize / 6;
    cursor += gsodif;
    if (stats->Bot)
        ResultsShit = "autoplay";
    Encore::TextDisplay infoDisplay;
    infoDisplay.Pos(left, cursor).Fnt(ASSET(rubik)).Size(gsodif * 4).lDrawText(
        "resultsMenu.splash." + ResultsShit);

    NPatchInfo shadowOverlay;
    shadowOverlay.source = { 0, 0, 128, 128 };
    shadowOverlay.top = cardWidth * 0.1;
    shadowOverlay.bottom = cardWidth * 0.1;
    shadowOverlay.left = cardWidth * 0.1;
    shadowOverlay.right = cardWidth * 0.1;
    shadowOverlay.layout = 0;

    BeginBlendMode(BLEND_MULTIPLIED);
    DrawTextureNPatch(ASSET(borderShadow),
                      shadowOverlay,
                      { cardPos, cardTop + topCardHeight, cardWidth, lowerCardBottom },
                      { 0 },
                      0,
                      WHITE);
    EndBlendMode();
    float scoreBoxX = cardPos + (cardWidth * 0.1f);
    float scoreBoxY = cardTop + topCardHeight + (lowerCardBottom * 0.05f);
    float scoreBoxWidth = cardWidth * 0.8f;
    float scoreBoxHeight = lowerCardBottom * 0.3f;

    NPatchInfo scoreBoxPatch;
    scoreBoxPatch.source = { 0, 0, 128, 128 };
    scoreBoxPatch.top = scoreBoxWidth * 0.1f;
    scoreBoxPatch.bottom = scoreBoxWidth * 0.1f;
    scoreBoxPatch.left = scoreBoxWidth * 0.1f;
    scoreBoxPatch.right = scoreBoxWidth * 0.1f;
    scoreBoxPatch.layout = 0;
    DrawTextureNPatch(ASSET(resultsBox),
                      scoreBoxPatch,
                      { scoreBoxX, scoreBoxY, scoreBoxWidth, scoreBoxHeight },
                      { 0 },
                      0,
                      WHITE);

    std::string scoreString = GameMenu::scoreCommaFormatter(stats->Score);

    Encore::Text::DrawText(
        assets.redHatMono,
        scoreString,
        { scorePos, scoreBoxY + (scoreBoxHeight * 0.55f) },
        scoreBoxHeight * 0.4,
        GetColor(0x00adffFF),
        CENTER
    );

    renderPlayerStars(
        player,
        (cardPos + cardHalfWidth),
        scoreBoxY + (scoreBoxHeight * 0.1f),
        scoreBoxHeight * 0.35,
        false
    );

    Encore::TextDisplay Header;
    Header.Pos(cardPos + cardHalfWidth, scoreBoxY + (scoreBoxHeight * 1.125)).
           Align(CENTER).Fnt(ASSET(josefinSansBold)).Col(LIGHTGRAY).Size(
               lowerCardBottom * 0.05);

    float statsHeight = scoreBoxY + (scoreBoxHeight * 1.25) + (lowerCardBottom * 0.05);
    float ActualStatsHeight = u.hinpct(0.03f);
    float statsLeft = cardPos + u.winpct(0.01f);
    float statsRight = cardPos + cardWidth - u.winpct(0.01f);
    Encore::TextDisplay LeftStatData;
    Encore::TextDisplay RightStatData;

    LeftStatData.Pos(statsLeft, statsHeight).Size(u.hinpct(0.025f));
    RightStatData.Pos(statsRight, statsHeight).Size(u.hinpct(0.025f)).Align(RIGHT);

    Rectangle infoRect = { cardPos, statsHeight, cardWidth, lowerCardBottom - (scoreBoxHeight * 1.25f) };
    switch (resultsState.at(playerslot)) {
    case GENERAL:
        DrawStatistics(stats, curGrade, infoRect, lowerCardBottom);
        break;
    case SECTIONS:
        DrawSections(player, infoRect, lowerCardBottom, playerslot);
        break;
    case HISTOGRAM:
        DrawHistogram(player, infoRect, lowerCardBottom, playerslot);
        break;
    }
};


void resultsMenu::renderPlayerStars(
    Player &player,
    float xPos,
    float yPos,
    float scale,
    bool left
) {
    auto &stats = player.engine->stats;
    auto &engine = player.engine;

    int inst = engine->inst >= PlasticDrums ? engine->inst - 5 : engine->inst;
    stats->StarThresholdValue = stats->Score / engine->chart->BaseScore;
    for (int i = 0; i < 6; i++) {
        if (stats->StarThresholdValue > STAR_THRESHOLDS[inst][i]) {
            stats->Stars = i;
        }
    }

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
    for (int i = 0; i < 5; i++) {
        if (i > stats->Stars)
            break;
        DrawTexturePro(
            stats->Stars >= 5 ? GoldStar : Star,
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