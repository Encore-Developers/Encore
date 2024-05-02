//
// Created by marie on 02/05/2024.
//

#include "gameplay.h"

Assets assets;
SongList songList;

Settings settings;

template<typename CharT>
struct Separators : public std::numpunct<CharT>
{
    [[nodiscard]] virtual std::string do_grouping()
    const
    {
        return "\003";
    }
};

std::string Gameplay::scoreCommaFormatter(int value) {
    std::stringstream ss;
    ss.imbue(std::locale(std::cout.getloc(), new Separators <char>()));
    ss << std::fixed << value;
    return ss.str();
}

float diffDistance = Player::diff == 3 ? 2.0f : 1.5f;
float lineDistance = Player::diff == 3 ? 1.5f : 1.0f;
Camera3D camera = { 0 };



void Gameplay::gameplay() {

    Player::accentColor = Color{255, 0, 255, 255};
    Player::overdriveColor = Color{255, 200, 0, 255};

    // Y UP!!!! REMEMBER!!!!!!
    //						    	  x,    y,     z
    // 0.0f, 5.0f, -3.5f
    //							    	 6.5f
    camera.position = Vector3{ 0.0f, 7.0f, -10.0f };
    // 0.0f, 0.0f, 6.5f
    camera.target = Vector3{ 0.0f, 0.0f, 13.0f };

    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
    camera.fovy = 34.5f;
    camera.projection = CAMERA_PERSPECTIVE;
    // IMAGE BACKGROUNDS??????
    ClearBackground(BLACK);

    int scorePos = (GetScreenWidth()/4)* 3;
    DrawTextureEx(songBackground, {0,0},0, (float)GetScreenHeight()/songBackground.height,WHITE);
    int starsval = Player::stars(songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].baseScore,Player::diff);
    for (int i = 0; i < 5; i++) {
        DrawTextureEx(emptyStar, {scorePos+((float)i*40),80},0,0.15f,WHITE);
    }
    for (int i = 0; i < starsval; i++) {
        DrawTextureEx(Player::goldStars? goldStar : star, {scorePos+((float)i*40),80},0,0.15f,WHITE);
    }
    // DrawTextRubik(TextFormat("%s", starsDisplay), 5, GetScreenHeight() - 470, 48, goldStars ? GOLD : WHITE);
    int totalScore = Player::score + Player::sustainScoreBuffer[0] + Player::sustainScoreBuffer[1] + Player::sustainScoreBuffer[2] + Player::sustainScoreBuffer[3] + Player::sustainScoreBuffer[4];


    Assets::DrawTextRHDI(scoreCommaFormatter(totalScore).c_str(), (float)(scorePos + 200) - Assets::MeasureTextRHDI(scoreCommaFormatter(totalScore).c_str()), 30,  Color{107, 161, 222,255});
    Assets::DrawTextRHDI(scoreCommaFormatter(Player::combo).c_str(), (float)(scorePos + 200) - Assets::MeasureTextRHDI(scoreCommaFormatter(Player::combo).c_str()), 125, Player::FC ? GOLD : (Player::combo <= 3) ? RED : WHITE);
    Assets::DrawTextRubik32(TextFormat("%s", Player::FC ? "FC" : ""), 5, (float)GetScreenHeight() - 40, GOLD);
    // DrawTextRubik(TextFormat("%s", lastNotePerfect ? "Perfect" : ""), 5, (GetScreenHeight() - 370), 30, GOLD);

    float multFill = (!Player::overdrive ? (float)(Player::multiplier(Player::instrument) - 1) : ((float)(Player::multiplier(Player::instrument) / 2) - 1)) / (float)Player::maxMultForMeter(Player::instrument);
    SetShaderValue(odMultShader, multLoc, &multFill, SHADER_UNIFORM_FLOAT);
    SetShaderValue(multNumberShader, uvOffsetXLoc, &Player::uvOffsetX, SHADER_UNIFORM_FLOAT);
    SetShaderValue(multNumberShader, uvOffsetYLoc, &Player::uvOffsetY, SHADER_UNIFORM_FLOAT);
    float comboFill = Player::comboFillCalc(Player::instrument);
    SetShaderValue(odMultShader, comboCounterLoc, &comboFill, SHADER_UNIFORM_FLOAT);
    SetShaderValue(odMultShader, odLoc, &Player::overdriveFill, SHADER_UNIFORM_FLOAT);
    if (Player::extraGameplayStats) {
        Assets::DrawTextRubik(TextFormat("Perfect Hit: %01i", Player::perfectHit), 5, (float)GetScreenHeight() - 280, 24,
                      (Player::perfectHit > 0) ? GOLD : WHITE);
        Assets::DrawTextRubik(TextFormat("Max Combo: %01i", Player::maxCombo), 5, (float)GetScreenHeight() - 130, 24, WHITE);
        Assets::DrawTextRubik(TextFormat("Multiplier: %01i", Player::multiplier(Player::instrument)), 5, (float)GetScreenHeight() - 100, 24,
                      (Player::multiplier(Player::instrument) >= 4) ? SKYBLUE : WHITE);
        Assets::DrawTextRubik(TextFormat("Notes Hit: %01i", Player::notesHit), 5, (float)GetScreenHeight() - 250, 24, Player::FC ? GOLD : WHITE);
        Assets::DrawTextRubik(TextFormat("Notes Missed: %01i", Player::notesMissed), 5, (float)GetScreenHeight() - 220, 24,
                      ((Player::combo == 0) && (!Player::FC)) ? RED : WHITE);
        Assets::DrawTextRubik(TextFormat("Strikes: %01i", Player::playerOverhits), 5, (float)GetScreenHeight() - 40, 24, Player::FC ? GOLD : WHITE);
    }

    if ((Player::overdrive ? Player::multiplier(Player::instrument) / 2 : Player::multiplier(Player::instrument))>= (Player::instrument == 1 || Player::instrument == 3 ? 6 : 4)) {
        expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Player::accentColor;
        emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Player::accentColor;
        smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Player::accentColor;
        smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Player::accentColor;
    } else {
        expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = LIGHTGRAY;
        emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = LIGHTGRAY;
        smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = LIGHTGRAY;
        smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = LIGHTGRAY;
    }



    if (GuiButton({ 0,0,60,60 }, "<")) {
        for (Note& note : songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].notes) {
            note.accounted = false;
            note.hit = false;
            note.miss = false;
            note.held = false;
            note.heldTime = 0;
            note.hitTime = 0;
            note.perfect = false;
            note.countedForODPhrase = false;
        }
        glfwSetKeyCallback(glfwGetCurrentContext(), RhythmLogic::origKeyCallback);
        glfwSetGamepadStateCallback( RhythmLogic::origGamepadCallback);
        // notes = songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size();
        // notes = songList.songs[curPlayingSong].parts[instrument]->charts[diff];
        for (odPhrase& phrase : songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].odPhrases) {
            phrase.missed = false;
            phrase.notesHit = 0;
            phrase.added = false;
        }
        Scenes::SwitchScreen(SONG_SELECT);
        Player::overdrive = false;
        Player::overdriveFill = 0.0f;
        Player::overdriveActiveFill = 0.0f;
        Player::overdriveActiveTime = 0.0;
        RhythmLogic::curODPhrase = 0;
        expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
        emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
        multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
        multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
        multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
        expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = LIGHTGRAY;
        emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = LIGHTGRAY;
        RhythmLogic::isPlaying = false;
        RhythmLogic::midiLoaded = false;
        RhythmLogic::streamsLoaded = false;
    }
    if (!RhythmLogic::streamsLoaded) {
        RhythmLogic::loadedStreams = LoadStems(songList.songs[RhythmLogic::curPlayingSong].stemsPath);
        for (auto& stream : RhythmLogic::loadedStreams) {
            std::cout << GetMusicTimeLength(stream.first) << std::endl;
        }

        RhythmLogic::streamsLoaded = true;
        Player::resetPlayerStats();
    }
    else {
        float songPlayed = GetMusicTimePlayed(RhythmLogic::loadedStreams[0].first);
        int songLength = GetMusicTimeLength(RhythmLogic::loadedStreams[0].first);
        int playedMinutes = GetMusicTimePlayed(RhythmLogic::loadedStreams[0].first)/60;
        int playedSeconds = (int)GetMusicTimePlayed(RhythmLogic::loadedStreams[0].first) % 60;
        int songMinutes = GetMusicTimeLength(RhythmLogic::loadedStreams[0].first)/60;
        int songSeconds = (int)GetMusicTimeLength(RhythmLogic::loadedStreams[0].first) % 60;

        const char* textTime = TextFormat("%i:%02i / %i:%02i ", playedMinutes,playedSeconds,songMinutes,songSeconds);
        int textLength = Assets::MeasureTextRubik32(textTime);

        Assets::DrawTextRubik32(textTime,GetScreenWidth() - textLength,GetScreenHeight()-40,WHITE);
        if (GetTime() >= GetMusicTimeLength(RhythmLogic::loadedStreams[0].first) + RhythmLogic::startedPlayingSong) {
            for (Note& note : songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].notes) {
                note.accounted = false;
                note.hit = false;
                note.miss = false;
                note.held = false;
                note.heldTime = 0;
                note.hitTime = 0;
                note.perfect = false;
                note.countedForODPhrase = false;
                // notes += 1;
            }
            glfwSetKeyCallback(glfwGetCurrentContext(),  RhythmLogic::origKeyCallback);
            glfwSetGamepadStateCallback(RhythmLogic::origGamepadCallback);
            // notes = (int)songList.songs[curPlayingSong].parts[instrument]->charts[diff].notes.size();
            for (odPhrase& phrase : songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].odPhrases) {
                phrase.missed = false;
                phrase.notesHit = 0;
                phrase.added = false;
            }
            Player::overdrive = false;
            Player::overdriveFill = 0.0f;
            Player::overdriveActiveFill = 0.0f;
            Player::overdriveActiveTime = 0.0;
            RhythmLogic::isPlaying = false;
            RhythmLogic::midiLoaded = false;
            RhythmLogic::streamsLoaded = false;
            RhythmLogic::curODPhrase = 0;
            expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
            emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
            multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
            multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
            multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
            expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Player::accentColor;
            emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Player::accentColor;
            Scenes::SwitchScreen(RESULTS);

        }
        for (auto& stream : RhythmLogic::loadedStreams) {
            stream.first.looping = false;
            UpdateMusicStream(stream.first);
            PlayMusicStream(stream.first);
            if (Player::instrument == stream.second)
                SetAudioStreamVolume(stream.first.stream, Player::mute ? Player::missVolume : Player::selInstVolume);
            else
                SetAudioStreamVolume(stream.first.stream, Player::otherInstVolume);

        }
    }
    float highwayLength = Player::defaultHighwayLength * settings.highwayLengthMult;
    double musicTime = GetMusicTimePlayed(RhythmLogic::loadedStreams[0].first);
    if (Player::overdrive) {

        // expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTextureOD;
        // emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTextureOD;
        multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFillActive;
        multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFillActive;
        multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFillActive;


        Player::overdriveFill = Player::overdriveActiveFill - ((musicTime - Player::overdriveActiveTime) / (1920 / songList.songs[RhythmLogic::curPlayingSong].bpms[RhythmLogic::curBPM].bpm));
        if (Player::overdriveFill <= 0) {
            Player::overdrive = false;
            Player::overdriveActiveFill = 0;
            Player::overdriveActiveTime = 0.0;

            expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
            emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
            multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
            multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
            multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;

        }
    }
    for (int i = RhythmLogic::curBPM; i < songList.songs[RhythmLogic::curPlayingSong].bpms.size(); i++) {
        if (musicTime > songList.songs[RhythmLogic::curPlayingSong].bpms[i].time && i < songList.songs[RhythmLogic::curPlayingSong].bpms.size() - 1)
            RhythmLogic::curBPM++;
    }

    if (musicTime < 7.5) {
        Assets::DrawTextRHDI(songList.songs[RhythmLogic::curPlayingSong].title.c_str(), 25, ((GetScreenHeight()/3)) - 20, WHITE);
        DrawTextEx(redHatDisplayItalic, songList.songs[RhythmLogic::curPlayingSong].artist.c_str(), {45, ((GetScreenHeight()/3)) + 25.0f}, 30, 1.5, LIGHTGRAY);
        //DrawTextRHDI(songList.songs[curPlayingSong].artist.c_str(), 5, 130, WHITE);
    }

    BeginMode3D(camera);
    if (Player::diff == 3) {
        float highwayPosShit = ((20) * (1 - settings.highwayLengthMult));
        DrawModel(expertHighwaySides, Vector3{ 0,0,settings.highwayLengthMult < 1.0f ? -(highwayPosShit* (0.875f)) : 0 }, 1.0f, WHITE);
        DrawModel(expertHighway, Vector3{ 0,0,settings.highwayLengthMult < 1.0f ? -(highwayPosShit* (0.875f)) : 0 }, 1.0f, WHITE);
        if (settings.highwayLengthMult > 1.0f) {
            DrawModel(expertHighway, Vector3{ 0,0,((highwayLength*1.5f)+Player::smasherPos)-20 }, 1.0f, WHITE);
            DrawModel(expertHighwaySides, Vector3{ 0,0,((highwayLength*1.5f)+Player::smasherPos)-20 }, 1.0f, WHITE);
            if (highwayLength > 23.0f) {
                DrawModel(expertHighway, Vector3{ 0,0,((highwayLength*1.5f)+Player::smasherPos)-40 }, 1.0f, WHITE);
                DrawModel(expertHighwaySides, Vector3{ 0,0,((highwayLength*1.5f)+Player::smasherPos)-40 }, 1.0f, WHITE);
            }
        }

        if (Player::overdrive) {DrawModel(odHighwayX, Vector3{0,0.001f,0},1,WHITE);}

        for (int i = 0; i < 5; i++) {
            if (RhythmLogic::heldFrets[i] || RhythmLogic::heldFretsAlt[i]) {
                DrawModel(smasherPressed, Vector3{ diffDistance - (float)(i), 0.01f, Player::smasherPos }, 1.0f, WHITE);
            }
            else {
                DrawModel(smasherReg, Vector3{ diffDistance - (float)(i), 0.01f, Player::smasherPos }, 1.0f, WHITE);
            }
        }
        //DrawModel(lanes, Vector3 {0,0.1f,0}, 1.0f, WHITE);

        for (int i = 0; i < 4; i++) {
            float radius = (i == (settings.mirrorMode ? 2 : 1)) ? 0.05 : 0.02;

            DrawCylinderEx(Vector3{ lineDistance - i, 0, Player::smasherPos + 0.5f }, Vector3{ lineDistance - i, 0, (highwayLength *1.5f) + Player::smasherPos }, radius, radius, 15, Color{ 128,128,128,128 });
        }

        DrawModel(smasherBoard, Vector3{ 0, 0.001f, 0 }, 1.0f, WHITE);
    }
    else {
        float highwayPosShit = ((20) * (1 - settings.highwayLengthMult));
        DrawModel(emhHighwaySides, Vector3{ 0,0,settings.highwayLengthMult < 1.0f ? -(highwayPosShit* (0.875f)) : 0 }, 1.0f, WHITE);
        DrawModel(emhHighway, Vector3{ 0,0,settings.highwayLengthMult < 1.0f ? -(highwayPosShit* (0.875f)) : 0 }, 1.0f, WHITE);
        if (settings.highwayLengthMult > 1.0f) {
            DrawModel(emhHighway, Vector3{ 0,0,((highwayLength*1.5f)+Player::smasherPos)-20 }, 1.0f, WHITE);
            DrawModel(emhHighwaySides, Vector3{ 0,0,((highwayLength*1.5f)+Player::smasherPos)-20 }, 1.0f, WHITE);
            if (highwayLength > 23.0f) {
                DrawModel(expertHighway, Vector3{ 0,0,((highwayLength*1.5f)+Player::smasherPos)-40 }, 1.0f, WHITE);
                DrawModel(emhHighwaySides, Vector3{ 0,0,((highwayLength*1.5f)+Player::smasherPos)-40 }, 1.0f, WHITE);
            }
        }
        if (Player::overdrive) {DrawModel(odHighwayEMH, Vector3{0,0.001f,0},1,WHITE);}

        for (int i = 0; i < 4; i++) {
            if (RhythmLogic::heldFrets[i] || RhythmLogic::heldFretsAlt[i]) {
                DrawModel(smasherPressed, Vector3{ diffDistance - (float)(i), 0.01f, Player::smasherPos }, 1.0f, WHITE);
            }
            else {
                DrawModel(smasherReg, Vector3{ diffDistance - (float)(i), 0.01f, Player::smasherPos }, 1.0f, WHITE);

            }
        }
        for (int i = 0; i < 3; i++) {
            float radius = (i == 1) ? 0.03 : 0.01;
            DrawCylinderEx(Vector3{ lineDistance - (float)i, 0, Player::smasherPos + 0.5f }, Vector3{ lineDistance - (float)i, 0, (highwayLength *1.5f) + Player::smasherPos }, radius,
                           radius, 4.0f, Color{ 128, 128, 128, 128 });
        }
        DrawModel(smasherBoardEMH, Vector3{ 0, 0.001f, 0 }, 1.0f, WHITE);
    }
    if (songList.songs[RhythmLogic::curPlayingSong].beatLines.size() >= 0) {
        for (int i = RhythmLogic::curBeatLine; i < songList.songs[RhythmLogic::curPlayingSong].beatLines.size(); i++) {
            if (songList.songs[RhythmLogic::curPlayingSong].beatLines[i].first >= songList.songs[RhythmLogic::curPlayingSong].music_start && songList.songs[RhythmLogic::curPlayingSong].beatLines[i].first <= songList.songs[RhythmLogic::curPlayingSong].end) {
                double relTime = ((songList.songs[RhythmLogic::curPlayingSong].beatLines[i].first - musicTime) + Player::VideoOffset) * settings.trackSpeedOptions[settings.trackSpeed]  * ( 11.5f / highwayLength);
                if (relTime > 1.5) break;
                float radius = songList.songs[RhythmLogic::curPlayingSong].beatLines[i].second ? 0.03f : 0.0075f;
                DrawCylinderEx(Vector3{ -diffDistance - 0.5f,0,Player::smasherPos + (highwayLength * (float)relTime) }, Vector3{ diffDistance + 0.5f,0,Player::smasherPos + (highwayLength * (float)relTime) }, radius, radius, 4, Color{ 128,128,128,128 });
                if (relTime < -1 && RhythmLogic::curBeatLine < songList.songs[RhythmLogic::curPlayingSong].beatLines.size() - 1) {
                    RhythmLogic::curBeatLine++;

                }
            }
        }
    }

    // DrawTriangle3D(Vector3{ 2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,20.0f }, BLACK);
    // DrawTriangle3D(Vector3{ 2.5f,0.0f,0.0f }, Vector3{ -2.5f,0.0f,20.0f }, Vector3{ 2.5f,0.0f,20.0f }, BLACK);

    Player::notes = (int)songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff].notes.size();
    DrawModel(odFrame, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    DrawModel(odBar, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    DrawModel(multFrame, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    DrawModel(multBar, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    if (Player::instrument == 1 || Player::instrument == 3) {

        DrawModel(multCtr5, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    }
    else {

        DrawModel(multCtr3, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    }
    DrawModel(multNumber, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);


    // DrawLine3D(Vector3{ 2.5f, 0.05f, 2.0f }, Vector3{ -2.5f, 0.05f, 2.0f}, WHITE);
    Chart& curChart = songList.songs[RhythmLogic::curPlayingSong].parts[Player::instrument]->charts[Player::diff];
    if (!curChart.odPhrases.empty()) {

        float odStart = ((curChart.odPhrases[RhythmLogic::curODPhrase].start - musicTime) + Player::VideoOffset) * settings.trackSpeedOptions[settings.trackSpeed] * (11.5f / highwayLength);
        float odEnd = ((curChart.odPhrases[RhythmLogic::curODPhrase].end - musicTime) + Player::VideoOffset) * settings.trackSpeedOptions[settings.trackSpeed] * (11.5f / highwayLength);

        // horrifying.

        DrawCylinderEx(Vector3{ Player::diff == 3 ? 2.7f : 2.2f,0,(float)(Player::smasherPos + (highwayLength * odStart)) >= (highwayLength * 1.5f) + Player::smasherPos ? (highwayLength * 1.5f) + Player::smasherPos : (float)(Player::smasherPos + (highwayLength * odStart)) }, Vector3{ Player::diff == 3 ? 2.7f : 2.2f,0,(float)(Player::smasherPos + (highwayLength * odEnd)) >= (highwayLength * 1.5f) + Player::smasherPos ? (highwayLength * 1.5f) + Player::smasherPos : (float)(Player::smasherPos + (highwayLength * odEnd)) }, 0.075, 0.075, 10, Player::overdriveColor);
        DrawCylinderEx(Vector3{ Player::diff == 3 ? -2.7f : -2.2f,0,(float)(Player::smasherPos + (highwayLength * odStart)) >= (highwayLength * 1.5f) + Player::smasherPos ? (highwayLength * 1.5f) + Player::smasherPos : (float)(Player::smasherPos + (highwayLength * odStart)) }, Vector3{ Player::diff == 3 ? -2.7f : -2.2f,0,(float)(Player::smasherPos + (highwayLength * odEnd)) >= (highwayLength * 1.5f) + Player::smasherPos ? (highwayLength * 1.5f) + Player::smasherPos : (float)(Player::smasherPos + (highwayLength * odEnd)) }, 0.075, 0.075, 10, Player::overdriveColor);

    }
    for (int lane = 0; lane < (Player::diff == 3 ? 5 : 4); lane++) {
        for (int i = RhythmLogic::curNoteIdx[lane]; i < curChart.notes_perlane[lane].size(); i++) {
            Note& curNote = curChart.notes[curChart.notes_perlane[lane][i]];
            if (!curChart.odPhrases.empty()) {

                if (curNote.time >= curChart.odPhrases[RhythmLogic::curODPhrase].start && curNote.time <= curChart.odPhrases[RhythmLogic::curODPhrase].end && !curChart.odPhrases[RhythmLogic::curODPhrase].missed) {
                    if(curNote.hit) {
                        if (curNote.hit && !curNote.countedForODPhrase) {
                            curChart.odPhrases[RhythmLogic::curODPhrase].notesHit++;
                            curNote.countedForODPhrase = true;
                        }
                    }
                    curNote.renderAsOD = true;

                }
                if (curChart.odPhrases[RhythmLogic::curODPhrase].missed) {
                    curNote.renderAsOD = false;
                }
                if (curChart.odPhrases[RhythmLogic::curODPhrase].notesHit == curChart.odPhrases[RhythmLogic::curODPhrase].noteCount && !curChart.odPhrases[RhythmLogic::curODPhrase].added && Player::overdriveFill < 1.0f) {
                    Player::overdriveFill += 0.25f;
                    if (Player::overdriveFill > 1.0f) Player::overdriveFill = 1.0f;
                    if (Player::overdrive) {
                        Player::overdriveActiveFill = Player::overdriveFill;
                        Player::overdriveActiveTime = musicTime;
                    }
                    curChart.odPhrases[RhythmLogic::curODPhrase].added = true;
                }
            }
            if (!curNote.hit && !curNote.accounted && curNote.time + 0.1 < musicTime) {
                curNote.miss = true;
                Player::MissNote();
                if (!curChart.odPhrases.empty() && !curChart.odPhrases[RhythmLogic::curODPhrase].missed && curNote.time>=curChart.odPhrases[RhythmLogic::curODPhrase].start && curNote.time<curChart.odPhrases[RhythmLogic::curODPhrase].end) curChart.odPhrases[RhythmLogic::curODPhrase].missed = true;
                curNote.accounted = true;
            }


            double relTime = ((curNote.time - musicTime) + Player::VideoOffset) * settings.trackSpeedOptions[settings.trackSpeed] * ( 11.5f / highwayLength);
            double relEnd = (((curNote.time + curNote.len) - musicTime) + Player::VideoOffset) * settings.trackSpeedOptions[settings.trackSpeed] * ( 11.5f / highwayLength);
            float notePosX = diffDistance - (1.0f * (settings.mirrorMode ? (Player::diff == 3 ? 4 : 3) - curNote.lane : curNote.lane));
            if (relTime > 1.5) {
                break;
            }
            if (relEnd > 1.5) relEnd = 1.5;
            if (curNote.lift && !curNote.hit) {
                // lifts						//  distance between notes
                //									(furthest left - lane distance)
                if (curNote.renderAsOD)					//  1.6f	0.8
                    DrawModel(liftModelOD, Vector3{ notePosX,0,Player::smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
                    // energy phrase
                else
                    DrawModel(liftModel, Vector3{ notePosX,0,Player::smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
                // regular
            }
            else {
                // sustains
                if ((curNote.len) > 0) {
                    if (curNote.hit && curNote.held) {
                        if (curNote.heldTime < (curNote.len * settings.trackSpeedOptions[settings.trackSpeed])) {
                            curNote.heldTime = 0.0 - relTime;
                            Player::sustainScoreBuffer[curNote.lane] = (curNote.heldTime / curNote.len) * (12 * curNote.beatsLen) * Player::multiplier(Player::instrument);
                            if (relTime < 0.0) relTime = 0.0;
                        }
                        if (relEnd <= 0.0) {
                            if (relTime < 0.0) relTime = relEnd;
                            Player::score += Player::sustainScoreBuffer[curNote.lane];
                            Player::sustainScoreBuffer[curNote.lane] = 0;
                            curNote.held = false;
                        }
                    }
                    else if (curNote.hit && !curNote.held) {
                        relTime = relTime + curNote.heldTime;
                    }

                    /*Color SustainColor = Color{ 69,69,69,255 };
                    if (curNote.held) {
                        if (od) {
                            Color SustainColor = Color{ 217, 183, 82 ,255 };
                        }
                        Color SustainColor = Color{ 172,82,217,255 };
                    }*/

                    if (curNote.held && !curNote.renderAsOD) {
                        DrawCylinderEx(Vector3{ notePosX, 0.05f, Player::smasherPos + (highwayLength * (float)relTime) }, Vector3{ notePosX,0.05f, Player::smasherPos + (highwayLength * (float)relEnd) }, 0.1f, 0.1f, 15, Player::accentColor);
                    }
                    if (curNote.renderAsOD && curNote.held) {
                        DrawCylinderEx(Vector3{ notePosX, 0.05f, Player::smasherPos + (highwayLength * (float)relTime) }, Vector3{ notePosX,0.05f, Player::smasherPos + (highwayLength * (float)relEnd) }, 0.1f, 0.1f, 15, Color{ 255, 255, 255 ,255 });
                    }
                    if (!curNote.held && curNote.hit || curNote.miss) {
                        DrawCylinderEx(Vector3{ notePosX, 0.05f, Player::smasherPos + (highwayLength * (float)relTime) }, Vector3{ notePosX,0.05f, Player::smasherPos + (highwayLength * (float)relEnd) }, 0.1f, 0.1f, 15, Color{ 69,69,69,255 });
                    }
                    if (!curNote.hit && !curNote.accounted && !curNote.miss) {
                        if (curNote.renderAsOD) {
                            DrawCylinderEx(Vector3{ notePosX, 0.05f, Player::smasherPos + (highwayLength * (float)relTime) }, Vector3{ notePosX,0.05f, Player::smasherPos + (highwayLength * (float)relEnd) }, 0.1f, 0.1f, 15, Color{ 200, 200, 200 ,255 });
                        }
                        else {
                            DrawCylinderEx(Vector3{ notePosX, 0.05f,
                                                    Player::smasherPos + (highwayLength * (float)relTime) },
                                           Vector3{ notePosX, 0.05f,
                                                    Player::smasherPos + (highwayLength * (float)relEnd) }, 0.1f, 0.1f, 15,
                                           Player::accentColor);
                        }
                    }


                    // DrawLine3D(Vector3{ diffDistance - (1.0f * curNote.lane),0.05f,smasherPos + (12.5f * (float)relTime) }, Vector3{ diffDistance - (1.0f * curNote.lane),0.05f,smasherPos + (12.5f * (float)relEnd) }, Color{ 172,82,217,255 });
                }
                // regular notes
                if (((curNote.len) > 0 && (curNote.held || !curNote.hit)) || ((curNote.len) == 0 && !curNote.hit)) {
                    if (curNote.renderAsOD) {
                        if ((!curNote.held && !curNote.miss) || !curNote.hit) {
                            DrawModel(noteModelOD, Vector3{ notePosX,0,Player::smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
                        }

                    }
                    else {
                        if ((!curNote.held && !curNote.miss) || !curNote.hit) {
                            DrawModel(noteModel, Vector3{ notePosX,0,Player::smasherPos + (highwayLength * (float)relTime) }, 1.1f, WHITE);
                        }

                    }

                }
                expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Player::accentColor;
            }
            if (curNote.miss) {
                DrawModel(curNote.lift ? liftModel : noteModel, Vector3{ notePosX,0,Player::smasherPos + (highwayLength * (float)relTime) }, 1.0f, RED);
                if (GetMusicTimePlayed(RhythmLogic::loadedStreams[0].first) < curNote.time + 0.4 && Player::MissHighwayColor) {
                    expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = RED;
                }
                else {
                    expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Player::accentColor;
                }
            }
            if (curNote.hit && GetMusicTimePlayed(RhythmLogic::loadedStreams[0].first) < curNote.hitTime + 0.15f) {
                DrawCube(Vector3{ notePosX, 0, Player::smasherPos }, 1.0f, 0.5f, 0.5f, curNote.perfect ? Color{ 255,215,0,64 } : Color{ 255,255,255,64 });
                if (curNote.perfect) {
                    DrawCube(Vector3{ Player::diff == 3 ? 3.3f : 2.8f, 0, Player::smasherPos }, 1.0f, 0.01f, 0.5f, ORANGE);

                }
            }
            // DrawText3D(rubik, TextFormat("%01i", combo), Vector3{2.8f, 0, smasherPos}, 32, 0.5,0,false,FC ? GOLD : (combo <= 3) ? RED : WHITE);


            if (relEnd < -1 && RhythmLogic::curNoteIdx[lane] < curChart.notes_perlane[lane].size() - 1) RhythmLogic::curNoteIdx[lane] = i + 1;



        }

    }
    if (!curChart.odPhrases.empty() && RhythmLogic::curODPhrase<curChart.odPhrases.size() - 1 && musicTime>curChart.odPhrases[RhythmLogic::curODPhrase].end && (curChart.odPhrases[RhythmLogic::curODPhrase].added ||curChart.odPhrases[RhythmLogic::curODPhrase].missed)) {
        RhythmLogic::curODPhrase++;
    }
#ifndef NDEBUG
    // DrawTriangle3D(Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * goodFrontend * trackSpeed[speedSelection]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos + (highwayLength * goodFrontend * trackSpeed[speedSelection]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * goodBackend * trackSpeed[speedSelection]) }, Color{ 0,255,0,80 });
    // DrawTriangle3D(Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * goodBackend * trackSpeed[speedSelection]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos - (highwayLength * goodBackend * trackSpeed[speedSelection]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * goodFrontend * trackSpeed[speedSelection]) }, Color{ 0,255,0,80 });

    // DrawTriangle3D(Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * perfectFrontend * trackSpeed[speedSelection]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos + (highwayLength * perfectFrontend * trackSpeed[speedSelection]) }, Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * perfectBackend * trackSpeed[speedSelection]) }, Color{ 190,255,0,80 });
    // DrawTriangle3D(Vector3{ diffDistance + 0.5f,0.05f,smasherPos - (highwayLength * perfectBackend * trackSpeed[speedSelection]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos - (highwayLength * perfectBackend * trackSpeed[speedSelection]) }, Vector3{ -diffDistance - 0.5f,0.05f,smasherPos + (highwayLength * perfectFrontend * trackSpeed[speedSelection]) }, Color{ 190,255,0,80 });
#endif
    EndMode3D();

    float songPlayed = GetMusicTimePlayed(RhythmLogic::loadedStreams[0].first);
    int songLength = GetMusicTimeLength(RhythmLogic::loadedStreams[0].first);
    int playedMinutes = GetMusicTimePlayed(RhythmLogic::loadedStreams[0].first)/60;
    int playedSeconds = (int)GetMusicTimePlayed(RhythmLogic::loadedStreams[0].first) % 60;
    int songMinutes = GetMusicTimeLength(RhythmLogic::loadedStreams[0].first)/60;
    int songSeconds = (int)GetMusicTimeLength(RhythmLogic::loadedStreams[0].first) % 60;
    const char* textTime = TextFormat("%i:%02i / %i:%02i ", playedMinutes,playedSeconds,songMinutes,songSeconds);
    int textLength = Assets::MeasureTextRubik32(textTime);
    GuiSetStyle(PROGRESSBAR, BORDER_WIDTH, 0);
    GuiSetStyle(PROGRESSBAR, BASE, ColorToInt(Player::FC ? GOLD : Player::accentColor));
    GuiSetStyle(PROGRESSBAR, BASE_COLOR_NORMAL, ColorToInt(Player::FC ? GOLD : Player::accentColor));
    GuiSetStyle(PROGRESSBAR, BASE_COLOR_FOCUSED, ColorToInt(Player::FC ? GOLD : Player::accentColor));
    GuiSetStyle(PROGRESSBAR, BASE_COLOR_DISABLED, ColorToInt(Player::FC ? GOLD : Player::accentColor));
    GuiSetStyle(PROGRESSBAR, BASE_COLOR_PRESSED, ColorToInt(Player::FC ? GOLD : Player::accentColor));

    GuiProgressBar(Rectangle {0,(float)GetScreenHeight()-7,(float)GetScreenWidth(),8}, "", "", &songPlayed, 0, songLength);

};
