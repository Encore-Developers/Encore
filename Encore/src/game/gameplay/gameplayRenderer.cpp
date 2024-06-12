//
// Created by marie on 09/06/2024.
//

#include "game/gameplay/gameplayRenderer.h"
#include "game/Assets.h"
#include "game/Settings.h"
#include "game/menus/gameMenu.h"
#include "raymath.h"
#include "game/menus/uiUnits.h"
#include "rlgl.h"

Assets &gprAssets = Assets::getInstance();
Settings& gprSettings = Settings::getInstance();
AudioManager &gprAudioManager = AudioManager::getInstance();
Menu& gprMenu = Menu::getInstance();
Units& gprU = Units::getInstance();


void gameplayRenderer::RenderNotes(Player& player, Chart& curChart, double time, RenderTexture2D &notes_tex, float length) {
    float diffDistance = player.diff == 3 ? 2.0f : 1.5f;
    float lineDistance = player.diff == 3 ? 1.5f : 1.0f;

    BeginTextureMode(notes_tex);
    ClearBackground({0,0,0,0});
    BeginMode3D(camera);
    // glDisable(GL_CULL_FACE);
    for (int lane = 0; lane < (player.diff == 3 ? 5 : 4); lane++) {
        for (int i = curNoteIdx[lane]; i < curChart.notes_perlane[lane].size(); i++) {

            Color NoteColor = gprMenu.hehe && player.diff == 3 ? lane == 0 || lane == 4 ? SKYBLUE : lane == 1 || lane == 3 ? PINK : WHITE : player.accentColor;
            Note & curNote = curChart.notes[curChart.notes_perlane[lane][i]];
            if (curNote.hit) {
                player.totalOffset += curNote.HitOffset;
            }
            gprAssets.liftModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;

            gprAssets.noteTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
            gprAssets.noteBottomModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
            if (!curChart.Solos.empty()) {
                if (curNote.time >= curChart.Solos[curSolo].start &&
                    curNote.time <= curChart.Solos[curSolo].end) {
                    if (curNote.hit) {
                        if (curNote.hit && !curNote.countedForSolo) {
                            curChart.Solos[curSolo].notesHit++;
                            curNote.countedForSolo = true;
                        }
                    }
                }
            }
            if (!curChart.odPhrases.empty()) {

                if (curNote.time >= curChart.odPhrases[curODPhrase].start &&
                    curNote.time <= curChart.odPhrases[curODPhrase].end &&
                    !curChart.odPhrases[curODPhrase].missed) {
                    if (curNote.hit) {
                        if (curNote.hit && !curNote.countedForODPhrase) {
                            curChart.odPhrases[curODPhrase].notesHit++;
                            curNote.countedForODPhrase = true;
                        }
                    }
                    curNote.renderAsOD = true;

                }
                if (curChart.odPhrases[curODPhrase].missed) {
                    curNote.renderAsOD = false;
                }
                if (curChart.odPhrases[curODPhrase].notesHit ==
                    curChart.odPhrases[curODPhrase].noteCount &&
                    !curChart.odPhrases[curODPhrase].added && player.overdriveFill < 1.0f) {
                    player.overdriveFill += 0.25f;
                    if (player.overdriveFill > 1.0f) player.overdriveFill = 1.0f;
                    if (player.overdrive) {
                        player.overdriveActiveFill = player.overdriveFill;
                        player.overdriveActiveTime = time;
                    }
                    curChart.odPhrases[curODPhrase].added = true;
                }
            }
            if (!curNote.hit && !curNote.accounted && curNote.time + 0.1 < time+player.VideoOffset-player.InputOffset && !songEnded) {
                curNote.miss = true;
                player.MissNote();
                if (!curChart.odPhrases.empty() && !curChart.odPhrases[curODPhrase].missed &&
                    curNote.time >= curChart.odPhrases[curODPhrase].start &&
                    curNote.time < curChart.odPhrases[curODPhrase].end)
                    curChart.odPhrases[curODPhrase].missed = true;
                player.combo = 0;
                curNote.accounted = true;
            }

            double relTime = ((curNote.time - time)) *
                             gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);
            double relEnd = (((curNote.time + curNote.len) - time)) *
                            gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);
            float notePosX = diffDistance - (1.0f *
                                             (float) (gprSettings.mirrorMode ? (player.diff == 3 ? 4 : 3) -
                                                                                curNote.lane
                                                                              : curNote.lane));
            if (relTime > 1.5) {
                break;
            }
            if (relEnd > 1.5) relEnd = 1.5;
            if (curNote.lift && !curNote.hit) {
                // lifts						//  distance between notes
                //									(furthest left - lane distance)
                if (curNote.renderAsOD)                    //  1.6f	0.8
                    DrawModel(gprAssets.liftModelOD, Vector3{notePosX, 0, player.smasherPos +
                                                                       (length *
                                                                        (float) relTime)}, 1.1f, WHITE);
                    // energy phrase
                else
                    DrawModel(gprAssets.liftModel, Vector3{notePosX, 0, player.smasherPos +
                                                                     (length * (float) relTime)},
                              1.1f, WHITE);
                // regular
            } else {
                // sustains
                if ((curNote.len) > 0) {
                    if (curNote.hit && curNote.held) {
                        if (curNote.heldTime <
                            (curNote.len * gprSettings.trackSpeedOptions[gprSettings.trackSpeed])) {
                            curNote.heldTime = 0.0 - relTime;
                            player.sustainScoreBuffer[curNote.lane] =
                                    (float) (curNote.heldTime / curNote.len) * (12 * curNote.beatsLen) *
                                    player.multiplier(player.instrument);
                            if (relTime < 0.0) relTime = 0.0;
                        }
                        if (relEnd <= 0.0) {
                            if (relTime < 0.0) relTime = relEnd;
                            player.score += player.sustainScoreBuffer[curNote.lane];
                            player.sustainScoreBuffer[curNote.lane] = 0;
                            curNote.held = false;
                        }
                    } else if (curNote.hit && !curNote.held) {
                        relTime = relTime + curNote.heldTime;
                    }

                    /*Color SustainColor = Color{ 69,69,69,255 };
                    if (curNote.held) {
                        if (od) {
                            Color SustainColor = Color{ 217, 183, 82 ,255 };
                        }
                        Color SustainColor = Color{ 172,82,217,255 };
                    }*/
                    float sustainLen =
                            (length * (float) relEnd) - (length * (float) relTime);
                    Matrix sustainMatrix = MatrixMultiply(MatrixScale(1, 1, sustainLen),
                                                          MatrixTranslate(notePosX, 0.01f,
                                                                          player.smasherPos +
                                                                          (length *
                                                                           (float) relTime) +
                                                                          (sustainLen / 2.0f)));
                    BeginBlendMode(BLEND_ALPHA);
                    gprAssets.sustainMat.maps[MATERIAL_MAP_DIFFUSE].color = ColorTint(NoteColor, { 180,180,180,255 });
                    gprAssets.sustainMatHeld.maps[MATERIAL_MAP_DIFFUSE].color = ColorBrightness(NoteColor, 0.5f);


                    if (curNote.held && !curNote.renderAsOD) {
                        DrawMesh(sustainPlane, gprAssets.sustainMatHeld, sustainMatrix);
                        DrawCube(Vector3{notePosX, 0.1, player.smasherPos}, 0.4f, 0.2f, 0.4f,
                                 player.accentColor);
                        //DrawCylinderEx(Vector3{ notePosX, 0.05f, player.smasherPos + (highwayLength * (float)relTime) }, Vector3{ notePosX,0.05f, player.smasherPos + (highwayLength * (float)relEnd) }, 0.1f, 0.1f, 15, player.accentColor);
                    }
                    if (curNote.renderAsOD && curNote.held) {
                        DrawMesh(sustainPlane, gprAssets.sustainMatHeldOD, sustainMatrix);
                        DrawCube(Vector3{notePosX, 0.1, player.smasherPos}, 0.4f, 0.2f, 0.4f, WHITE);
                        //DrawCylinderEx(Vector3{ notePosX, 0.05f, player.smasherPos + (highwayLength * (float)relTime) }, Vector3{ notePosX,0.05f, player.smasherPos + (highwayLength * (float)relEnd) }, 0.1f, 0.1f, 15, Color{ 255, 255, 255 ,255 });
                    }
                    if (!curNote.held && curNote.hit || curNote.miss) {

                        DrawMesh(sustainPlane, gprAssets.sustainMatMiss, sustainMatrix);
                        //DrawCylinderEx(Vector3{ notePosX, 0.05f, player.smasherPos + (highwayLength * (float)relTime) }, Vector3{ notePosX,0.05f, player.smasherPos + (highwayLength * (float)relEnd) }, 0.1f, 0.1f, 15, Color{ 69,69,69,255 });
                    }
                    if (!curNote.hit && !curNote.accounted && !curNote.miss) {
                        if (curNote.renderAsOD) {
                            DrawMesh(sustainPlane, gprAssets.sustainMatOD, sustainMatrix);
                            //DrawCylinderEx(Vector3{ notePosX, 0.05f, player.smasherPos + (highwayLength * (float)relTime) }, Vector3{ notePosX,0.05f, player.smasherPos + (highwayLength * (float)relEnd) }, 0.1f, 0.1f, 15, Color{ 200, 200, 200 ,255 });
                        } else {
                            DrawMesh(sustainPlane, gprAssets.sustainMat, sustainMatrix);
                            /*DrawCylinderEx(Vector3{notePosX, 0.05f,
                                                    player.smasherPos + (highwayLength * (float)relTime) },
                                           Vector3{ notePosX, 0.05f,
                                                    player.smasherPos + (highwayLength * (float)relEnd) }, 0.1f, 0.1f, 15,
                                           player.accentColor);*/
                        }
                    }
                    EndBlendMode();

                    // DrawLine3D(Vector3{ diffDistance - (1.0f * curNote.lane),0.05f,smasherPos + (12.5f * (float)relTime) }, Vector3{ diffDistance - (1.0f * curNote.lane),0.05f,smasherPos + (12.5f * (float)relEnd) }, Color{ 172,82,217,255 });
                }
                // regular notes
                if (((curNote.len) > 0 && (curNote.held || !curNote.hit)) ||
                    ((curNote.len) == 0 && !curNote.hit)) {
                    if (curNote.renderAsOD) {
                        if ((!curNote.held && !curNote.miss) || !curNote.hit) {
                            DrawModel(gprAssets.noteTopModelOD, Vector3{notePosX, 0, player.smasherPos +
                                                                                  (length *
                                                                                   (float) relTime)}, 1.1f,
                                      WHITE);
                            DrawModel(gprAssets.noteBottomModelOD, Vector3{notePosX, 0, player.smasherPos +
                                                                                     (length *
                                                                                      (float) relTime)}, 1.1f,
                                      WHITE);
                        }

                    } else {
                        if ((!curNote.held && !curNote.miss) || !curNote.hit) {
                            DrawModel(gprAssets.noteTopModel, Vector3{notePosX, 0, player.smasherPos +
                                                                                (length *
                                                                                 (float) relTime)}, 1.1f,
                                      WHITE);
                            DrawModel(gprAssets.noteBottomModel, Vector3{notePosX, 0, player.smasherPos +
                                                                                   (length *
                                                                                    (float) relTime)}, 1.1f,
                                      WHITE);
                        }

                    }

                }
                gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
            }
            if (curNote.miss) {


                if (curNote.lift) {
                    DrawModel(gprAssets.liftModel,
                              Vector3{notePosX, 0, player.smasherPos + (length * (float) relTime)},
                              1.0f, RED);
                } else {
                    DrawModel(gprAssets.noteBottomModel,
                              Vector3{notePosX, 0, player.smasherPos + (length * (float) relTime)},
                              1.0f, RED);
                    DrawModel(gprAssets.noteTopModel,
                              Vector3{notePosX, 0, player.smasherPos + (length * (float) relTime)},
                              1.0f, RED);
                }


                if (gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) <
                    curNote.time + 0.4 && gprSettings.missHighwayColor) {
                    gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = RED;
                } else {
                    gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
                }
            }
            if (curNote.hit && gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) <
                               curNote.hitTime + 0.15f) {
                DrawCube(Vector3{ notePosX, 0.125 , player.smasherPos }, 1.0f, 0.25f, 0.5f,
                         curNote.perfect ? Color{255, 215, 0, 150} : Color{255, 255, 255, 150});
                if (curNote.perfect) {
                    DrawCube(Vector3{player.diff == 3 ? 3.3f : 2.8f, 0, player.smasherPos}, 1.0f, 0.01f,
                             0.5f, ORANGE);

                }
            }
            // DrawText3D(gprAssets.rubik, TextFormat("%01i", combo), Vector3{2.8f, 0, smasherPos}, 32, 0.5,0,false,FC ? GOLD : (combo <= 3) ? RED : WHITE);


            if (relEnd < -1 && curNoteIdx[lane] < curChart.notes_perlane[lane].size() - 1)
                curNoteIdx[lane] = i + 1;


        }

    }
    EndMode3D();
    EndTextureMode();

    SetTextureWrap(notes_tex.texture,TEXTURE_WRAP_CLAMP);
    notes_tex.texture.width = (float)GetScreenWidth();
    notes_tex.texture.height = (float)GetScreenHeight();
    DrawTexturePro(notes_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {0,0}, 0, WHITE );
}

void gameplayRenderer::RenderHud(Player& player, RenderTexture2D& hud_tex) {
    BeginTextureMode(hud_tex);
    ClearBackground({0,0,0,0});
    BeginMode3D(camera);
    DrawModel(gprAssets.odFrame, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    DrawModel(gprAssets.odBar, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    DrawModel(gprAssets.multFrame, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    DrawModel(gprAssets.multBar, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    if (player.instrument == 1 || player.instrument == 3) {

        DrawModel(gprAssets.multCtr5, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    }
    else {

        DrawModel(gprAssets.multCtr3, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    }
    DrawModel(gprAssets.multNumber, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
    EndMode3D();
    EndTextureMode();

    SetTextureWrap(hud_tex.texture,TEXTURE_WRAP_CLAMP);
    hud_tex.texture.width = (float)GetScreenWidth();
    hud_tex.texture.height = (float)GetScreenHeight();
    DrawTexturePro(hud_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {0,0}, 0, WHITE );
}

void gameplayRenderer::RenderGameplay(Player& player, double time, Song song, RenderTexture2D& highway_tex, RenderTexture2D& hud_tex, RenderTexture2D& notes_tex) {

    Chart& curChart = song.parts[player.instrument]->charts[player.diff];
    float highwayLength = player.defaultHighwayLength * gprSettings.highwayLengthMult;

    float multFill = (!player.overdrive ? (float)(player.multiplier(player.instrument) - 1) : ((float)(player.multiplier(player.instrument) / 2) - 1)) / (float)player.maxMultForMeter(player.instrument);
    SetShaderValue(gprAssets.odMultShader, gprAssets.multLoc, &multFill, SHADER_UNIFORM_FLOAT);
    SetShaderValue(gprAssets.multNumberShader, gprAssets.uvOffsetXLoc, &player.uvOffsetX, SHADER_UNIFORM_FLOAT);
    SetShaderValue(gprAssets.multNumberShader, gprAssets.uvOffsetYLoc, &player.uvOffsetY, SHADER_UNIFORM_FLOAT);
    float comboFill = player.comboFillCalc(player.instrument);
    SetShaderValue(gprAssets.odMultShader, gprAssets.comboCounterLoc, &comboFill, SHADER_UNIFORM_FLOAT);
    SetShaderValue(gprAssets.odMultShader, gprAssets.odLoc, &player.overdriveFill, SHADER_UNIFORM_FLOAT);

    if ((player.overdrive ? player.multiplier(player.instrument) / 2 : player.multiplier(player.instrument))>= (player.instrument == 1 || player.instrument == 3 ? 6 : 4)) {
        gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
        gprAssets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
        gprAssets.smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
        gprAssets.smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
    }
    else {
        gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;
        gprAssets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;
        gprAssets.smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;
        gprAssets.smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;
    }


    double musicTime = gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) - player.VideoOffset;
    if (player.overdrive) {

        // gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.highwayTextureOD;
        // gprAssets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.highwayTextureOD;
        gprAssets.multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFillActive;
        gprAssets.multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFillActive;
        gprAssets.multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFillActive;
        // THIS IS LOGIC!
        player.overdriveFill = player.overdriveActiveFill - (float)((musicTime - player.overdriveActiveTime) / (1920 / song.bpms[curBPM].bpm));
        if (player.overdriveFill <= 0) {
            player.overdrive = false;
            player.overdriveActiveFill = 0;
            player.overdriveActiveTime = 0.0;

            gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.highwayTexture;
            gprAssets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.highwayTexture;
            gprAssets.multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFill;
            gprAssets.multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFill;
            gprAssets.multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFill;

        }
    }

    for (int i = curBPM; i < song.bpms.size(); i++) {
        if (musicTime > song.bpms[i].time && i < song.bpms.size() - 1)
            curBPM++;
    }

    if (!curChart.odPhrases.empty() && curODPhrase<curChart.odPhrases.size() - 1 && musicTime>curChart.odPhrases[curODPhrase].end && (curChart.odPhrases[curODPhrase].added ||curChart.odPhrases[curODPhrase].missed)) {
        curODPhrase++;
    }

    if (!curChart.Solos.empty() && curSolo<curChart.Solos.size() - 1 && musicTime>curChart.Solos[curSolo].end) {
        curSolo++;
    }

    if (!curChart.Solos.empty() && time >= curChart.Solos[curSolo].start - 1 && time <= curChart.Solos[curSolo].end + 2.5) {


        int solopctnum = Remap(curChart.Solos[curSolo].notesHit, 0, curChart.Solos[curSolo].noteCount, 0, 100);
        Color accColor = solopctnum == 100 ? GOLD : WHITE;
        const char* soloPct = TextFormat("%i%%", solopctnum);
        float soloPercentLength = MeasureTextEx(gprAssets.rubikBold, soloPct, gprU.hinpct(0.09f), 0).x;

        Vector2 SoloBoxPos = {(GetScreenWidth()/2) - (soloPercentLength/2), gprU.hpct(0.2f)};

        DrawTextEx(gprAssets.rubikBold, soloPct, SoloBoxPos, gprU.hinpct(0.09f), 0, accColor);

        const char* soloHit = TextFormat("%i/%i", curChart.Solos[curSolo].notesHit, curChart.Solos[curSolo].noteCount);
        float soloHitLength = MeasureTextEx(gprAssets.josefinSansItalic, soloHit, gprU.hinpct(0.04f), 0).x;

        Vector2 SoloHitPos = {(GetScreenWidth()/2) - (soloHitLength/2), gprU.hpct(0.2f) + gprU.hinpct(0.1f)};

        DrawTextEx(gprAssets.josefinSansItalic, soloHit, SoloHitPos, gprU.hinpct(0.04f), 0, accColor);

        if (time >= curChart.Solos[curSolo].end && time <= curChart.Solos[curSolo].end + 2.5) {

            const char* PraiseText = "";
            if (solopctnum == 100) {
                PraiseText = "Perfect Solo!";
            } else if (solopctnum == 99) {
                PraiseText  = "Awesome Choke!";
            } else if (solopctnum > 90) {
                PraiseText  = "Awesome solo!";
            } else if (solopctnum > 80) {
                PraiseText  = "Great solo!";
            } else if (solopctnum > 75) {
                PraiseText  = "Decent solo";
            } else if (solopctnum > 50) {
                PraiseText  = "OK solo";
            } else if (solopctnum > 0) {
                PraiseText  = "Bad solo";
            }
            int PraiseWidth = MeasureTextEx(gprAssets.josefinSansItalic, PraiseText, gprU.hinpct(0.05f), 0).x;
            Vector2 PraisePos = {gprU.wpct(0.5f) - (PraiseWidth/2), gprU.hpct(0.2f) - gprU.hinpct(0.06f)};
            DrawTextEx(gprAssets.josefinSansItalic, PraiseText, PraisePos, gprU.hinpct(0.05f), 0, accColor);
        }
    }

    if (player.diff == 3) {
        RenderExpertHighway(player, song, time, highway_tex);
    } else {
        RenderEmhHighway(player, song, time, highway_tex);
    }
    RenderNotes(player, curChart, time, notes_tex, highwayLength);
    RenderHud(player, hud_tex);
}

void gameplayRenderer::RenderExpertHighway(Player& player, Song song, double time, RenderTexture2D &highway_tex)  {
    BeginTextureMode(highway_tex);
    ClearBackground({0,0,0,0});
    BeginMode3D(camera);

    float diffDistance = player.diff == 3 ? 2.0f : 1.5f;
    float lineDistance = player.diff == 3 ? 1.5f : 1.0f;

    float highwayLength = player.defaultHighwayLength * gprSettings.highwayLengthMult;
    float highwayPosShit = ((20) * (1 - gprSettings.highwayLengthMult));

    DrawTriangle3D({-diffDistance-0.5f,-0.002,0},{-diffDistance-0.5f,-0.002,(highwayLength *1.5f) + player.smasherPos},{diffDistance+0.5f,-0.002,0},Color{0,0,0,255});
    DrawTriangle3D({diffDistance+0.5f,-0.002,(highwayLength *1.5f) + player.smasherPos},{diffDistance+0.5f,-0.002,0},{-diffDistance-0.5f,-0.002,(highwayLength *1.5f) + player.smasherPos},Color{0,0,0,255});

    DrawModel(gprAssets.expertHighwaySides, Vector3{ 0,0,gprSettings.highwayLengthMult < 1.0f ? -(highwayPosShit* (0.875f)) : -0.2f }, 1.0f, WHITE);
    DrawModel(gprAssets.expertHighway, Vector3{ 0,0,gprSettings.highwayLengthMult < 1.0f ? -(highwayPosShit* (0.875f)) : -0.2f }, 1.0f, WHITE);
    if (gprSettings.highwayLengthMult > 1.0f) {
        DrawModel(gprAssets.expertHighway, Vector3{ 0,0,((highwayLength*1.5f)+player.smasherPos)-20-0.2f }, 1.0f, WHITE);
        DrawModel(gprAssets.expertHighwaySides, Vector3{ 0,0,((highwayLength*1.5f)+player.smasherPos)-20-0.2f }, 1.0f, WHITE);
        if (highwayLength > 23.0f) {
            DrawModel(gprAssets.expertHighway, Vector3{ 0,0,((highwayLength*1.5f)+player.smasherPos)-40-0.2f }, 1.0f, WHITE);
            DrawModel(gprAssets.expertHighwaySides, Vector3{ 0,0,((highwayLength*1.5f)+player.smasherPos)-40-0.2f }, 1.0f, WHITE);
        }
    }

    BeginBlendMode(BLEND_ALPHA);
    if (player.overdrive) {DrawModel(gprAssets.odHighwayX, Vector3{0,0.001f,0},1,WHITE);}

    DrawTriangle3D({-diffDistance-0.5f,0.002,player.smasherPos},{-diffDistance-0.5f,0.002,(highwayLength *1.5f) + player.smasherPos},{diffDistance+0.5f,0.002,player.smasherPos},Color{0,0,0,64});
    DrawTriangle3D({diffDistance+0.5f,0.002,(highwayLength *1.5f) + player.smasherPos},{diffDistance+0.5f,0.002,player.smasherPos},{-diffDistance-0.5f,0.002,(highwayLength *1.5f) + player.smasherPos},Color{0,0,0,64});

    DrawModel(gprAssets.smasherBoard, Vector3{ 0, 0.003f, 0 }, 1.0f, WHITE);

    for (int i = 0; i < 5;  i++) {
        Color NoteColor = gprMenu.hehe && player.diff == 3 ? i == 0 || i == 4 ? SKYBLUE : i == 1 || i == 3 ? PINK : WHITE : player.accentColor;

        gprAssets.smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
        gprAssets.smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;

        if (heldFrets[i] || heldFretsAlt[i]) {
            DrawModel(gprAssets.smasherPressed, Vector3{ diffDistance - (float)(i), 0.01f, player.smasherPos }, 1.0f, WHITE);
        }
        else {
            DrawModel(gprAssets.smasherReg, Vector3{ diffDistance - (float)(i), 0.01f, player.smasherPos }, 1.0f, WHITE);
        }
    }
    //DrawModel(gprAssets.lanes, Vector3 {0,0.1f,0}, 1.0f, WHITE);

    for (int i = 0; i < 4; i++) {
        float radius = (i == (gprSettings.mirrorMode ? 2 : 1)) ? 0.05 : 0.02;

        DrawCylinderEx(Vector3{ lineDistance - (float)i, 0, player.smasherPos + 0.5f }, Vector3{ lineDistance - i, 0, (highwayLength *1.5f) + player.smasherPos }, radius, radius, 15, Color{ 128,128,128,128 });
    }

    if (!song.beatLines.empty()) {
        DrawBeatlines(player, song, highwayLength, time);
    }
    if (!song.parts[player.instrument]->charts[player.diff].Solos.empty()) {
        DrawSolo(player, song.parts[player.instrument]->charts[player.diff], highwayLength, time);
    }
    if (!song.parts[player.instrument]->charts[player.diff].odPhrases.empty()) {
        DrawOverdrive(player, song.parts[player.instrument]->charts[player.diff], highwayLength, time);
    }

    EndBlendMode();
    EndMode3D();
    EndTextureMode();

    SetTextureWrap(highway_tex.texture,TEXTURE_WRAP_CLAMP);
    highway_tex.texture.width = (float)GetScreenWidth();
    highway_tex.texture.height = (float)GetScreenHeight();
    DrawTexturePro(highway_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {0,0}, 0, WHITE );

}

void gameplayRenderer::RenderEmhHighway(Player& player, Song song, double time, RenderTexture2D &highway_tex) {
    BeginTextureMode(highway_tex);
    ClearBackground({0,0,0,0});
    BeginMode3D(camera);

    float diffDistance = player.diff == 3 ? 2.0f : 1.5f;
    float lineDistance = player.diff == 3 ? 1.5f : 1.0f;

    float highwayLength = player.defaultHighwayLength * gprSettings.highwayLengthMult;
    float highwayPosShit = ((20) * (1 - gprSettings.highwayLengthMult));

    DrawModel(gprAssets.emhHighwaySides, Vector3{ 0,0,gprSettings.highwayLengthMult < 1.0f ? -(highwayPosShit* (0.875f)) : 0 }, 1.0f, WHITE);
    DrawModel(gprAssets.emhHighway, Vector3{ 0,0,gprSettings.highwayLengthMult < 1.0f ? -(highwayPosShit* (0.875f)) : 0 }, 1.0f, WHITE);
    if (gprSettings.highwayLengthMult > 1.0f) {
        DrawModel(gprAssets.emhHighway, Vector3{ 0,0,((highwayLength*1.5f)+player.smasherPos)-20 }, 1.0f, WHITE);
        DrawModel(gprAssets.emhHighwaySides, Vector3{ 0,0,((highwayLength*1.5f)+player.smasherPos)-20 }, 1.0f, WHITE);
        if (highwayLength > 23.0f) {
            DrawModel(gprAssets.emhHighway, Vector3{ 0,0,((highwayLength*1.5f)+player.smasherPos)-40 }, 1.0f, WHITE);
            DrawModel(gprAssets.emhHighwaySides, Vector3{ 0,0,((highwayLength*1.5f)+player.smasherPos)-40 }, 1.0f, WHITE);
        }
    }
    if (player.overdrive) {DrawModel(gprAssets.odHighwayEMH, Vector3{0,0.001f,0},1,WHITE);}

    DrawTriangle3D({-diffDistance-0.5f,0.002,player.smasherPos},{-diffDistance-0.5f,0.002,(highwayLength *1.5f) + player.smasherPos},{diffDistance+0.5f,0.002,player.smasherPos},Color{0,0,0,64});
    DrawTriangle3D({diffDistance+0.5f,0.002,(highwayLength *1.5f) + player.smasherPos},{diffDistance+0.5f,0.002,player.smasherPos},{-diffDistance-0.5f,0.002,(highwayLength *1.5f) + player.smasherPos},Color{0,0,0,64});

    DrawModel(gprAssets.smasherBoardEMH, Vector3{ 0, 0.001f, 0 }, 1.0f, WHITE);

    for (int i = 0; i < 4; i++) {
        Color NoteColor = gprMenu.hehe && player.diff == 3 ? i == 0 || i == 4 ? SKYBLUE : i == 1 || i == 3 ? PINK : WHITE : player.accentColor;

        gprAssets.smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
        gprAssets.smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;

        if (heldFrets[i] || heldFretsAlt[i]) {
            DrawModel(gprAssets.smasherPressed, Vector3{ diffDistance - (float)(i), 0.01f, player.smasherPos }, 1.0f, WHITE);
        }
        else {
            DrawModel(gprAssets.smasherReg, Vector3{ diffDistance - (float)(i), 0.01f, player.smasherPos }, 1.0f, WHITE);

        }
    }
    for (int i = 0; i < 3; i++) {
        float radius = (i == 1) ? 0.03 : 0.01;
        DrawCylinderEx(Vector3{ lineDistance - (float)i, 0, player.smasherPos + 0.5f }, Vector3{ lineDistance - (float)i, 0, (highwayLength *1.5f) + player.smasherPos }, radius,
                       radius, 4.0f, Color{ 128, 128, 128, 128 });
    }
    if (!song.beatLines.empty()) {
        DrawBeatlines(player, song, highwayLength, time);
    }
    if (!song.parts[player.instrument]->charts[player.diff].Solos.empty()) {
        DrawSolo(player, song.parts[player.instrument]->charts[player.diff], highwayLength, time);
    }
    if (!song.parts[player.instrument]->charts[player.diff].odPhrases.empty()) {
        DrawOverdrive(player, song.parts[player.instrument]->charts[player.diff], highwayLength, time);
    }
    
    EndBlendMode();
    EndMode3D();
    EndTextureMode();

    SetTextureWrap(highway_tex.texture,TEXTURE_WRAP_CLAMP);
    highway_tex.texture.width = (float)GetScreenWidth();
    highway_tex.texture.height = (float)GetScreenHeight();
    DrawTexturePro(highway_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {0,0}, 0, WHITE );

}

void gameplayRenderer::DrawBeatlines(Player& player, Song song, float length, double musicTime) {
    float diffDistance = player.diff == 3 ? 2.0f : 1.5f;
    float lineDistance = player.diff == 3 ? 1.5f : 1.0f;
    std::vector<std::pair<double, bool>> beatlines = song.beatLines;

    if (beatlines.size() >= 0) {
        for (int i = curBeatLine; i < beatlines.size(); i++) {
            if (beatlines[i].first >= song.music_start-1 && beatlines[i].first <= song.end) {
                double relTime = ((song.beatLines[i].first - musicTime)) * gprSettings.trackSpeedOptions[gprSettings.trackSpeed]  * ( 11.5f / length);
                if (relTime > 1.5) break;
                float radius = beatlines[i].second ? 0.05f : 0.01f;
                DrawCylinderEx(Vector3{ -diffDistance - 0.5f,0,player.smasherPos + (length * (float)relTime) },
                               Vector3{ diffDistance + 0.5f,0,player.smasherPos + (length * (float)relTime) },
                               radius,
                               radius,
                               4,
                               DARKGRAY);
                if (relTime < -1 && curBeatLine < beatlines.size() - 1) {
                    curBeatLine++;
                }
            }
        }
    }
}

void gameplayRenderer::DrawOverdrive(Player& player,  Chart& curChart, float length, double musicTime) {

    float odStart = (float)((curChart.odPhrases[curODPhrase].start - musicTime)) * gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);
    float odEnd = (float)((curChart.odPhrases[curODPhrase].end - musicTime)) * gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);

    // can be flipped. btw

    // horrifying.
    // main calc
    bool Beginning = (float)(player.smasherPos + (length * odStart)) >= (length * 1.5f) + player.smasherPos;
    bool Ending = (float)(player.smasherPos + (length * odEnd)) >= (length * 1.5f) + player.smasherPos;

    float HighwayEnd = (length * 1.5f) + player.smasherPos;

    // right calc
    float RightSideX = player.diff == 3 ? 2.7f : 2.2f;

    Vector3 RightSideStart = {RightSideX ,0,Beginning ?  HighwayEnd : (float)(player.smasherPos + (length * odStart)) };
    Vector3 RightSideEnd = { RightSideX,0, Ending ? HighwayEnd : (float)(player.smasherPos + (length * odEnd)) };

    // left calc
    float LeftSideX = player.diff == 3 ? -2.7f : -2.2f;

    Vector3 LeftSideStart = {LeftSideX ,0,Beginning ?  HighwayEnd : (float)(player.smasherPos + (length * odStart)) };
    Vector3 LeftSideEnd = { LeftSideX,0, Ending ? HighwayEnd : (float)(player.smasherPos + (length * odEnd)) };

    // draw
    DrawCylinderEx(RightSideStart, RightSideEnd, 0.07, 0.07, 10, RAYWHITE);
    DrawCylinderEx(LeftSideStart, LeftSideEnd, 0.07, 0.07, 10, RAYWHITE);
}

void gameplayRenderer::DrawSolo(Player& player, Chart& curChart, float length, double musicTime) {

    float soloStart = (float)((curChart.Solos[curSolo].start - musicTime)) * gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);
    float soloEnd = (float)((curChart.Solos[curSolo].end - musicTime)) * gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);

    // can be flipped. btw

    // horrifying.
    // main calc
    bool Beginning = (float)(player.smasherPos + (length * soloStart)) >= (length * 1.5f) + player.smasherPos;
    bool Ending = (float)(player.smasherPos + (length * soloEnd)) >= (length * 1.5f) + player.smasherPos;

    float HighwayEnd = (length * 1.5f) + player.smasherPos;

    // right calc
    float RightSideX = player.diff == 3 ? 2.7f : 2.2f;

    Vector3 RightSideStart = {RightSideX ,-0.0025,Beginning ?  HighwayEnd : (float)(player.smasherPos + (length * soloStart)) };
    Vector3 RightSideEnd = { RightSideX,-0.0025, Ending ? HighwayEnd : (float)(player.smasherPos + (length * soloEnd)) };

    // left calc
    float LeftSideX = player.diff == 3 ? -2.7f : -2.2f;

    Vector3 LeftSideStart = {LeftSideX ,-0.0025,Beginning ?  HighwayEnd : (float)(player.smasherPos + (length * soloStart)) };
    Vector3 LeftSideEnd = { LeftSideX,-0.0025, Ending ? HighwayEnd : (float)(player.smasherPos + (length * soloEnd)) };

    // draw
    DrawCylinderEx(RightSideStart, RightSideEnd, 0.07, 0.07, 10, SKYBLUE);
    DrawCylinderEx(LeftSideStart, LeftSideEnd, 0.07, 0.07, 10, SKYBLUE);
}
