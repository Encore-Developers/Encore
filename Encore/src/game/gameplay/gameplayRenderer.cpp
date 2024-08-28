//
// Created by marie on 09/06/2024.
//

#include "game/gameplay/gameplayRenderer.h"
#include "game/assets.h"
#include "game/settings.h"
#include "game/menus/gameMenu.h"
#include "raymath.h"
#include "game/menus/uiUnits.h"
#include "rlgl.h"
#include "easing/easing.h"

Assets &gprAssets = Assets::getInstance();
Settings& gprSettings = Settings::getInstance();
AudioManager &gprAudioManager = AudioManager::getInstance();
GameMenu& gprMenu = TheGameMenu;
Units& gprU = Units::getInstance();



void gameplayRenderer::RenderNotes(Player& player, Chart& curChart, double time, RenderTexture2D &notes_tex, float length) {
	float diffDistance = player.diff == 3 ? 2.0f : 1.5f;
	float lineDistance = player.diff == 3 ? 1.5f : 1.0f;

	BeginTextureMode(notes_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(camera3pVector[cameraSel]);
	// glDisable(GL_CULL_FACE);
	for (int lane = 0; lane < (player.diff == 3 ? 5 : 4); lane++) {
		for (int i = curNoteIdx[lane]; i < curChart.notes_perlane[lane].size(); i++) {
			Color NoteColor;
			if (player.plastic) {
				bool pd = (player.instrument == 4);
				switch (lane) {
					case 0:
						if (pd) {
							NoteColor = ORANGE;
						} else {
							NoteColor = GREEN;
						}
						break;
					case 1:
						if (pd) {
							NoteColor = RED;
						} else {
							NoteColor = RED;
						}
						break;
					case 2:
						if (pd) {
							NoteColor = YELLOW;
						} else {
							NoteColor = YELLOW;
						}
						break;
					case 3:
						if (pd) {
							NoteColor = BLUE;
						} else {
							NoteColor = BLUE;
						}
						break;
					case 4:
						if (pd) {
							NoteColor = GREEN;
						} else {
							NoteColor = ORANGE;
						}
						break;
					default:
						NoteColor = player.accentColor;
						break;
				}
			}   else {
				NoteColor = gprMenu.hehe && player.diff == 3 ? (lane == 0 || lane == 4 ? SKYBLUE : (lane == 1 || lane == 3 ? PINK : WHITE)) : player.accentColor;
			}

			// Color NoteColor = gprMenu.hehe && player.diff == 3 ? (lane == 0 || lane == 4 ? SKYBLUE : (lane == 1 || lane == 3 ? PINK : WHITE)) : player.accentColor;
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
					curNote.time < curChart.odPhrases[curODPhrase].end &&
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
			if (!curNote.hit && !curNote.accounted && curNote.time + goodBackend + player.InputOffset < time && !songEnded) {
				curNote.miss = true;
				player.MissNote();
				if (!curChart.odPhrases.empty() && !curChart.odPhrases[curODPhrase].missed &&
					curNote.time >= curChart.odPhrases[curODPhrase].start &&
					curNote.time < curChart.odPhrases[curODPhrase].end)
					curChart.odPhrases[curODPhrase].missed = true;
				player.combo = 0;
				curNote.accounted = true;
			} else if (bot) {
				if (!curNote.hit && !curNote.accounted && curNote.time < time &&
					curNoteInt < curChart.notes.size() && !songEnded) {
					curNote.hit = true;
					if (curNote.len > 0) curNote.held = true;
					curNote.accounted = true;
					player.combo++;
					curNote.accounted = true;
					curNote.hitTime = time;
				}
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
			if (curNote.lift && !curNote.hit && !curNote.miss) {
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
							if (!bot) {
								player.sustainScoreBuffer[curNote.lane] =
										(float) (curNote.heldTime / curNote.len) * (12 * curNote.beatsLen) *
										player.multiplier(player.instrument);
							}
							if (relTime < 0.0) relTime = 0.0;
						}
						if (relEnd <= 0.0) {
							if (relTime < 0.0) relTime = relEnd;
							if (!bot) {
								player.score += player.sustainScoreBuffer[curNote.lane];
								player.sustainScoreBuffer[curNote.lane] = 0;
							}
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
						if ((!curNote.held && !curNote.miss) && !curNote.hit) {
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
						if ((!curNote.held && !curNote.miss) && !curNote.hit) {
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
			double HitAnimDuration = 0.15f;
			double PerfectHitAnimDuration = 1.0f;
			if (curNote.hit && gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) <
							   (curNote.hitTime) + HitAnimDuration) {

				double TimeSinceHit = gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) - curNote.hitTime;
				unsigned char HitAlpha = Remap(getEasingFunction(EaseInBack)(TimeSinceHit/HitAnimDuration), 0, 1.0, 196, 0);

				DrawCube(Vector3{notePosX, 0.125, player.smasherPos}, 1.0f, 0.25f, 0.5f,
						 curNote.perfect ? Color{255, 215, 0, HitAlpha} : Color{255, 255, 255, HitAlpha});
				// if (curNote.perfect) {
				// 	DrawCube(Vector3{3.3f, 0, player.smasherPos}, 1.0f, 0.01f,
				// 			 0.5f, Color{255,161,0,HitAlpha});
				// }


			}
			if (curNote.hit && gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) <
							(curNote.hitTime) + PerfectHitAnimDuration && curNote.perfect) {

				double TimeSinceHit = gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) - curNote.hitTime;
				unsigned char HitAlpha = Remap(getEasingFunction(EaseOutQuad)(TimeSinceHit/PerfectHitAnimDuration), 0, 1.0, 255, 0);
				float HitPosLeft = Remap(getEasingFunction(EaseInOutBack)(TimeSinceHit/PerfectHitAnimDuration), 0, 1.0, 3.4, 3.0);

				DrawCube(Vector3{HitPosLeft, -0.1f, player.smasherPos}, 1.0f, 0.01f,
						 0.5f, Color{255,161,0,HitAlpha});
				DrawCube(Vector3{HitPosLeft, -0.11f, player.smasherPos}, 1.0f, 0.01f,
						 1.0f, Color{255,161,0,(unsigned char)(HitAlpha/2)});
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
	DrawTexturePro(notes_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
}

void gameplayRenderer::RenderClassicNotes(Player& player, Chart& curChart, double time, RenderTexture2D &notes_tex, float length) {
	float diffDistance = 2.0f;
	float lineDistance = 1.5f;
	BeginTextureMode(notes_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(camera3pVector[cameraSel]);
	// glDisable(GL_CULL_FACE);


	for (auto & curNote : curChart.notes) {

		if (!curChart.Solos.empty()) {
			if (curNote.time >= curChart.Solos[curSolo].start &&
				curNote.time < curChart.Solos[curSolo].end) {
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
				curNote.time < curChart.odPhrases[curODPhrase].end &&
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
		if (!curNote.hit && !curNote.accounted && curNote.time + goodBackend + player.InputOffset < time &&
			!songEnded && curNoteInt < curChart.notes.size() && !songEnded && !bot) {
			TraceLog(LOG_INFO, TextFormat("Missed note at %f, note %01i", time, curNoteInt));
			curNote.miss = true;
			FAS = false;
			player.MissNote();
			if (!curChart.odPhrases.empty() && !curChart.odPhrases[curODPhrase].missed &&
				curNote.time >= curChart.odPhrases[curODPhrase].start &&
				curNote.time < curChart.odPhrases[curODPhrase].end)
				curChart.odPhrases[curODPhrase].missed = true;
			player.combo = 0;
			curNote.accounted = true;
			curNoteInt++;
		} else if (bot) {
			if (!curNote.hit && !curNote.accounted && curNote.time < time && curNoteInt < curChart.notes.size() && !songEnded) {
				curNote.hit = true;
				if (curNote.len > 0) curNote.held = true;
				curNote.accounted = true;
				player.combo++;
				curNote.accounted = true;
				curNote.hitTime = time;
				curNoteInt++;
			}
		}

		double relTime = ((curNote.time - time)) *
						 gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);
		double relEnd = (((curNote.time + curNote.len) - time)) *
						gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);

		float hopoScale = curNote.phopo ? 0.75f : 1.1f;

		for (int lane: curNote.pLanes) {

			int noteLane = gprSettings.mirrorMode ? 4 - lane : lane;

			Color NoteColor;
			switch (lane) {
				case 0:
					NoteColor = gprMenu.hehe ? SKYBLUE : GREEN;
					break;
				case 1:
					NoteColor = gprMenu.hehe ? PINK : RED;
					break;
				case 2:
					NoteColor = gprMenu.hehe ? RAYWHITE : YELLOW;
					break;
				case 3:
					NoteColor = gprMenu.hehe ? PINK : BLUE;
					break;
				case 4:
					NoteColor = gprMenu.hehe ? SKYBLUE : ORANGE;
					break;
				default:
					NoteColor = player.accentColor;
					break;
			}

			gprAssets.noteTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
			gprAssets.noteTopModelHP.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;

			float notePosX = diffDistance - (1.0f * noteLane);
			if (relTime > 1.5) {
				break;
			}
			if (relEnd > 1.5) relEnd = 1.5;
			if ((curNote.phopo || curNote.pTap) && !curNote.hit && !curNote.miss) {
				if (curNote.renderAsOD) {
					gprAssets.noteTopModelHP.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GOLD;
					gprAssets.noteBottomModelHP.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
					DrawModel(gprAssets.noteTopModelHP, Vector3{notePosX, 0, player.smasherPos +
																			 (length *
																			  (float) relTime)}, 1.1f,
							  WHITE);
					DrawModel(gprAssets.noteBottomModelHP, Vector3{notePosX, 0, player.smasherPos +
																				(length *
																				 (float) relTime)}, 1.1f,
							  WHITE);
				} else {
					gprAssets.noteTopModelHP.materials[0].maps[MATERIAL_MAP_ALBEDO].color = curNote.pTap ? BLACK : NoteColor;
					gprAssets.noteBottomModelHP.materials[0].maps[MATERIAL_MAP_ALBEDO].color = curNote.pTap ? NoteColor : WHITE;
					DrawModel(gprAssets.noteTopModelHP, Vector3{notePosX, 0, player.smasherPos +
																			 (length *
																			  (float) relTime)}, 1.1f,
							  WHITE);
					DrawModel(gprAssets.noteBottomModelHP, Vector3{notePosX, 0, player.smasherPos +
																				(length *
																				 (float) relTime)}, 1.1f,
							  WHITE);
				}
			} else if (curNote.miss && (curNote.phopo || curNote.pTap)) {
				gprAssets.noteTopModelHP.materials[0].maps[MATERIAL_MAP_ALBEDO].color = RED;
				gprAssets.noteBottomModelHP.materials[0].maps[MATERIAL_MAP_ALBEDO].color = RED;
				DrawModel(gprAssets.noteTopModelHP, Vector3{notePosX, 0, player.smasherPos +
																		 (length *
																		  (float) relTime)}, 1.1f,
						  WHITE);
				DrawModel(gprAssets.noteBottomModelHP, Vector3{notePosX, 0, player.smasherPos +
																			(length *
																			 (float) relTime)}, 1.1f,
						  WHITE);
			}
			if ((curNote.len) > 0) {
				int pressedMask = 0b000000;

				for (int pressedButtons = 0; pressedButtons < heldFrets.size(); pressedButtons++) {
					if (heldFrets[pressedButtons] || heldFretsAlt[pressedButtons])
						pressedMask += curChart.PlasticFrets[pressedButtons];
				}

				Note &lastNote = curChart.notes[curNoteInt == 0 ? 0 : curNoteInt - 1];
				bool chordMatch = (extendedSustainActive ? pressedMask >= curNote.mask : pressedMask == curNote.mask);
				bool singleMatch = (extendedSustainActive ? pressedMask >= curNote.mask : pressedMask >= curNote.mask && pressedMask < (curNote.mask * 2));
				bool noteMatch = (curNote.chord ? chordMatch : singleMatch);

				if (curNote.extendedSustain)
					TraceLog(LOG_INFO, "extended sustain lol");

				if ((!noteMatch && curNote.held) && curNote.time + curNote.len + 0.1 > time) {
					curNote.held = false;
					if (curNote.extendedSustain == true)
						extendedSustainActive = false;
				}

				if ((curNote.hit && curNote.held) && curNote.time + curNote.len + 0.1 > time) {
					if (curNote.heldTime < (curNote.len * gprSettings.trackSpeedOptions[gprSettings.trackSpeed])) {
						curNote.heldTime = 0.0 - relTime;
						player.score +=
						        (float) (curNote.heldTime / curNote.len) * (12 * curNote.beatsLen) *
						        player.multiplier(player.instrument);
						if (relTime < 0.0) relTime = 0.0;
					}
					if (relEnd <= 0.0) {
						if (relTime < 0.0) relTime = relEnd;
						curNote.held = false;
						if (curNote.extendedSustain == true)
							extendedSustainActive = false;
					}
				} else if (curNote.hit && !curNote.held) {
					relTime = relTime + curNote.heldTime;
				}
				float sustainLen =
						(length * (float) relEnd) - (length * (float) relTime);
				Matrix sustainMatrix = MatrixMultiply(MatrixScale(1, 1, sustainLen),
													  MatrixTranslate(notePosX, 0.01f,
																	  player.smasherPos +
																	  (length *
																	   (float) relTime) +
																	  (sustainLen / 2.0f)));
				BeginBlendMode(BLEND_ALPHA);
				gprAssets.sustainMat.maps[MATERIAL_MAP_DIFFUSE].color = ColorTint(NoteColor, {180, 180, 180, 255});
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
			}
			if ((!curNote.phopo  && ((curNote.len) > 0 && (curNote.held || !curNote.hit)) ||
				((curNote.len) == 0 && !curNote.hit) && !curNote.phopo && !curNote.pTap) && !curNote.miss) {
				if (curNote.renderAsOD) {
					if ((!curNote.held && !curNote.miss && !curNote.phopo) && !curNote.hit) {
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
					if ((!curNote.held && !curNote.miss && !curNote.pTap && !curNote.phopo) && !curNote.hit) {
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

			} else if ((!curNote.phopo && !curNote.pTap) && curNote.miss) {
				gprAssets.noteTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = RED;
				DrawModel(gprAssets.noteTopModel, Vector3{notePosX, 0, player.smasherPos +
																			   (length *
																				(float) relTime)}, 1.1f,
								  RED);
				DrawModel(gprAssets.noteBottomModel, Vector3{notePosX, 0, player.smasherPos +
																		  (length *
																		   (float) relTime)}, 1.1f,
						  RED);
			}

			if (curNote.miss && curNote.time + 0.5 < time) {
				if (curNote.phopo) {
					DrawModel(gprAssets.noteBottomModelHP,
							  Vector3{notePosX, 0, player.smasherPos + (length * (float) relTime)},
							  1.0f, RED);
					DrawModel(gprAssets.noteTopModelHP,
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
			double HitAnimDuration = 0.15f;
			double PerfectHitAnimDuration = 1.0f;
			if (curNote.hit && gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) <
							   curNote.hitTime + HitAnimDuration) {

				double TimeSinceHit = gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) - curNote.hitTime;
				unsigned char HitAlpha = Remap(getEasingFunction(EaseInBack)(TimeSinceHit/HitAnimDuration), 0, 1.0, 196, 0);

				DrawCube(Vector3{notePosX, 0.125, player.smasherPos}, 1.0f, 0.25f, 0.5f,
						 curNote.perfect ? Color{255, 215, 0, HitAlpha} : Color{255, 255, 255, HitAlpha});
				// if (curNote.perfect) {
				// 	DrawCube(Vector3{3.3f, 0, player.smasherPos}, 1.0f, 0.01f,
				// 			 0.5f, Color{255,161,0,HitAlpha});
				// }


			}
			if (curNote.hit && gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) <
							   curNote.hitTime + PerfectHitAnimDuration && curNote.perfect) {

				double TimeSinceHit = gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) - curNote.hitTime;
				unsigned char HitAlpha = Remap(getEasingFunction(EaseOutQuad)(TimeSinceHit/PerfectHitAnimDuration), 0, 1.0, 255, 0);
				float HitPosLeft = Remap(getEasingFunction(EaseInOutBack)(TimeSinceHit/PerfectHitAnimDuration), 0, 1.0, 3.4, 3.0);

				DrawCube(Vector3{HitPosLeft, -0.1f, player.smasherPos}, 1.0f, 0.01f,
						 0.5f, Color{255,161,0,HitAlpha});
				DrawCube(Vector3{HitPosLeft, -0.11f, player.smasherPos}, 1.0f, 0.01f,
						 1.0f, Color{255,161,0,(unsigned char)(HitAlpha/2)});
			}
		}
	}
	EndMode3D();
	EndTextureMode();

	SetTextureWrap(notes_tex.texture,TEXTURE_WRAP_CLAMP);
	notes_tex.texture.width = (float)GetScreenWidth();
	notes_tex.texture.height = (float)GetScreenHeight();
	DrawTexturePro(notes_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
}


void gameplayRenderer::RenderHud(Player& player, RenderTexture2D& hud_tex, float length) {
	BeginTextureMode(hud_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(camera3pVector[cameraSel]);
	if (showHitwindow) {
		BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
		float lineDistance = player.diff == 3 ? 1.5f : 1.0f;
		float highwayLength = player.defaultHighwayLength * gprSettings.highwayLengthMult;
		float highwayPosShit = ((20) * (1 - gprSettings.highwayLengthMult));
		double hitwindowFront = ((0.1-(player.InputOffset))) * gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);
		double hitwindowBack = ((-0.1-(player.InputOffset))) * gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);
		bool Beginning = (float) (player.smasherPos + (length * hitwindowFront)) >= (length * 1.5f) + player.smasherPos;
		bool Ending = (float) (player.smasherPos + (length * hitwindowBack)) >= (length * 1.5f) + player.smasherPos;

		float Front = (float) (player.smasherPos + (length * hitwindowFront));
		float Back = (float) (player.smasherPos + (length * hitwindowBack));

		DrawTriangle3D({0 - lineDistance - 1.0f, 0.003, Back},
					   {0 - lineDistance - 1.0f, 0.003, Front},
					   {lineDistance + 1.0f, 0.003, Back},
					   Color{96, 96, 96, 64});

		DrawTriangle3D({lineDistance + 1.0f, 0.003, Front},
					   {lineDistance + 1.0f, 0.003, Back},
					   {0 - lineDistance - 1.0f, 0.003, Front},
					   Color{96, 96, 96, 64});

		double perfectFront = ((0.025f-(player.InputOffset))) * gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);
		double perfectBack = ((-0.025f-(player.InputOffset))) * gprSettings.trackSpeedOptions[gprSettings.trackSpeed] * (11.5f / length);
		bool pBeginning = (float) (player.smasherPos + (length * perfectFront)) >= (length * 1.5f) + player.smasherPos;
		bool pEnding = (float) (player.smasherPos + (length * perfectBack)) >= (length * 1.5f) + player.smasherPos;

		float pFront = (float) (player.smasherPos + (length * perfectFront));
		float pBack = (float) (player.smasherPos + (length * perfectBack));

		DrawTriangle3D({0 - lineDistance - 1.0f, 0.006, pBack},
					   {0 - lineDistance - 1.0f, 0.006, pFront},
					   {lineDistance + 1.0f, 0.006, pBack},
					   Color{32, 32, 32, 8});

		DrawTriangle3D({lineDistance + 1.0f, 0.006, pFront},
					   {lineDistance + 1.0f, 0.006, pBack},
					   {0 - lineDistance - 1.0f, 0.006, pFront},
					   Color{32, 32, 32, 8});
		BeginBlendMode(BLEND_ALPHA);
	}
	DrawModel(gprAssets.odFrame, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
	DrawModel(gprAssets.odBar, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
	DrawModel(gprAssets.multFrame, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
	DrawModel(gprAssets.multBar, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
	if (player.instrument == 1 || player.instrument == 3 || player.instrument == 5) {

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
	DrawTexturePro(hud_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
}

void gameplayRenderer::RaiseHighway() {
	if (!highwayInAnimation) {
		startTime = GetTime();
		highwayInAnimation = true;
	}
	if (GetTime() <= startTime + animDuration && highwayInAnimation) {
		double timeSinceStart = GetTime() - startTime;
		highwayLevel = Remap(1.0 - getEasingFunction(EaseOutExpo)(timeSinceStart/animDuration), 0, 1.0, 0, -GetScreenHeight());
		highwayInEndAnim = true;
	}
};

void gameplayRenderer::LowerHighway() {
	if (!highwayOutAnimation) {
		startTime = GetTime();
		highwayOutAnimation = true;
	}
	if (GetTime() <= startTime + animDuration && highwayOutAnimation) {
		double timeSinceStart = GetTime() - startTime;
		highwayLevel = Remap(1.0 - getEasingFunction(EaseInExpo)(timeSinceStart/animDuration), 1.0, 0, 0, -GetScreenHeight());
		highwayOutEndAnim = true;
	}
};

void gameplayRenderer::RenderGameplay(Player& player, double time, Song song, RenderTexture2D& highway_tex, RenderTexture2D& hud_tex, RenderTexture2D& notes_tex, RenderTexture2D& highwayStatus_tex, RenderTexture2D& smasher_tex) {

	Chart& curChart = song.parts[player.instrument]->charts[player.diff];
	float highwayLength = player.defaultHighwayLength * gprSettings.highwayLengthMult;

	float multFill = (!player.overdrive ? (float)(player.multiplier(player.instrument) - 1) : ((float)(player.multiplier(player.instrument) / 2) - 1)) / (float)player.maxMultForMeter(player.instrument);
	SetShaderValue(gprAssets.odMultShader, gprAssets.multLoc, &multFill, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.multNumberShader, gprAssets.uvOffsetXLoc, &player.uvOffsetX, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.multNumberShader, gprAssets.uvOffsetYLoc, &player.uvOffsetY, SHADER_UNIFORM_FLOAT);
	float comboFill = player.comboFillCalc(player.instrument);
	SetShaderValue(gprAssets.odMultShader, gprAssets.comboCounterLoc, &comboFill, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.odMultShader, gprAssets.odLoc, &player.overdriveFill, SHADER_UNIFORM_FLOAT);

	int PlayerComboMax = (player.instrument == 1 || player.instrument == 3 || player.instrument == 5) ? 50 : 30;

	Color highwayColor = ColorContrast(player.accentColor, Clamp(Remap(player.combo, 0, PlayerComboMax, -0.6f, 0.0f), -0.6, 0.0f));

	gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = highwayColor;
	gprAssets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = highwayColor;
	gprAssets.smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = highwayColor;
	gprAssets.smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = highwayColor;

	if (player.overdrive) gprAssets.expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.overdriveColor;
	else if (!bot) gprAssets.expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
	if (bot) gprAssets.expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GOLD;

	if (player.overdrive) gprAssets.emhHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.overdriveColor;
	else if (!bot) gprAssets.emhHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
	if (bot) gprAssets.emhHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GOLD;

	if (bot) player.FC = false;

	if (bot) player.bot = true;
	else player.bot = false;

	RaiseHighway();
	if (GetTime() >= startTime + animDuration && highwayInEndAnim) {
		gprAudioManager.BeginPlayback(gprAudioManager.loadedStreams[0].handle);
		highwayInEndAnim = false;
	}

	/*
	if ((player.overdrive ? player.multiplier(player.instrument) / 2 : player.multiplier(player.instrument))>= (player.instrument == 1 || player.instrument == 3 || player.instrument == 5 ? 6 : 4)) {
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
	 */


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
			player.overdriveActivateTime = musicTime;
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

	if (!curChart.Solos.empty() && curSolo<curChart.Solos.size() - 1 && musicTime-2.5f>curChart.Solos[curSolo].end) {
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

	if (player.diff == 3 || player.plastic) {
		RenderExpertHighway(player, song, time, highway_tex, highwayStatus_tex, smasher_tex);
	} else {
		RenderEmhHighway(player, song, time, highway_tex);
	}
	if (player.plastic) {
		RenderClassicNotes(player, curChart, time, notes_tex, highwayLength);
	} else {
		RenderNotes(player, curChart, time, notes_tex, highwayLength);
	}
	if (!bot) RenderHud(player, hud_tex, highwayLength);
}

void gameplayRenderer::RenderExpertHighway(Player& player, Song song, double time, RenderTexture2D& highway_tex, RenderTexture2D& highwayStatus_tex, RenderTexture2D& smasher_tex)  {
	BeginTextureMode(highway_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(camera3pVector[cameraSel]);

	float diffDistance = 2.0f;
	float lineDistance = 1.5f;

	float highwayLength = player.defaultHighwayLength * gprSettings.highwayLengthMult;
	float highwayPosShit = ((20) * (1 - gprSettings.highwayLengthMult));

	textureOffset += 0.1f;

	//DrawTriangle3D({-diffDistance-0.5f,-0.002,0},{-diffDistance-0.5f,-0.002,(highwayLength *1.5f) + player.smasherPos},{diffDistance+0.5f,-0.002,0},Color{0,0,0,255});
	//DrawTriangle3D({diffDistance+0.5f,-0.002,(highwayLength *1.5f) + player.smasherPos},{diffDistance+0.5f,-0.002,0},{-diffDistance-0.5f,-0.002,(highwayLength *1.5f) + player.smasherPos},Color{0,0,0,255});

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
	unsigned char laneColor = 0;
	unsigned char laneAlpha = 48;
	unsigned char OverdriveAlpha = 255;

	double OverdriveAnimDuration = 0.25f;

	if (gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) <= player.overdriveActiveTime + OverdriveAnimDuration) {
		double TimeSinceOverdriveActivate = gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) - player.overdriveActiveTime;
		OverdriveAlpha = Remap(getEasingFunction(EaseOutQuint)(TimeSinceOverdriveActivate/OverdriveAnimDuration), 0, 1.0, 0, 255);
	} else OverdriveAlpha = 255;

	if (gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) <= player.overdriveActivateTime + OverdriveAnimDuration && gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) > 0.0) {
		double TimeSinceOverdriveActivate = gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) - player.overdriveActivateTime;
		OverdriveAlpha = Remap(getEasingFunction(EaseOutQuint)(TimeSinceOverdriveActivate/OverdriveAnimDuration), 0, 1.0, 255, 0);
	} else if (!player.overdrive) OverdriveAlpha = 0;

	if (player.overdrive || gprAudioManager.GetMusicTimePlayed(gprAudioManager.loadedStreams[0].handle) <= player.overdriveActivateTime + OverdriveAnimDuration) {DrawModel(gprAssets.odHighwayX, Vector3{0,0.001f,0},1,Color{255,255,255,OverdriveAlpha});}
	BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
	DrawTriangle3D({lineDistance - 1.0f,0.003,0},
				   {lineDistance - 1.0f,0.003,(highwayLength *1.5f) + player.smasherPos},
				   {lineDistance,0.003,0},
				   Color{laneColor,laneColor,laneColor,laneAlpha});

	DrawTriangle3D({lineDistance,0.003,(highwayLength *1.5f) + player.smasherPos},
				   {lineDistance,0.003,0},
				   {lineDistance - 1.0f,0.003,(highwayLength *1.5f) + player.smasherPos},
				   Color{laneColor,laneColor,laneColor,laneAlpha});

	//for (int i = 0; i < 4; i++) {
	//    float radius = player.plastic ? 0.02 : ((i == (gprSettings.mirrorMode ? 2 : 1)) ? 0.05 : 0.02);
//
	//    DrawCylinderEx(Vector3{ lineDistance - (float)i, 0, player.smasherPos + 0.5f }, Vector3{ lineDistance - i, 0, (highwayLength *1.5f) + player.smasherPos }, radius, radius, 15, Color{ 128,128,128,128 });
	//}

	DrawTriangle3D({0-lineDistance,0.003,0},
				   {0-lineDistance,0.003,(highwayLength *1.5f) + player.smasherPos},
				   {0-lineDistance + 1.0f,0.003,0},
				   Color{laneColor,laneColor,laneColor,laneAlpha});

	DrawTriangle3D({0-lineDistance + 1.0f,0.003,(highwayLength *1.5f) + player.smasherPos},
				   {0-lineDistance + 1.0f,0.003,0},
				   {0-lineDistance,0.003,(highwayLength *1.5f) + player.smasherPos},
				   Color{laneColor,laneColor,laneColor,laneAlpha});

	if (!player.plastic)
		DrawCylinderEx(Vector3{ lineDistance-1.0f, 0, player.smasherPos}, Vector3{ lineDistance-1.0f, 0, (highwayLength *1.5f) + player.smasherPos }, 0.025f, 0.025f, 15, Color{ 128,128,128,128 });

	EndBlendMode();

	EndMode3D();
	EndTextureMode();

	SetTextureWrap(highway_tex.texture,TEXTURE_WRAP_CLAMP);
	highway_tex.texture.width = (float)GetScreenWidth();
	highway_tex.texture.height = (float)GetScreenHeight();
	DrawTexturePro(highway_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );

	BeginBlendMode(BLEND_ALPHA);
	BeginTextureMode(highwayStatus_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(camera3pVector[cameraSel]);

	if (!song.beatLines.empty()) {
		DrawBeatlines(player, song, highwayLength, time);
	}

	if (!song.parts[player.instrument]->charts[player.diff].odPhrases.empty()) {
		DrawOverdrive(player, song.parts[player.instrument]->charts[player.diff], highwayLength, time);
	}
	if (!song.parts[player.instrument]->charts[player.diff].Solos.empty()) {
		DrawSolo(player, song.parts[player.instrument]->charts[player.diff], highwayLength, time);
	}

	float darkYPos = 0.015f;

	BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
	DrawTriangle3D({-diffDistance-0.5f,darkYPos,player.smasherPos},{-diffDistance-0.5f,darkYPos,(highwayLength *1.5f) + player.smasherPos},{diffDistance+0.5f,darkYPos,player.smasherPos},Color{0,0,0,64});
	DrawTriangle3D({diffDistance+0.5f,darkYPos,(highwayLength *1.5f) + player.smasherPos},{diffDistance+0.5f,darkYPos,player.smasherPos},{-diffDistance-0.5f,darkYPos,(highwayLength *1.5f) + player.smasherPos},Color{0,0,0,64});



	EndBlendMode();
	EndMode3D();
	EndTextureMode();
	SetTextureWrap(highwayStatus_tex.texture,TEXTURE_WRAP_CLAMP);
	highwayStatus_tex.texture.width = (float)GetScreenWidth();
	highwayStatus_tex.texture.height = (float)GetScreenHeight();
	DrawTexturePro(highwayStatus_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );

	BeginTextureMode(smasher_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(camera3pVector[cameraSel]);
	BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
	DrawModel(gprAssets.smasherBoard, Vector3{ 0, 0.004f, 0 }, 1.0f, WHITE);
	BeginBlendMode(BLEND_ALPHA);
	for (int i = 0; i < 5;  i++) {
		Color NoteColor; // = gprMenu.hehe && player.diff == 3 ? i == 0 || i == 4 ? SKYBLUE : i == 1 || i == 3 ? PINK : WHITE : player.accentColor;
		int noteColor = gprSettings.mirrorMode ? 4 - i : i;
		if (player.plastic) {
			bool pd = (player.instrument == 4);

			switch (noteColor) {
				case 0:
					NoteColor = gprMenu.hehe ? SKYBLUE : GREEN;
					break;
				case 1:
					NoteColor = gprMenu.hehe ? PINK :RED;
					break;
				case 2:
					NoteColor = gprMenu.hehe ? RAYWHITE :YELLOW;
					break;
				case 3:
					NoteColor = gprMenu.hehe ? PINK :BLUE;
					break;
				case 4:
					NoteColor = gprMenu.hehe ? SKYBLUE :ORANGE;
					break;
				default:
					NoteColor = player.accentColor;
					break;
			}
		}   else {
			NoteColor = gprMenu.hehe ? (i == 0 || i == 4 ? SKYBLUE : (i == 1 || i == 3 ? PINK : WHITE)) : player.accentColor;
		}

		gprAssets.smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
		gprAssets.smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;

		if (heldFrets[noteColor] || heldFretsAlt[noteColor]) {
			DrawModel(gprAssets.smasherPressed, Vector3{ diffDistance - (float)(i), 0.01f, player.smasherPos }, 1.0f, WHITE);
		}
		else {
			DrawModel(gprAssets.smasherReg, Vector3{ diffDistance - (float)(i), 0.01f, player.smasherPos }, 1.0f, WHITE);
		}
	}
	//DrawModel(gprAssets.lanes, Vector3 {0,0.1f,0}, 1.0f, WHITE);

	EndBlendMode();
	EndMode3D();
	EndTextureMode();
	SetTextureWrap(smasher_tex.texture,TEXTURE_WRAP_CLAMP);
	smasher_tex.texture.width = (float)GetScreenWidth();
	smasher_tex.texture.height = (float)GetScreenHeight();
	DrawTexturePro(smasher_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );




}

void gameplayRenderer::RenderEmhHighway(Player& player, Song song, double time, RenderTexture2D &highway_tex) {
	BeginTextureMode(highway_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(camera3pVector[cameraSel]);

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
	DrawTexturePro(highway_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {0,highwayLevel}, 0, WHITE );

}

void gameplayRenderer::DrawBeatlines(Player& player, Song song, float length, double musicTime) {
	float diffDistance = player.diff == 3 || player.plastic ? 2.0f : 1.5f;
	float lineDistance = player.diff == 3 || player.plastic ? 1.5f : 1.0f;
	std::vector<std::pair<double, bool>> beatlines = song.beatLines;

	if (beatlines.size() >= 0) {
		for (int i = curBeatLine; i < beatlines.size(); i++) {
			if (beatlines[i].first >= song.music_start-1 && beatlines[i].first <= song.end) {
				Color BeatLineColor = {255,255,255,128};
				double relTime = ((song.beatLines[i].first - musicTime)) * gprSettings.trackSpeedOptions[gprSettings.trackSpeed]  * ( 11.5f / length);
				if (i > 0) {
					double secondLine = ((((song.beatLines[i-1].first + song.beatLines[i].first)/2) - musicTime)) * gprSettings.trackSpeedOptions[gprSettings.trackSpeed]  * ( 11.5f / length);
					if (secondLine > 1.5) break;

					if (song.beatLines[i].first - song.beatLines[i-1].first <= 0.2 || (player.smasherPos + (length * (float)relTime)) - (player.smasherPos + (length * (float)secondLine)) <= 1.5) BeatLineColor = {0,0,0,0};
					else BeatLineColor = {128,128,128,128};
					DrawCylinderEx(Vector3{ -diffDistance - 0.5f,0,player.smasherPos + (length * (float)secondLine) },
								   Vector3{ diffDistance + 0.5f,0,player.smasherPos + (length * (float)secondLine) },
								   0.01f,
								   0.01f,
								   4,
								   BeatLineColor);
				}

				if (relTime > 1.5) break;
				float radius = beatlines[i].second ? 0.06f : 0.03f;

				BeatLineColor = (beatlines[i].second) ? Color{ 255, 255, 255, 196 } : Color{ 255, 255, 255, 128 };

				DrawCylinderEx(Vector3{ -diffDistance - 0.5f,0,player.smasherPos + (length * (float)relTime) },
							   Vector3{ diffDistance + 0.5f,0,player.smasherPos + (length * (float)relTime) },
							   radius,
							   radius,
							   4,
							   BeatLineColor);


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
	float RightSideX = player.diff == 3 || player.plastic ? 2.7f : 2.2f;

	Vector3 RightSideStart = {RightSideX ,0,Beginning ?  HighwayEnd : (float)(player.smasherPos + (length * odStart)) };
	Vector3 RightSideEnd = { RightSideX,0, Ending ? HighwayEnd : (float)(player.smasherPos + (length * odEnd)) };

	// left calc
	float LeftSideX = player.diff == 3 || player.plastic ? -2.7f : -2.2f;

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

	float soloPlaneStart = Beginning ?  HighwayEnd : (float)(player.smasherPos + (length * soloStart));
	float soloPlaneEnd = Ending ? HighwayEnd : (float)(player.smasherPos + (length * soloEnd));

	//float soloLen = (length * (float) soloPlaneEnd) - (length * (float) soloPlaneStart);
	// Matrix soloMatrix = MatrixMultiply(MatrixScale(1, 1, soloLen),
	//                                   MatrixTranslate(0, 0.005f,
//                                                       (length *
	//                                                    (float) soloStart) +
	//                                                   (soloLen / 2.0f)));
	//gprAssets.soloMat.maps[MATERIAL_MAP_DIFFUSE].color = SKYBLUE;

	// DrawMesh(soloPlane, gprAssets.soloMat, soloMatrix);


	// right calc
	float RightSideX = player.diff == 3 || player.plastic ? 2.7f : 2.2f;

	Vector3 RightSideStart = {RightSideX ,-0.0025,Beginning ?  HighwayEnd : (float)(player.smasherPos + (length * soloStart)) };
	Vector3 RightSideEnd = { RightSideX,-0.0025, Ending ? HighwayEnd : (float)(player.smasherPos + (length * soloEnd)) };

	// left calc
	float LeftSideX = player.diff == 3 || player.plastic ? -2.7f : -2.2f;

	Vector3 LeftSideStart = {LeftSideX ,-0.0025,Beginning ?  HighwayEnd : (float)(player.smasherPos + (length * soloStart)) };
	Vector3 LeftSideEnd = { LeftSideX,-0.0025, Ending ? HighwayEnd : (float)(player.smasherPos + (length * soloEnd)) };

	// draw
	DrawCylinderEx(RightSideStart, RightSideEnd, 0.07, 0.07, 10, SKYBLUE);
	DrawCylinderEx(LeftSideStart, LeftSideEnd, 0.07, 0.07, 10, SKYBLUE);
}
