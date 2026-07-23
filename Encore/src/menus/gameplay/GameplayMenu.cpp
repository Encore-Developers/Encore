//
// Created by marie on 20/10/2024.
//

#include "GameplayMenu.h"

#include "../main/MainMenu.h"
#include "../util/uiUnits.h"
#include "song/audio.h"
#include "song/songlist.h"
#include "raymath.h"
#include "raygui.h"
#include "resultsMenu.h"
#include "gameplay/enctime.h"
#include "../util/styles.h"
#include "easing/easing.h"

#include "users/playerManager.h"
#include "../MenuManager.h"
#include "../overshell/OvershellHelper.h"
#include "../../settings/settings.h"
#include "SDL3/SDL_filesystem.h"
#include "debug/EncoreDebug.h"

#include <raylib.h>

#include <utility>

#include "gameplay/LyricRenderer.h"
#include "gameplay/inputCallbacks.h"
#include "settings/keybinds.h"
#include "song/OpenSource.h"
#include "tracy/Tracy.hpp"

Encore::LyricRenderer TheLyricRenderer;

GameplayMenu::GameplayMenu(Song* song, std::shared_ptr<VideoBackground> videoBackground) : curSong(song), videoBackground(std::move(videoBackground)) {
    hasOvershell = false;
}

GameplayMenu::~GameplayMenu() {
    ShowCursor();
}

bool GameplayMenu::CheckPauseInput(Encore::ControllerEvent event) {
    if (IsPaused()) {
        OvershellControllerInputCallback(this, event);
        return true;
    }
    if ((event.channel == Encore::InputChannel::PAUSE && event.action == Encore::Action::PRESS)
        || event.channel == Encore::InputChannel::DISCONNECT) {
        for (int i = 0; i < MAX_PLAYERS; i++) {
            if (!ThePlayerManager.ActivePlayers[i]) continue;
            Player &player = ThePlayerManager.GetActivePlayer(i);
            if (player.joypadID == event.slot) {
                OvershellState[i] = OS_OPTIONS;
                for (int g = 0; g < player.engine->chart->Lanes.size(); g++) {
                    player.engine->chart->DropSustain(g);
                }
                break;
            }
        }
        return true;
    }
    return false;
}
void GameplayMenu::UpdatePauseState() {
    if (IsPaused()) {
        if (!streamsPaused) {
            TheAudioManager.pauseStreams();
            streamsPaused = true;
        }
    } else {
        if (streamsPaused) {
            TheAudioManager.unpauseStreams();
            for (int i = 0; i < MAX_PLAYERS; i++) {
                if (!ThePlayerManager.ActivePlayers[i]) continue;
                Player &player = ThePlayerManager.GetActivePlayer(i);
                player.engine->chart->MissedNotePointers.clear();
            }
            streamsPaused = false;
        }
    }
}
bool GameplayMenu::IsPaused() {
    for (const int i : OvershellState) {
        if (i != OS_ATTRACT) {
            return true;
        }
    }
    return false;
}
void GameplayMenu::SetPresence() {
    int inst = 0;
    if (ThePlayerManager.PlayersActive == 1) {
        for (int playerNum = 0; playerNum < MAX_PLAYERS; playerNum++) {
            if (!ThePlayerManager.ActivePlayers[playerNum]) continue;
            inst = ThePlayerManager.GetActivePlayer(playerNum).Instrument;
        }
    }
    TheGameRPC.DiscordUpdatePresenceSong(
        "Playing a song",
        curSong->title + " - " + curSong->artist,
        inst,
        ThePlayerManager.PlayersActive
    );
    TheGameRPC.SteamUpdatePresence("song", (curSong->title + " - " + curSong->artist).c_str());
    TheGameRPC.SteamUpdatePresence("steam_display", "#StatusPlayingSongNamed");
    TheGameRPC.SteamOverlayPosition(true);
}

void GameplayMenu::KeyboardInputCallback(SDL_KeyboardEvent* sdlEvent) {
    if (sdlEvent->repeat)
        return;
    for (auto track : tracks) {
        if (!track) {
            continue;
        }
        Player &player = track->player;
        if (player.joypadID != -1 && player.joypadID != -2) {
            continue;
        }
        Encore::RhythmEngine::BaseEngine *engine = player.engine.get();

        Encore::ControllerEvent event;
        event.slot = -1;
        if (sdlEvent->down) {
            event.action = Encore::Action::PRESS;
        } else {
            event.action = Encore::Action::RELEASE;
        }
        SDL_Keycode sdlKeycode = sdlEvent->key;
        if (sdlKeycode == TheGameKeybinds.overdriveBinds.first || sdlKeycode == TheGameKeybinds.
            overdriveBinds.second) {
            event.channel = Encore::InputChannel::OVERDRIVE;
        }
        if (!player.Bot) {
            if (player.bindingType != PAD) {
                if (sdlKeycode == TheGameKeybinds.strumBinds.first) {
                    event.channel = Encore::InputChannel::STRUM_UP;
                } else if (sdlKeycode == TheGameKeybinds.strumBinds.second) {
                    event.channel = Encore::InputChannel::STRUM_DOWN;
                }
            }
            int DiffMax = (player.Difficulty == 3 || player.Instrument > PartVocals)
                ? 6
                : 4;

            for (int i = 0; i < DiffMax; i++) {
                if (sdlKeycode == TheGameKeybinds.keybinds5k[i] || sdlKeycode == TheGameKeybinds.
                    keybinds5kalt[
                        i]) {
                    event.channel = Encore::IntIC(i);
                }
            }
        }
        if (sdlKeycode == SDLK_ESCAPE && sdlEvent->down) {
            event.channel = Encore::InputChannel::PAUSE;
        }
        event.timestamp = SDLTimeToAudioTime(sdlEvent->timestamp);
        if (!CheckPauseInput(event))
            if (event.channel != Encore::InputChannel::INVALID) {
                engine->UpdateOnFrame(event.timestamp);
                engine->ProcessInput(event);
                event.slot = player.ActiveSlot;
                recordingReplay.inputs.push_back(event);
            }
    }
};

void GameplayMenu::ControllerInputCallback(Encore::ControllerEvent event) {
    for (auto track : tracks) {
        if (!track) {
            continue;
        }
        Player &player = track->player;
        if (player.joypadID != -2 && player.joypadID != event.slot) {
            continue;
        }

        Encore::RhythmEngine::BaseEngine *engine = player.engine.get();

        if (!CheckPauseInput(event)) {
            if (player.Bot)
                continue;
            if (engine->allowTimestampedInputs) {
                if (engine->IsWithinPracticeSection(event.timestamp) || !engine->practice)
                    engine->UpdateOnFrame(event.timestamp);
            }
            engine->ProcessInput(event);
            event.slot = player.ActiveSlot;
            recordingReplay.inputs.push_back(event);
        }
    }
};

void GameplayMenu::DrawScorebox(Units &u, Assets &assets, float scoreY) {
    Rectangle scoreboxSrc{
        0, 0, float(assets.Scorebox.width), float(assets.Scorebox.height)
    };
    double score = 0;
    for (int playerNum = 0; playerNum < MAX_PLAYERS; playerNum++) {
        if (!ThePlayerManager.ActivePlayers[playerNum]) continue;
        score += ThePlayerManager.GetActivePlayer(playerNum).engine->stats->Score;
    }
    // not optimized at all LMAO
    float WidthOfScorebox = u.hinpct(0.28);
    // float scoreY = u.hpct(0.15f);
    float ScoreboxX = u.RightSide;
    float HeightOfScorebox = WidthOfScorebox / (5 + (1/3));
    float ScoreboxY = scoreY;
    float scoreTextPadding = (HeightOfScorebox - u.hinpct(0.05)) / 2;
    Rectangle scoreboxDraw{ ScoreboxX, ScoreboxY, WidthOfScorebox, HeightOfScorebox };
    DrawTexturePro(
        assets.Scorebox,
        scoreboxSrc,
        scoreboxDraw,
        { WidthOfScorebox, 0 },
        0,
        WHITE
    );
    BeginBlendMode(BLEND_ADDITIVE);
    Encore::Text::DrawText(
        assets.redHatMono,
        GameMenu::scoreCommaFormatter(
            score
        ),
        { u.RightSide - u.winpct(0.0145f), ScoreboxY + scoreTextPadding },
        u.hinpct(0.05),
        Color{ 107, 161, 222, 255 },
        RIGHT
    );
    EndBlendMode();
}

void GameplayMenu::DrawTimerbox(Units &u, Assets &assets, float scoreY) {
    Rectangle TimerboxSrc{
        0, 0, float(assets.Timerbox.width), float(assets.Timerbox.height)
    };
    float WidthOfTimerbox = u.hinpct(0.14);
    // float scoreY = u.hpct(0.15f);
    float TimerboxX = u.RightSide;

    float HeightOfTimerbox = WidthOfTimerbox / 2;
    float TimerboxY = scoreY;
    Rectangle TimerboxDraw{ TimerboxX, TimerboxY, WidthOfTimerbox, HeightOfTimerbox };
    DrawTexturePro(
        assets.Timerbox,
        TimerboxSrc,
        TimerboxDraw,
        { WidthOfTimerbox, HeightOfTimerbox / 2 },
        0,
        WHITE
    );
    int played = TheSongTime.GetElapsedTime();
    int length = TheSongTime.GetSongLength();
    float Width = Remap(played, 0, length, 0, WidthOfTimerbox);
    BeginScissorMode(
        TimerboxX - WidthOfTimerbox,
        TimerboxY - (HeightOfTimerbox / 2),
        Width + 1,
        HeightOfTimerbox + 1
    );
    DrawTexturePro(
        assets.TimerboxOutline,
        TimerboxSrc,
        TimerboxDraw,
        { WidthOfTimerbox, HeightOfTimerbox / 2 },
        0,
        WHITE
    );
    EndScissorMode();
    int playedMinutes = played / 60;
    int playedSeconds = played % 60;
    int songMinutes = length / 60;
    int songSeconds = length % 60;
    const char *textTime = TextFormat(
        "%i:%02i / %i:%02i",
        playedMinutes,
        playedSeconds,
        songMinutes,
        songSeconds
    );
    Encore::Text::DrawText(
        assets.rubik,
        textTime,
        { u.RightSide - (WidthOfTimerbox / 2), TimerboxY - u.hinpct(SmallHeader * (0.66 * 1.25)) },
        u.hinpct(SmallHeader * 0.66),
        WHITE,
        CENTER
    );
}

void GameplayMenu::DrawGameplayStars(
    Units &u,
    Assets &assets,
    float scorePos,
    float starY
) {
    // todo: redo for band
    double score = 0;
    double baseScore = 0;
    for (int playerNum = 0; playerNum < MAX_PLAYERS; playerNum++) {
        if (!ThePlayerManager.ActivePlayers[playerNum]) continue;
        score += ThePlayerManager.GetActivePlayer(playerNum).engine->stats->Score;
        baseScore += ThePlayerManager.GetActivePlayer(playerNum).engine->chart->BaseScore;
    }
    // auto &player = ThePlayerManager.GetActivePlayer(playerInt);
    // int inst = player.Instrument % 5;
    // int diff = player.Difficulty;
    double starPercent = score / baseScore;
    // int starsVal = player.engine->stats->Stars;
    for (int i = 0; i < 5; i++) {
        bool firstStar = (i == 0);
        float starX = scorePos - u.hinpct(0.26) + (i * u.hinpct(0.0525));
        float starWH = u.hinpct(0.05);
        Rectangle emptyStarWH = {
            0, 0, (float)assets.emptyStar.width, (float)assets.emptyStar.height
        };
        Rectangle starRect = { starX, starY, starWH, starWH };
        DrawTexturePro(assets.emptyStar, emptyStarWH, starRect, { 0, 0 }, 0, WHITE);
        float yMaskPos = Remap(
            starPercent,
            firstStar ? 0 : STAR_THRESHOLDS[0][i - 1],
            STAR_THRESHOLDS[0][i],
            0,
            u.hinpct(0.05)
        );
        BeginScissorMode(starX, (starY + starWH) - yMaskPos, starWH, yMaskPos);
        DrawTexturePro(
            assets.star,
            emptyStarWH,
            starRect,
            { 0, 0 },
            0,
            starPercent < STAR_THRESHOLDS[0][i] ? Color{ 192, 192, 192, 128 } : WHITE
        );
        EndScissorMode();
    }
    if (starPercent >= STAR_THRESHOLDS[0][4]) {
        float starWH = u.hinpct(0.05);
        Rectangle emptyStarWH = {
            0, 0, (float)assets.goldStar.width, (float)assets.goldStar.height
        };
        unsigned char alpha = starPercent >= STAR_THRESHOLDS[0][5] ? 255 : (1-TheSongTime.GetBeatlineDelta() * 255);
        float yMaskPos = Remap(
            starPercent,
            STAR_THRESHOLDS[0][4],
            STAR_THRESHOLDS[0][5],
            0,
            u.hinpct(0.05)
        );
        BeginScissorMode(
            scorePos - (starWH * 6),
            (starY + starWH) - yMaskPos,
            scorePos,
            yMaskPos
        );
        for (int i = 0; i < 5; i++) {
            float starX = scorePos - u.hinpct(0.26) + (i * u.hinpct(0.0525));
            Rectangle starRect = { starX, starY, starWH, starWH };
            DrawTexturePro(
                starPercent >= STAR_THRESHOLDS[0][5]
                ? assets.goldStar
                : assets.goldStarUnfilled,
                emptyStarWH,
                starRect,
                { 0, 0 },
                0,
                { 255, 255, 255, alpha }
            );
        }
        EndScissorMode();
    }
}

unsigned char BeatToCharViaTickThing(
    int tick,
    int MinBrightness,
    int MaxBrightness,
    int QuarterNoteLength
) {
    float TickModulo = tick % QuarterNoteLength;
    return Remap(
        TickModulo / float(QuarterNoteLength),
        0,
        1.0f,
        MaxBrightness,
        MinBrightness
    );
}



double GetNotePos(double noteTime, double songTime, float length, float end) {
    return ((noteTime - songTime) * (length * 2.5)) - end;
}

void GameplayMenu::SaveReplay() {
    std::filesystem::path replayPath = SDL_GetPrefPath("Encore", "v0.2.0");
    bool playback = false;
    for (auto track : tracks) {
        if (track->player.PlaybackReplay) {
            playback = true;
            break;
        }
    }
    if (!playback) {
        // TODO: date+time filename
        replayPath /= (curSong->title + ".encrReplay");
        encore::bin_ofstream_le replayOut(replayPath, std::ios::binary);
        for (auto &track : tracks) {
            recordingReplay.participants.emplace_back(track->player);
        }
        recordingReplay.Save(replayOut);
        replayOut.close();
    }
}
void GameplayMenu::Draw() {
    UpdatePauseState();
    UIInput = IsPaused();
    if (IsPaused() || EncoreDebug::showDebug) {
        ShowCursor();
    } else {
        HideCursor();
    }
    Units &u = Units::getInstance();
    Assets &assets = Assets::getInstance();
    TheSongTime.UpdateTick();
    TheSongTime.UpdateBeatlines();
    ClearBackground(BLACK);
    unsigned char BackgroundColor = 0;
    if (!songPlaying && tracks.back()->IntroTimer==0) {
        TheAudioManager.UpdateAudioStreamVolumes();
        TheAudioManager.BeginPlayback(TheAudioManager.loadedStreams[0].handle);
        songPlaying = true;
        if (IsPaused()) {
            TheAudioManager.pauseStreams();
            streamsPaused = true;
        }
    }
    Texture2D* video = nullptr;
    if (videoBackground) {
        video = videoBackground->GetTexture(TheSongTime.GetElapsedTime() + curSong->videoStartTime);
    }
    if (video) {
        DrawTexturePro(*video, {0, 0, (float)video->width, (float)video->height}, {0, 0, (float)GetRenderWidth(), (float)GetRenderHeight()}, Vector2(0, 0), 0, WHITE);
    } else {
        GameMenu::DrawAlbumArtBackground();
    }
    DrawRectangle(0, 0, GetRenderWidth(), GetRenderHeight(), Color{ 0, 0, 0, 128 });
    DrawRectangle(
        0,
        0,
        GetRenderWidth(),
        GetRenderHeight(),
        Color{ 255, 255, 255, BackgroundColor }
    );

    if (TheSongTime.GetElapsedTime() > curSong->end - 0.1) {
        // TODO: endgame
        TheSongTime.FullReset();
        TheAudioManager.unloadStreams();
        songPlaying = false;
        SaveReplay();
        TheMenuManager.CreateAndSwitchMenu<resultsMenu>(curSong);
        return;
    }

    TheLyricRenderer.RenderLyrics();

    for (auto &stream : TheAudioManager.loadedStreams) {
        float volume = TheGameSettings.GetInactiveVolume();
        if (stream.instrument == AUDIOSTEM(Vocals))
            volume = TheGameSettings.GetVocalsVolume();
        if (stream.instrument == AUDIOSTEM(Crowd))
            volume = TheGameSettings.GetCrowdVolume();
        stream.volume = volume;
    }

    for (auto& track : tracks) {
        Player &player = track->player;

        if (player.PlaybackReplay) {
            player.ReplayPlayer->Advance(TheSongTime.GetElapsedTime());
            while (player.ReplayPlayer->HasNextInput()) {
                auto input = player.ReplayPlayer->GetNextInput();
                player.engine->UpdateOnFrame(input->timestamp);
                player.engine->ProcessInput(*input);
            }
        }

        if (!IsPaused()) {
            ZoneScopedN("Engine Update")
            if (player.engine->IsWithinPracticeSection(TheSongTime.GetElapsedTime()) || !
                player.engine->practice) {
                }
            player.engine->UpdateOnFrame(TheSongTime.GetElapsedTime());
            player.engine->UpdateStats(player.Instrument, player.Difficulty);
        }

        track->Draw();
        // double offset = player.engine->stats->TotalOffset / player.engine->stats->NotesHit;
        // if (player.engine->stats->NotesHit == 0) offset = 0;
        // TheSongTime.SetOffset(offset);
        auto chart = player.engine->chart;
        float volume = TheGameSettings.GetActiveVolume();
        if (player.engine->stats->AudioMuted) {
            volume = TheGameSettings.GetMuteVolume();
        }
        TheAudioManager.SetAudioStreamVolume(GetStemFromInstrument(SongPart(player.Instrument)), volume);
    }
    TheAudioManager.UpdateAudioStreamVolumes();

    float scorePos = u.RightSide - u.hinpct(0.01f);
    float scoreY = u.hpct(0.2f);
    float starY = scoreY + u.hinpct(0.065f);
    // Draw Stars
    DrawGameplayStars(u, assets, scorePos, starY);

    // Draw Timerbox
    DrawTimerbox(u, assets, scoreY);

    // Score Drawing
    DrawScorebox(u, assets, scoreY);

    // please God smite this code. flip a few bits in my hard drive. please get rid of this shit somehow
    // there's better ways. forgive me for I have sinned

    const auto sourceTex = TheSourceIcons[curSong->source]->GetTexture();
    Encore::TextDisplay title;
    Encore::TextDisplay secondary;
    float topOfVocalBar = u.hpct(0.2f);
    float TitleFontSize = u.hinpct(0.0425f * 0.75f);
    title.Size(TitleFontSize).Pos(u.wpct(0.01f), topOfVocalBar)
    .Fnt(ASSET(josefinSansBold)).DrawText(curSong->title);

    float TitleWidth = title.TextWidth(curSong->title);
    float TitleFontOffset = (TitleFontSize * 1.25f);
    float SecondaryFontSize = TitleFontSize * 0.85f;
    secondary.Size(SecondaryFontSize)
    .Pos(title.pos.x + TitleWidth + u.winpct(0.005f), topOfVocalBar + (TitleFontSize - SecondaryFontSize))
    .Fnt(ASSET(josefinSansBoldItalic)).Col(LIGHTGRAY);
    if (TheSongList.PlaylistSize > 0)
        secondary.DrawText(LOCALISE_FMT("gameplay.playlistDisplay", TheSongList.PlaylistIndex, TheSongList.PlaylistSize));

    secondary.Pos(u.wpct(0.01f), topOfVocalBar + TitleFontOffset)
    .DrawText(curSong->artist + ", " + curSong->releaseYear);

    secondary.AddPos(( TitleFontSize * 1.125f), TitleFontSize)
    .DrawText(curSong->charters[0]);
    DrawTexturePro(sourceTex, {0,0, (float)sourceTex.width, (float)sourceTex.height},
        {u.wpct(0.01f), topOfVocalBar + (TitleFontOffset + SecondaryFontSize), TitleFontSize, TitleFontSize}, {0,0}, 0, WHITE
    );

    std::string CurrentSectionName = "None";
    if (!TheSongTime.Sections.empty()) {
        for (int i = 0; i < TheSongTime.Sections.size() - 1; i++) {
            if (TheSongTime.Sections.at(i).start <= TheSongTime.GetElapsedTime()
                && TheSongTime.Sections.at(i+1).start > TheSongTime.GetElapsedTime()) {
                CurrentSectionName = TheSongTime.Sections.at(i).name;
                break;
            }
        }
    }

    secondary.AddPos(-( TitleFontSize * 1.125f), TitleFontSize).DrawText(LOCALISE_FMT("gameplay.sectionDisplay", CurrentSectionName));

    if (IsPaused()) {
        DrawPauseMenu();
    }

    GameMenu::DrawTopBarText(true);
}

void GameplayMenu::Load() {
    ZoneScoped;
    buttReg.buttMap.clear();
    NEWBUTTONACTION2(buttReg, LANE_1, "generic.confirm", {});
    NEWBUTTONACTION2(buttReg, PAUSE, "generic.unpause", {
        if (_action != Encore::Action::PRESS) return;
        for (auto &state : OvershellState) {
            state = OS_ATTRACT;
        }
    })
    NEWBUTTONACTION2(buttReg, LANE_2, "generic.back", {
        if (_action != Encore::Action::PRESS) return;
        for (auto &state : OvershellState) {
            state = OS_ATTRACT;
        }
    })
    curSong->LoadAlbumArt();
    TheSongTime.Reset();
    TheSongTime.SetOffset(TheGameSettings.AudioOffset / 1000.0);
    double songEnd = floor(TheAudioManager.GetMusicTimeLength());
    TheSongTime.Start(songEnd);
    dropInDropOut = false;

    float widthPerPlayer = 2.0f / ThePlayerManager.PlayersActive;
    double End = 0.0;
    double EndEvent = curSong->end;
    double LastNote = 0.0;
    double AudioEnd = TheAudioManager.GetMusicTimeLength();
    int playerCount = 0;

    float trackMaxScale = 1;

    if (ThePlayerManager.PlayersActive == 1) {
        trackMaxScale = 1.1;
    }

    for (auto &stream : TheAudioManager.loadedStreams) {
        float volume = TheGameSettings.GetInactiveVolume();
        if (stream.instrument == AUDIOSTEM(Vocals))
            volume = TheGameSettings.GetVocalsVolume();
        if (stream.instrument == AUDIOSTEM(Crowd))
            volume = TheGameSettings.GetCrowdVolume();
        stream.volume = volume;
    }

    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (!ThePlayerManager.ActivePlayers[i]) continue;
        ZoneScopedN("Player Init")
        Player &player = ThePlayerManager.GetActivePlayer(i);

        TheAudioManager.SetAudioStreamVolume(GetStemFromInstrument(SongPart(player.Instrument)), TheGameSettings.GetActiveVolume());

        auto track = std::make_shared<Encore::Track>(player);
        track->Load();
        track->ColumnLeft = -1 + widthPerPlayer * playerCount;
        track->ColumnRight = -1 + widthPerPlayer * (playerCount + 1);
        track->MaxScale = trackMaxScale;
        track->IntroTimer += (0.5 * playerCount);
        switch (player.Instrument) {
        case PlasticGuitar:
        case PlasticBass:
        case PlasticKeys:
            track->Configure5Lane();
            track->ColorProfileType = Encore::ProfileManager::PLASTIC;
            break;
        case PlasticDrums:
            if (player.engine->chart->size == 5) {
                track->ConfigureDrums();
            } else {
                track->Configure5LaneDrums();
            }
            track->ColorProfileType = Encore::ProfileManager::DRUMS;
            break;
        default:
            if (player.Difficulty == 3) {
                track->Configure5Lane();
            } else {
                track->Configure4Lane();
            }
            track->ColorProfileType = Encore::ProfileManager::PAD;
        }
        if (player.Instrument == PlasticBass || player.Instrument == PartVocals
            || player.Instrument == PartBass) {
            player.engine->stats->SixMultiplier = true;
        }
        if (player.Bot)
            player.engine->stats->Bot = true;

        for (auto &lane : player.engine->chart->Lanes) {
            if (lane.empty()) continue;
            if (lane.back().LengthSeconds + lane.back().StartSeconds > LastNote) {
                LastNote = lane.back().StartSeconds + lane.back().LengthSeconds + 1;
            }
        }
        tracks.push_back(track);
        playerCount++;
    }

    if (LastNote > EndEvent) End = LastNote;
    if (AudioEnd > LastNote) End = AudioEnd;
    if (EndEvent > LastNote) End = EndEvent;
    curSong->end = End;

    recordingReplay.song = curSong->hash;

    TheAudioManager.UpdateAudioStreamVolumes();
}
void GameplayMenu::DrawPauseMenu() {
    Assets &assets = Assets::getInstance();
    Units &u = Units::getInstance();

    float AlbumArtLeft = u.LeftSide;
    float AlbumArtTop = u.hpct(0.05f);
    float AlbumArtRight = u.winpct(0.15f);

    // why do you exist
    float AlbumArtBottom = u.winpct(0.15f);
    DrawRectangle(
        0,
        0,
        (int)GetRenderWidth(),
        (int)GetRenderHeight(),
        GetColor(0x00000080)
    );

    GameMenu::DrawTopOvershell(0.2f);
    GameMenu::DrawTopBarText(true);

    DrawRectangle(
        (int)u.LeftSide,
        (int)AlbumArtTop,
        (int)AlbumArtRight + 12,
        (int)AlbumArtBottom + 12,
        WHITE
    );
    DrawRectangle(
        (int)u.LeftSide + 6,
        (int)AlbumArtTop + 6,
        (int)AlbumArtRight,
        (int)AlbumArtBottom,
        BLACK
    );
    buttReg.DrawPrompts(false, u.hpct(0.2f), AlbumArtLeft + AlbumArtRight + 24);
    DrawTexturePro(
        TheArtLoader.loadedArt->GetTexture(),
        Rectangle{ 0,
                   0,
                   (float)TheArtLoader.loadedArt->GetTexture().width,
                   (float)TheArtLoader.loadedArt->GetTexture().width },
        Rectangle{ u.LeftSide + 6, AlbumArtTop + 6, AlbumArtRight, AlbumArtBottom },
        { 0, 0 },
        0,
        WHITE
    );

    float BottomOvershell = u.hpct(1) - u.hinpct(0.15f);
    float TextPlacementTB = AlbumArtTop;
    float TextPlacementLR = AlbumArtRight + AlbumArtLeft + 32;
    Encore::Text::DrawText(
        ASSET(redHatDisplayItalic),
        curSong->title.c_str(),
        { TextPlacementLR, TextPlacementTB },
        u.hinpct(0.05f),
        WHITE,
        LEFT
    );
    Encore::Text::DrawText(
        ASSET(rubikItalic),
        curSong->artist.c_str(),
        { TextPlacementLR, TextPlacementTB + u.hinpct(0.05125f) },
        u.hinpct(0.04f),
        WHITE,
        LEFT
    );
    const auto sourceTex = TheSourceIcons[curSong->source]->GetTexture();
    DrawTexturePro(sourceTex, {0,0, (float)sourceTex.width, (float)sourceTex.height},
        {TextPlacementLR, TextPlacementTB + u.hinpct(0.095f), u.hinpct(0.04f), u.hinpct(0.04f)}, {0,0}, 0, WHITE
    );
    if (!curSong->charters.empty()) {
        Encore::Text::DrawText(
            ASSET(rubikItalic),
            curSong->charters[0],
            { TextPlacementLR + u.hinpct(0.05f), TextPlacementTB + u.hinpct(0.095f) },
            u.hinpct(0.04f),
            WHITE,
            LEFT
        );
    }
    GameMenu::DrawBottomOvershell();
    DrawOvershell();
}