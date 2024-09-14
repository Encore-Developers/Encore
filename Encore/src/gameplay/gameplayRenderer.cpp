//
// Created by marie on 09/06/2024.
//

#include "gameplayRenderer.h"

#include "assets.h"
#include "settings.h"
#include "menus/gameMenu.h"
#include "raymath.h"
#include "menus/uiUnits.h"
#include "rlgl.h"
#include "easing/easing.h"
#include "song/audio.h"
#include "users/player.h"

Assets &gprAssets = Assets::getInstance();
Settings& gprSettings = Settings::getInstance();
AudioManager &gprAudioManager = AudioManager::getInstance();
GameMenu& gprMenu = TheGameMenu;
PlayerManager &gprPlayerManager = PlayerManager::getInstance();
Units& gprU = Units::getInstance();

// Color accentColor = {255,0,255,255};
float defaultHighwayLength = 11.5f;
Color OverdriveColor = {255,200,0,255};



#define LETTER_BOUNDRY_SIZE     0.25f
#define TEXT_MAX_LAYERS         32
#define LETTER_BOUNDRY_COLOR    VIOLET

std::vector<Color> GRYBO = {GREEN, RED, YELLOW, BLUE, ORANGE};
std::vector<Color> TRANS = {SKYBLUE, PINK, WHITE, PINK, SKYBLUE};

enum DrumNotes {
	KICK,
	RED_DRUM,
	YELLOW_DRUM,
	BLUE_DRUM,
	GREEN_DRUM
};

bool SHOW_LETTER_BOUNDRY = false;
bool SHOW_TEXT_BOUNDRY = false;

// code from examples lol
static void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scale = fontSize/(float)font.baseSize;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding)/(float)font.baseSize*scale;
    position.z += (float)(font.glyphs[index].offsetY - font.glyphPadding)/(float)font.baseSize*scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f*font.glyphPadding, font.recs[index].height + 2.0f*font.glyphPadding };

    float width = (float)(font.recs[index].width + 2.0f*font.glyphPadding)/(float)font.baseSize*scale;
    float height = (float)(font.recs[index].height + 2.0f*font.glyphPadding)/(float)font.baseSize*scale;

    if (font.texture.id > 0)
    {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture (0.0f -> 1.0f)
        const float tx = srcRec.x/font.texture.width;
        const float ty = srcRec.y/font.texture.height;
        const float tw = (srcRec.x+srcRec.width)/font.texture.width;
        const float th = (srcRec.y+srcRec.height)/font.texture.height;

        if (SHOW_LETTER_BOUNDRY) {
	        DrawCubeWiresV(Vector3{ position.x + width/2, position.y, position.z + height/2}, Vector3{ width, LETTER_BOUNDRY_SIZE, height }, LETTER_BOUNDRY_COLOR);
        }

        rlCheckRenderBatchLimit(4 + 4*backface);
        rlSetTexture(font.texture.id);

        rlPushMatrix();
            rlTranslatef(position.x, position.y, position.z);

            rlBegin(RL_QUADS);
                rlColor4ub(tint.r, tint.g, tint.b, tint.a);

                // Front Face
                rlNormal3f(0.0f, 1.0f, 0.0f);                                   // Normal Pointing Up
                rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);              // Top Left Of The Texture and Quad
                rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height);     // Bottom Left Of The Texture and Quad
                rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height);     // Bottom Right Of The Texture and Quad
                rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);              // Top Right Of The Texture and Quad

                if (backface)
                {
                    // Back Face
                    rlNormal3f(0.0f, -1.0f, 0.0f);                              // Normal Pointing Down
                    rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);          // Top Right Of The Texture and Quad
                    rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);          // Top Left Of The Texture and Quad
                    rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height); // Bottom Left Of The Texture and Quad
                    rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height); // Bottom Right Of The Texture and Quad
                }
            rlEnd();
        rlPopMatrix();

        rlSetTexture(0);
    }
}

static void DrawText3D(Font font, const char *text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint)
{
	int length = TextLength(text);          // Total length in bytes of the text, scanned by codepoints in loop

	float textOffsetY = 0.0f;               // Offset between lines (on line break '\n')
	float textOffsetX = 0.0f;               // Offset X to next character to draw

	float scale = fontSize/(float)font.baseSize;

	for (int i = 0; i < length;)
	{
		// Get next codepoint from byte string and glyph index in font
		int codepointByteCount = 0;
		int codepoint = GetCodepoint(&text[i], &codepointByteCount);
		int index = GetGlyphIndex(font, codepoint);

		// NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
		// but we need to draw all of the bad bytes using the '?' symbol moving one byte
		if (codepoint == 0x3f) codepointByteCount = 1;

		if (codepoint == '\n')
		{
			// NOTE: Fixed line spacing of 1.5 line-height
			// TODO: Support custom line spacing defined by user
			textOffsetY += scale + lineSpacing/(float)font.baseSize*scale;
			textOffsetX = 0.0f;
		}
		else
		{
			if ((codepoint != ' ') && (codepoint != '\t'))
			{
				DrawTextCodepoint3D(font, codepoint, Vector3{ position.x + textOffsetX, position.y, position.z + textOffsetY }, fontSize, backface, tint);
			}

			if (font.glyphs[index].advanceX == 0) textOffsetX += (float)(font.recs[index].width + fontSpacing)/(float)font.baseSize*scale;
			else textOffsetX += (float)(font.glyphs[index].advanceX + fontSpacing)/(float)font.baseSize*scale;
		}

		i += codepointByteCount;   // Move text bytes counter to next codepoint
	}
}

static Vector3 MeasureText3D(Font font, const char* text, float fontSize, float fontSpacing, float lineSpacing)
{
	int len = TextLength(text);
	int tempLen = 0;                // Used to count longer text line num chars
	int lenCounter = 0;

	float tempTextWidth = 0.0f;     // Used to count longer text line width

	float scale = fontSize/(float)font.baseSize;
	float textHeight = scale;
	float textWidth = 0.0f;

	int letter = 0;                 // Current character
	int index = 0;                  // Index position in sprite font

	for (int i = 0; i < len; i++)
	{
		lenCounter++;

		int next = 0;
		letter = GetCodepoint(&text[i], &next);
		index = GetGlyphIndex(font, letter);

		// NOTE: normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
		// but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
		if (letter == 0x3f) next = 1;
		i += next - 1;

		if (letter != '\n')
		{
			if (font.glyphs[index].advanceX != 0) textWidth += (font.glyphs[index].advanceX+fontSpacing)/(float)font.baseSize*scale;
			else textWidth += (font.recs[index].width + font.glyphs[index].offsetX)/(float)font.baseSize*scale;
		}
		else
		{
			if (tempTextWidth < textWidth) tempTextWidth = textWidth;
			lenCounter = 0;
			textWidth = 0.0f;
			textHeight += scale + lineSpacing/(float)font.baseSize*scale;
		}

		if (tempLen < lenCounter) tempLen = lenCounter;
	}

	if (tempTextWidth < textWidth) tempTextWidth = textWidth;

	Vector3 vec = { 0 };
	vec.x = tempTextWidth + (float)((tempLen - 1)*fontSpacing/(float)font.baseSize*scale); // Adds chars spacing to measure
	vec.y = 0.25f;
	vec.z = textHeight;

	return vec;
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
		highwayLevel = Remap(1.0 - getEasingFunction(EaseInExpo)(timeSinceStart/animDuration), 0, 1.0, -GetScreenHeight(), 0);
		highwayOutEndAnim = true;
	}
};

void gameplayRenderer::NoteMultiplierEffect(double time, double hitTime, bool miss, Player *player) {
	if (time < hitTime + multiplierEffectTime && time > hitTime) {
		Color missColor = {200,0,0,255};
		Color comboColor = {200,200,200,255};

		Color RingDefault = ColorBrightness(player->AccentColor, -0.7);
		unsigned char r = RingDefault.r;
		unsigned char g = RingDefault.g;
		unsigned char b = RingDefault.b;
		unsigned char a = 255;
		double TimeSinceHit = time - hitTime;
		if (!miss) {
			if (player->stats->Combo <= player->stats->maxMultForMeter() * 10 && player->stats->Combo % 10 == 0) {
				r = Remap(getEasingFunction(EaseOutQuart)(TimeSinceHit/multiplierEffectTime), 0, 1.0, comboColor.r, RingDefault.r);
				g = Remap(getEasingFunction(EaseOutQuart)(TimeSinceHit/multiplierEffectTime), 0, 1.0, comboColor.g, RingDefault.g);
				b = Remap(getEasingFunction(EaseOutQuart)(TimeSinceHit/multiplierEffectTime), 0, 1.0, comboColor.b, RingDefault.b);
			}
		} else {
			if (player->stats->Combo != 0){
				r = Remap(getEasingFunction(EaseOutQuart)(TimeSinceHit/multiplierEffectTime), 0, 1.0, missColor.r, RingDefault.r);
				g = Remap(getEasingFunction(EaseOutQuart)(TimeSinceHit/multiplierEffectTime), 0, 1.0, missColor.g, RingDefault.g);
				b = Remap(getEasingFunction(EaseOutQuart)(TimeSinceHit/multiplierEffectTime), 0, 1.0, missColor.b, RingDefault.b);
			}
		}
		gprAssets.MultOuterFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].color = {r,g,b,a};
	}
}

void gameplayRenderer::RenderNotes(Player* player, Chart& curChart, double time, RenderTexture2D &notes_tex, float length) {
	float diffDistance = player->Difficulty == 3 ? 2.0f : 1.5f;
	float lineDistance = player->Difficulty == 3 ? 1.5f : 1.0f;
	float DiffMultiplier = Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
	BeginTextureMode(notes_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel]);
	// glDisable(GL_CULL_FACE);
	for (int lane = 0; lane < (player->Difficulty == 3 ? 5 : 4); lane++) {
		for (int i = player->stats->curNoteIdx[lane]; i < curChart.notes_perlane[lane].size(); i++) {

			Color NoteColor;
			if (player->ClassicMode) {
				NoteColor = GRYBO[lane];
			}   else {
				NoteColor = gprMenu.hehe && player->Difficulty == 3 ? TRANS[lane] : player->AccentColor;
			}

			Note & curNote = curChart.notes[curChart.notes_perlane[lane][i]];
			//if (curNote.hit) {
			//	player->stats->totalOffset += curNote.HitOffset;
			//}

			if (!curChart.Solos.empty()) {
				if (curNote.time >= curChart.Solos[player->stats->curSolo].start &&
					curNote.time <= curChart.Solos[player->stats->curSolo].end) {
					if (curNote.hit) {
						if (curNote.hit && !curNote.countedForSolo) {
							curChart.Solos[player->stats->curSolo].notesHit++;
							curNote.countedForSolo = true;
						}
					}
				}
			}
			if (!curChart.odPhrases.empty()) {

				if (curNote.time >= curChart.odPhrases[player->stats->curODPhrase].start &&
					curNote.time < curChart.odPhrases[player->stats->curODPhrase].end &&
					!curChart.odPhrases[player->stats->curODPhrase].missed) {
					if (curNote.hit) {
						if (curNote.hit && !curNote.countedForODPhrase) {
							curChart.odPhrases[player->stats->curODPhrase].notesHit++;
							curNote.countedForODPhrase = true;
						}
					}
					curNote.renderAsOD = true;

					}
				if (curChart.odPhrases[player->stats->curODPhrase].missed) {
					curNote.renderAsOD = false;
				}
				if (curChart.odPhrases[player->stats->curODPhrase].notesHit ==
					curChart.odPhrases[player->stats->curODPhrase].noteCount &&
					!curChart.odPhrases[player->stats->curODPhrase].added && player->stats->overdriveFill < 1.0f) {
					player->stats->overdriveFill += 0.25f;
					if (player->stats->overdriveFill > 1.0f) player->stats->overdriveFill = 1.0f;
					if (player->stats->Overdrive) {
						player->stats->overdriveActiveFill = player->stats->overdriveFill;
						player->stats->overdriveActiveTime = time;
					}
					curChart.odPhrases[player->stats->curODPhrase].added = true;
					}
			}
			if (!curNote.hit && !curNote.accounted && curNote.time + goodBackend < time && !songEnded) {
				curNote.miss = true;
				player->stats->MissNote();
				if (!curChart.odPhrases.empty() && !curChart.odPhrases[player->stats->curODPhrase].missed &&
					curNote.time >= curChart.odPhrases[player->stats->curODPhrase].start &&
					curNote.time < curChart.odPhrases[player->stats->curODPhrase].end) {
					curChart.odPhrases[player->stats->curODPhrase].missed = true;
				};
				if (!curChart.Sections.empty()) {
					curChart.Sections[player->stats->curSection].totalNotes++;
					curNote.countedForSection = true;
				}
				player->stats->Combo = 0;
				curNote.accounted = true;
			} else if (player->Bot) {
				if (!curNote.hit && !curNote.accounted && curNote.time < time &&
					player->stats->curNoteInt < curChart.notes.size() && !songEnded) {
					curNote.hit = true;
					player->stats->HitNote(false);
					if (gprPlayerManager.BandStats.Multiplayer) {
						gprPlayerManager.BandStats.AddNotePoint(curNote.perfect, player->stats->noODmultiplier());
					}
					if (curNote.len > 0) curNote.held = true;
					curNote.accounted = true;
					// player->stats->Notes += 1;
					// player->stats->Combo++;
					curNote.accounted = true;
					curNote.hitTime = time;
					}
			}
			if (!curChart.Sections.empty()) {
				if (curNote.time >= curChart.Sections[player->stats->curSection].Start &&
					curNote.time < curChart.Sections[player->stats->curSection].End) {
					if (!curNote.countedForSection) {
						if (curNote.hit) {
							curChart.Sections[player->stats->curSection].notesHit++;
							curChart.Sections[player->stats->curSection].totalNotes++;
							curNote.countedForSection = true;
						}
					}
					}
			}

			double relTime = GetNoteOnScreenTime(curNote.time, time, player->NoteSpeed, player->Difficulty, length);
			double relEnd = GetNoteOnScreenTime(curNote.time + curNote.len, time, player->NoteSpeed, player->Difficulty, length);
			float notePosX = diffDistance - (1.0f *
											 (float) (player->LeftyFlip ? (player->Difficulty == 3 ? 4 : 3) -
																			   curNote.lane
																			 : curNote.lane));
			if (relTime > 1.5) {
				break;
			}
			if (relEnd > 1.5) relEnd = 1.5;
			if (relEnd < -1) continue;
			// Vector3 NotePos = {notePosX, 0, smasherPos + (length * (float) relTime)};
			float noteScrollPos = smasherPos + (length * (float) relTime);

			nDrawPadNote(curNote, NoteColor, notePosX, noteScrollPos);

			if ((curNote.len) > 0) {
					if (curNote.hit && curNote.held) {
						if (curNote.heldTime <(curNote.len * player->NoteSpeed)) {
							curNote.heldTime = 0.0 - relTime;
							// note: this was old sustain scoring code
							//if (!bot) {
							//player.sustainScoreBuffer[curNote.lane] =
							//		(float) (curNote.heldTime / curNote.len) * (12 * curNote.beatsLen) *
							//		player.multiplier(player->Instrument);
							//}
							if (relTime < 0.0) relTime = 0.0;
							}
						if (relEnd <= 0.0) {
							if (relTime < 0.0) relTime = relEnd;
							//if (!bot) {
							//player.score += player.sustainScoreBuffer[curNote.lane];
							//player.sustainScoreBuffer[curNote.lane] = 0;
							//}
							curNote.held = false;
						}
					} else if (curNote.hit && !curNote.held) {
						relTime = relTime + curNote.heldTime;
					}
				float sustainLen = (length * (float) relEnd) - (length * (float) relTime);
				Matrix sustainMatrix = MatrixMultiply(MatrixScale(1, 1, sustainLen),
													  MatrixTranslate(notePosX, 0.01f,
																	  smasherPos +
																	  (length *
																	   (float) relTime) +
																	  (sustainLen / 2.0f)));

				nDrawSustain(curNote, NoteColor, notePosX, sustainMatrix);


			}

			nDrawFiveLaneHitEffects(curNote, time, notePosX);
			NoteMultiplierEffect(time, curNote.time, curNote.miss, player);

			if (relEnd < -1 && player->stats->curNoteIdx[lane] < curChart.notes_perlane[lane].size() - 1)
				player->stats->curNoteIdx[lane] = i + 1;
		}
	}
	EndMode3D();
	EndTextureMode();
	SetTextureWrap(notes_tex.texture,TEXTURE_WRAP_CLAMP);
	notes_tex.texture.width = (float)GetScreenWidth();
	notes_tex.texture.height = (float)GetScreenHeight();
	Vector2 res = {(float)GetScreenWidth(), (float)GetScreenHeight()};
	SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, notes_tex.texture);
	SetShaderValue(gprAssets.fxaa, gprAssets.resLoc, &res, SHADER_UNIFORM_VEC2);
	BeginShaderMode(gprAssets.fxaa);
	DrawTexturePro(notes_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
	EndShaderMode();
}

void gameplayRenderer::RenderClassicNotes(Player* player, Chart& curChart, double time, RenderTexture2D &notes_tex, float length) {
	float diffDistance = 2.0f;
	float lineDistance = 1.5f;
	float DiffMultiplier = Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
	BeginTextureMode(notes_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel]);
	// glDisable(GL_CULL_FACE);
	SongList& songList = SongList::getInstance();

	for (auto & curNote : curChart.notes) {

		if (!curChart.Solos.empty()) {
			if (curNote.time >= curChart.Solos[player->stats->curSolo].start &&
				curNote.time < curChart.Solos[player->stats->curSolo].end) {
				if (curNote.hit) {
					if (curNote.hit && !curNote.countedForSolo) {
						curChart.Solos[player->stats->curSolo].notesHit++;
						curNote.countedForSolo = true;
					}
				}
			}
		}


		if (!curChart.odPhrases.empty()) {

			if (curNote.time >= curChart.odPhrases[player->stats->curODPhrase].start &&
				curNote.time < curChart.odPhrases[player->stats->curODPhrase].end &&
				!curChart.odPhrases[player->stats->curODPhrase].missed) {
				if (curNote.hit) {
					if (curNote.hit && !curNote.countedForODPhrase) {
						curChart.odPhrases[player->stats->curODPhrase].notesHit++;
						curNote.countedForODPhrase = true;
					}
				}
				curNote.renderAsOD = true;

			}
			if (curChart.odPhrases[player->stats->curODPhrase].missed) {
				curNote.renderAsOD = false;
			}
			if (curChart.odPhrases[player->stats->curODPhrase].notesHit ==
				curChart.odPhrases[player->stats->curODPhrase].noteCount &&
				!curChart.odPhrases[player->stats->curODPhrase].added && player->stats->overdriveFill < 1.0f) {
				player->stats->overdriveFill += 0.25f;
				if (player->stats->overdriveFill > 1.0f) player->stats->overdriveFill = 1.0f;
				if (player->stats->Overdrive) {
					player->stats->overdriveActiveFill = player->stats->overdriveFill;
					player->stats->overdriveActiveTime = time;
				}
				curChart.odPhrases[player->stats->curODPhrase].added = true;
			}
		}

		if (!curNote.hit && !curNote.accounted && curNote.time + goodBackend < time &&
			!songEnded && player->stats->curNoteInt < curChart.notes.size() && !songEnded && !player->Bot) {
			TraceLog(LOG_INFO, TextFormat("Missed note at %f, note %01i", time, player->stats->curNoteInt));
			curNote.miss = true;
			player->stats->FAS = false;
			player->stats->MissNote();
			if (!curChart.odPhrases.empty() && !curChart.odPhrases[player->stats->curODPhrase].missed &&
				curNote.time >= curChart.odPhrases[player->stats->curODPhrase].start &&
				curNote.time < curChart.odPhrases[player->stats->curODPhrase].end)
				curChart.odPhrases[player->stats->curODPhrase].missed = true;
			if (!curChart.Sections.empty()) {
				curChart.Sections[player->stats->curSection].totalNotes++;
				curNote.countedForSection = true;
			}
			player->stats->Combo = 0;
			curNote.accounted = true;
			player->stats->curNoteInt++;
		} else if (player->Bot) {
			if (!curNote.hit && !curNote.accounted && curNote.time < time && player->stats->curNoteInt < curChart.notes.size() && !songEnded) {
				curNote.hit = true;
				player->stats->HitPlasticNote(curNote);
				gprPlayerManager.BandStats.AddClassicNotePoint(curNote.perfect, player->stats->noODmultiplier(), curNote.chordSize);
				// player->stats->Notes++;
				if (curNote.len > 0) {
					curNote.held = true;
				}
				curNote.accounted = true;
				// player->stats->Combo++;
				curNote.accounted = true;
				curNote.hitTime = time;
				player->stats->curNoteInt++;
			}
		}

		if (!curChart.Sections.empty()) {
			if (curNote.time >= curChart.Sections[player->stats->curSection].Start &&
				curNote.time < curChart.Sections[player->stats->curSection].End) {
				if (!curNote.countedForSection) {
					if (curNote.hit) {
						curChart.Sections[player->stats->curSection].notesHit++;
						curChart.Sections[player->stats->curSection].totalNotes++;
						curNote.countedForSection = true;
					}
				}
			}
		}

		double relTime = GetNoteOnScreenTime(curNote.time, time, player->NoteSpeed, player->Difficulty, length);
		double relEnd = GetNoteOnScreenTime(curNote.time + curNote.len, time, player->NoteSpeed, player->Difficulty, length);
		if (relEnd < -1) continue;
		for (int lane: curNote.pLanes) {


			int noteLane = gprSettings.mirrorMode ? 4 - lane : lane;

			Color NoteColor = gprMenu.hehe ? TRANS[lane] : GRYBO[lane];

			gprAssets.noteTopModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
			gprAssets.noteTopModelHP.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
			gprAssets.noteTopModel.meshes->colors = (unsigned char*)ColorToInt(NoteColor);
			gprAssets.noteTopModelHP.meshes->colors = (unsigned char*)ColorToInt(NoteColor);

			float notePosX = diffDistance - (1.0f * noteLane);
			if (relTime > 1.5) {
				break;
			}
			if (relEnd > 1.5) relEnd = 1.5;

			float NoteScroll = smasherPos + (length * (float)relTime);

			nDrawPlasticNote(curNote, NoteColor, notePosX, NoteScroll);

			// todo: REMOVE SUSTAIN CHECK FROM RENDERER
			// todo: PLEASE REMOVE SUSTAIN CHECK FROM RENDERER
			// todo: PLEASE REMOVE LOGIC FROM RENDERER PLEASE :sob:
			if ((curNote.len) > 0) {
				int pressedMask = 0b000000;

				for (int pressedButtons = 0; pressedButtons < player->stats->HeldFrets.size(); pressedButtons++) {
					if (player->stats->HeldFrets[pressedButtons] || player->stats->HeldFretsAlt[pressedButtons])
						pressedMask += curChart.PlasticFrets[pressedButtons];
				}

				Note &lastNote = curChart.notes[player->stats->curNoteInt == 0 ? 0 : player->stats->curNoteInt - 1];
				bool chordMatch = (extendedSustainActive ? pressedMask >= curNote.mask : pressedMask == curNote.mask);
				bool singleMatch = (extendedSustainActive ? pressedMask >= curNote.mask : pressedMask >= curNote.mask && pressedMask < (curNote.mask * 2));

				bool noteMatch = (curNote.chord ? chordMatch : singleMatch);

				if (curNote.extendedSustain)
					TraceLog(LOG_INFO, "extended sustain lol");

				if ((!noteMatch && curNote.held) && curNote.time + curNote.len + 0.1 > time && !player->Bot) {
					curNote.held = false;
					if (curNote.extendedSustain == true)
						extendedSustainActive = false;
				}

				if ((curNote.hit && curNote.held) && curNote.time + curNote.len + 0.1 > time) {
					if (curNote.heldTime < (curNote.len * (player->NoteSpeed * DiffMultiplier))) {
						curNote.heldTime = 0.0 - relTime;
						//player->stats->Score +=
						//        (float) (curNote.heldTime / curNote.len) * (12 * curNote.beatsLen) *
						 //       player->stats->multiplier();
						if (relTime < 0.0) relTime = 0.0;
					}
					if (relEnd <= 0.0) {
						if (relTime < 0.0) relTime = relEnd;
						curNote.held = false;
						if (curNote.extendedSustain == true)
							extendedSustainActive = false;
					}
				} else if (curNote.hit && !curNote.held && !player->Bot) {
					relTime = relTime + curNote.heldTime;
				}
				float sustainLen = (length * (float) relEnd) - (length * (float) relTime);
				Matrix sustainMatrix = MatrixMultiply(MatrixScale(1, 1, sustainLen),
													  MatrixTranslate(notePosX, 0.01f,
																	  smasherPos +
																	  (length *
																	   (float) relTime) +
																	  (sustainLen / 2.0f)));

				nDrawSustain(curNote, NoteColor, notePosX, sustainMatrix);
			}
			nDrawFiveLaneHitEffects(curNote, time, notePosX);
			NoteMultiplierEffect(time, curNote.time, curNote.miss, player);
		}
	}
	EndMode3D();
	EndTextureMode();
	SetTextureWrap(notes_tex.texture,TEXTURE_WRAP_CLAMP);
	notes_tex.texture.width = (float)GetScreenWidth();
	notes_tex.texture.height = (float)GetScreenHeight();
	Vector2 res = {(float)GetScreenWidth(), (float)GetScreenHeight()};
	SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, notes_tex.texture);
	SetShaderValue(gprAssets.fxaa, gprAssets.resLoc, &res, SHADER_UNIFORM_VEC2);
	BeginShaderMode(gprAssets.fxaa);
	DrawTexturePro(notes_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
	EndShaderMode();
}

void gameplayRenderer::RenderHud(Player* player, RenderTexture2D& hud_tex, float length) {
	BeginTextureMode(hud_tex);
	float DiffMultiplier = Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
	ClearBackground({0,0,0,0});
	BeginMode3D(cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel]);
	if (showHitwindow) {
		BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
		float lineDistance = player->Difficulty == 3 ? 1.5f : 1.0f;
		double hitwindowFront = ((0.1)) * (player->NoteSpeed * DiffMultiplier) * (11.5f / length);
		double hitwindowBack = ((-0.1)) * (player->NoteSpeed * DiffMultiplier) * (11.5f / length);

		float Front = (float) (smasherPos + (length * hitwindowFront));
		float Back = (float) (smasherPos + (length * hitwindowBack));

		DrawTriangle3D({0 - lineDistance - 1.0f, 0.003, Back},
					   {0 - lineDistance - 1.0f, 0.003, Front},
					   {lineDistance + 1.0f, 0.003, Back},
					   Color{96, 96, 96, 64});

		DrawTriangle3D({lineDistance + 1.0f, 0.003, Front},
					   {lineDistance + 1.0f, 0.003, Back},
					   {0 - lineDistance - 1.0f, 0.003, Front},
					   Color{96, 96, 96, 64});

		double perfectFront = ((0.025f)) * (player->NoteSpeed * DiffMultiplier) * (11.5f / length);
		double perfectBack = ((-0.025f)) * (player->NoteSpeed * DiffMultiplier) * (11.5f / length);

		float pFront = (float) (smasherPos + (length * perfectFront));
		float pBack = (float) (smasherPos + (length * perfectBack));

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
	DrawModel(gprAssets.odFrame, Vector3{ 0,0.0f,1.1f  }, 1.0f, WHITE);
	DrawModel(gprAssets.odBar, Vector3{ 0,0.0f,1.1f  }, 1.0f, WHITE);

	float FillPct = player->stats->comboFillCalc();
	Vector4 MultFillColor{0.8,0.8,0.8,1};

	if (player->stats->IsBassOrVox()) {
		if (player->stats->noODmultiplier() >= 6) {
			MultFillColor = {0.2,0.6,1,1};
		}
	} else {
		if (player->stats->noODmultiplier() >= 4) {
			MultFillColor = {0.2,0.6,1,1};
		}
	}

	SetShaderValue(gprAssets.MultiplierFill, gprAssets.FillPercentageLoc, &FillPct, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.MultiplierFill, gprAssets.MultiplierColorLoc, &MultFillColor, SHADER_UNIFORM_VEC4);
	SetShaderValueTexture(gprAssets.MultiplierFill, gprAssets.MultTextureLoc, gprAssets.MultFillBase);

	gprAssets.MultInnerDot.materials[0].maps[MATERIAL_MAP_ALBEDO].color = ColorBrightness(player->AccentColor, -0.5);
	gprAssets.MultInnerFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].color = ColorBrightness(player->AccentColor, -0.4);

	Vector4 FColor = { 0.5f, 0.4f, 0.1, 1.0f };
	float ForFCTime = GetTime();
	int FCING = player->stats->FC ? 1 : 0;
	SetTextureWrap(gprAssets.MultFCTex1,TEXTURE_WRAP_REPEAT);
	SetTextureWrap(gprAssets.MultFCTex2,TEXTURE_WRAP_REPEAT);
	SetTextureWrap(gprAssets.MultFCTex3,TEXTURE_WRAP_REPEAT);
	Color basicColor = player->Bot ? ColorBrightness(SKYBLUE, 0.2) : ColorBrightness(player->AccentColor, -0.4);
	Vector4 basicColorVec = {basicColor.r/255.0f, basicColor.g/255.0f,basicColor.b/255.0f,basicColor.a/255.0f};
	SetShaderValue(gprAssets.FullComboIndicator, gprAssets.FCIndLoc, &FCING, SHADER_UNIFORM_INT);
	SetShaderValue(gprAssets.FullComboIndicator, gprAssets.TimeLoc, &ForFCTime, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.FullComboIndicator, gprAssets.BasicColorLoc, &basicColorVec, SHADER_UNIFORM_VEC4);
	SetShaderValue(gprAssets.FullComboIndicator, gprAssets.FCColorLoc, &FColor, SHADER_UNIFORM_VEC4);
	SetShaderValueTexture(gprAssets.FullComboIndicator, gprAssets.BottomTextureLoc, gprAssets.MultFCTex3);
	SetShaderValueTexture(gprAssets.FullComboIndicator, gprAssets.MiddleTextureLoc, gprAssets.MultFCTex1);
	SetShaderValueTexture(gprAssets.FullComboIndicator, gprAssets.TopTextureLoc, gprAssets.MultFCTex2);


	DrawModel(gprAssets.MultInnerDot, Vector3{ 0,0.0f,1.225f }, 1, WHITE);
	DrawModel(gprAssets.MultFill, Vector3{ 0,0.0f,1.225f  }, 1, WHITE);
	DrawModel(gprAssets.MultOuterFrame, Vector3{ 0,0.0f,1.225f }, 1, WHITE);
	DrawModel(gprAssets.MultInnerFrame, Vector3{ 0,0.0f,1.225f }, 1, ColorBrightness(player->AccentColor, -0.4));



	//DrawModel(gprAssets.multFrame, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
	//DrawModel(gprAssets.multBar, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
	//if (player->Instrument == PAD_BASS|| player->Instrument == PAD_VOCALS || player->Instrument == PLASTIC_BASS) {
		//DrawModel(gprAssets.multCtr5, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
	//}
	//else {
		//DrawModel(gprAssets.multCtr3, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);
	//}
	// DrawModel(gprAssets.multNumber, Vector3{ 0,1.0f,-0.3f }, 0.8f, WHITE);

	// float fontSize = 40;

/*
	rlPushMatrix();
		rlRotatef(180, 0, 1, 0);
		rlRotatef(90, 1, 0, 0);
		rlScalef(1,5,1);
		//rlRotatef(90.0f, 0.0f, 0.0f, 1.0f);								//0, 1, 2.4
		float nameWidth = MeasureText3D(gprAssets.rubikBold, player->Name.c_str(), fontSize, 0, 0).x/2;
		DrawText3D(gprAssets.rubikBold, player->Name.c_str(), Vector3{ 0-nameWidth,0.0,-0.5 }, fontSize, 0, 0, 1, WHITE);
	rlPopMatrix();
*/

	EndMode3D();
	EndTextureMode();

	SetTextureWrap(hud_tex.texture,TEXTURE_WRAP_CLAMP);
	hud_tex.texture.width = (float)GetScreenWidth();
	hud_tex.texture.height = (float)GetScreenHeight();
	Vector2 res = {(float)GetScreenWidth(), (float)GetScreenHeight()};
	SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, hud_tex.texture);
	SetShaderValue(gprAssets.fxaa, gprAssets.resLoc, &res, SHADER_UNIFORM_VEC2);
	BeginShaderMode(gprAssets.fxaa);
	DrawTexturePro(hud_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
	EndShaderMode();
}

void gameplayRenderer::RenderGameplay(Player* player, double time, Song song, RenderTexture2D& highway_tex, RenderTexture2D& hud_tex, RenderTexture2D& notes_tex, RenderTexture2D& highwayStatus_tex, RenderTexture2D& smasher_tex) {

	Chart& curChart = song.parts[player->Instrument]->charts[player->Difficulty];
	float highwayLength = (defaultHighwayLength*1.5); //* player->HighwayLength;
	player->stats->Difficulty = player->Difficulty;
	player->stats->Instrument = player->Instrument;
	//float multFill = (!player->stats->Overdrive ? (float)(player->stats->multiplier() - 1) : (!player->stats->Multiplayer ? ((float)(player->stats->multiplier() / 2) - 1) / (float)player->stats->maxMultForMeter() : (float)(player->stats->multiplier() - 1)));
	float multFill = (!player->stats->Overdrive ? (float)(player->stats->multiplier() - 1) : ((float)(player->stats->multiplier() / 2) - 1)) / (float)player->stats->maxMultForMeter();

	//float multFill = 0.0;
	float DiffMultiplier = Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
	float relTime = (time * ((player->NoteSpeed * DiffMultiplier))) /2.75;//1.75;
	float highwaySpeedTime = (relTime - smasherPos);

	int PlayerComboMax = (player->Instrument == PAD_BASS || player->Instrument == PAD_VOCALS || player->Instrument == PLASTIC_BASS) ? 50 : 30;
	Color highwayColor = ColorContrast(player->AccentColor, Clamp(Remap(player->stats->Combo, 0, PlayerComboMax, -0.6f, 0.0f), -0.6, 0.0f));

	Vector4 ColorForHighway = Vector4{highwayColor.r/255.0f, highwayColor.g/255.0f, highwayColor.b/255.0f, highwayColor.a/255.0f};

	SetShaderValue(gprAssets.Highway, gprAssets.HighwayTimeShaderLoc, &highwaySpeedTime, SHADER_UNIFORM_FLOAT);

	SetShaderValue(gprAssets.Highway, gprAssets.HighwayColorShaderLoc, &ColorForHighway, SHADER_UNIFORM_VEC4);
	SetShaderValue(gprAssets.odMultShader, gprAssets.multLoc, &multFill, SHADER_UNIFORM_FLOAT);


	SetShaderValue(gprAssets.multNumberShader, gprAssets.uvOffsetXLoc, &player->stats->uvOffsetX, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.multNumberShader, gprAssets.uvOffsetYLoc, &player->stats->uvOffsetY, SHADER_UNIFORM_FLOAT);
	float comboFill = player->stats->comboFillCalc();
	int isBassOrVocal = 0;
	if (player->Instrument == PAD_BASS|| player->Instrument == PAD_VOCALS || player->Instrument == PLASTIC_BASS) {
		isBassOrVocal = 1;
	}
	SetShaderValue(gprAssets.odMultShader, gprAssets.isBassOrVocalLoc, &isBassOrVocal, SHADER_UNIFORM_INT);
	SetShaderValue(gprAssets.odMultShader, gprAssets.comboCounterLoc, &comboFill, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.odMultShader, gprAssets.odLoc, &player->stats->overdriveFill, SHADER_UNIFORM_FLOAT);

	float HighwayFadeStart = highwayLength + (smasherPos * 2);
	float HighwayEnd = highwayLength + (smasherPos * 4);
	gprAssets.HighwayFade.locs[SHADER_LOC_COLOR_DIFFUSE] = gprAssets.HighwayColorLoc;
	SetShaderValue(gprAssets.Highway, gprAssets.HighwayScrollFadeStartLoc, &HighwayFadeStart, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.Highway, gprAssets.HighwayScrollFadeEndLoc, &HighwayEnd, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayFadeStartLoc, &HighwayFadeStart, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayFadeEndLoc, &HighwayEnd, SHADER_UNIFORM_FLOAT);

	gprAssets.noteTopModel.materials->shader = gprAssets.HighwayFade;
	gprAssets.noteBottomModel.materials->shader = gprAssets.HighwayFade;
	gprAssets.liftModel.materials->shader = gprAssets.HighwayFade;
	gprAssets.liftModelOD.materials->shader = gprAssets.HighwayFade;
	gprAssets.noteTopModelHP.materials->shader = gprAssets.HighwayFade;
	gprAssets.noteBottomModelHP.materials->shader = gprAssets.HighwayFade;
	gprAssets.noteTopModelOD.materials->shader = gprAssets.HighwayFade;
	gprAssets.noteBottomModelOD.materials->shader = gprAssets.HighwayFade;
	gprAssets.CymbalInner.materials->shader = gprAssets.HighwayFade;
	gprAssets.CymbalOuter.materials->shader = gprAssets.HighwayFade;
	gprAssets.CymbalBottom.materials->shader = gprAssets.HighwayFade;
	gprAssets.smasherBoard.materials->shader = gprAssets.HighwayFade;
	gprAssets.smasherPressed.materials->shader = gprAssets.HighwayFade;
	gprAssets.smasherReg.materials->shader = gprAssets.HighwayFade;
	gprAssets.beatline.materials->shader = gprAssets.HighwayFade;
	// gprAssets.odBar.materials->shader = gprAssets.HighwayFade;
	// gprAssets.odFrame.materials->shader = gprAssets.HighwayFade;

	gprAssets.expertHighwaySides.materials->shader = gprAssets.HighwayFade;
	gprAssets.odHighwayX.materials->shader = gprAssets.HighwayFade;

	gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = highwayColor;
	gprAssets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = highwayColor;
	gprAssets.smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = highwayColor;
	gprAssets.smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = highwayColor;


	gprAssets.expertHighway.meshes[0].colors = (unsigned char*)ColorToInt(highwayColor);
	gprAssets.emhHighway.meshes[0].colors = (unsigned char*)ColorToInt(highwayColor);
	gprAssets.smasherBoard.meshes[0].colors = (unsigned char*)ColorToInt(highwayColor);
	gprAssets.smasherBoardEMH.meshes[0].colors = (unsigned char*)ColorToInt(highwayColor);

	gprAssets.expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player->AccentColor;
	gprAssets.emhHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player->AccentColor;


	if (player->Bot) player->stats->FC = false;

	// if (player->Bot) player->Bot = true;
	// else player->Bot = false;


	RaiseHighway();
	if (GetTime() >= startTime + animDuration && highwayInEndAnim) {
		gprAudioManager.BeginPlayback(gprAudioManager.loadedStreams[0].handle);
		audioStartTime = GetTime();
		songPlaying = true;

		highwayInEndAnim = false;
		songEnded = false;
	}

	/*
	if ((player->stats->Overdrive ? player.multiplier(player->Instrument) / 2 : player.multiplier(player->Instrument))>= (player->Instrument == PAD_BASS|| player->Instrument == PAD_VOCALS || player->Instrument == PLASTIC_BASS ? 6 : 4)) {
		gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
		gprAssets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
		gprAssets.smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
		gprAssets.smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = accentColor;
	}
	else {
		gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;
		gprAssets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;
		gprAssets.smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;
		gprAssets.smasherBoardEMH.materials[0].maps[MATERIAL_MAP_ALBEDO].color = GRAY;
	}
	 */


	if (player->stats->Overdrive) {

		// gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.highwayTextureOD;
		// gprAssets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.highwayTextureOD;
		if (!gprPlayerManager.BandStats.Multiplayer) {
			gprAssets.multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFillActive;
			gprAssets.multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFillActive;
			gprAssets.multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFillActive;
		}
		// THIS IS LOGIC!
		player->stats->overdriveFill = player->stats->overdriveActiveFill - (float)((time - player->stats->overdriveActiveTime) / (1920 / song.bpms[player->stats->curBPM].bpm));
		if (player->stats->overdriveFill <= 0) {
			player->stats->overdriveActivateTime = time;
			player->stats->Overdrive = false;
			player->stats->overdriveActiveFill = 0;
			player->stats->overdriveActiveTime = 0.0;
			gprPlayerManager.BandStats.PlayersInOverdrive -= 1;
			gprPlayerManager.BandStats.Overdrive = false;;

			gprAssets.expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.highwayTexture;
			gprAssets.emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = gprAssets.highwayTexture;
			gprAssets.multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFill;
			gprAssets.multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFill;
			gprAssets.multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = gprAssets.odMultFill;

		}
	}

	for (int i = player->stats->curBPM; i < song.bpms.size(); i++) {
		if (time > song.bpms[i].time && i < song.bpms.size() - 1)
			player->stats->curBPM++;
	}

	if (!curChart.odPhrases.empty() && player->stats->curODPhrase<curChart.odPhrases.size() - 1 && time>curChart.odPhrases[player->stats->curODPhrase].end && (curChart.odPhrases[player->stats->curODPhrase].added ||curChart.odPhrases[player->stats->curODPhrase].missed)) {
		player->stats->curODPhrase++;
	}

	if (!curChart.Solos.empty() && player->stats->curSolo<curChart.Solos.size() - 1 && time-2.5f>curChart.Solos[player->stats->curSolo].end) {
		player->stats->curSolo++;
	}

	if (!curChart.fills.empty() && player->stats->curFill < curChart.fills.size() - 1 && time>curChart.fills[player->stats->curFill].end) {
		player->stats->curFill++;
	}

	if (!curChart.Sections.empty() && player->stats->curSection < curChart.Sections.size() - 1 && time > curChart.Sections[player->stats->curSection].End) {
		std::cout << "Section " << curChart.Sections[player->stats->curSection].Name << " complete. Notes hit: " << curChart.Sections[player->stats->curSection].notesHit << "/" << curChart.Sections[player->stats->curSection].totalNotes << std::endl;
		player->stats->curSection++;
	}

	// BeginShaderMode(gprAssets.HighwayFade);
	if (player->Instrument == PLASTIC_DRUMS) {
		RenderPDrumsHighway(player, song, time, highway_tex, highwayStatus_tex, smasher_tex);
	}
	else if (player->Difficulty == 3 || (player->ClassicMode && player->Instrument!=4)) {
		RenderExpertHighway(player, song, time, highway_tex, highwayStatus_tex, smasher_tex);
	} else {
		RenderEmhHighway(player, song, time, highway_tex);
	}
	if (player->ClassicMode) {
		if (player->Instrument == PLASTIC_DRUMS) {
			RenderPDrumsNotes(player, curChart, time, notes_tex, highwayLength);
		}
		else {
			RenderClassicNotes(player, curChart, time, notes_tex, highwayLength);
		}
	} else {
		RenderNotes(player, curChart, time, notes_tex, highwayLength);
	}
	// EndShaderMode();
	// if (!player->Bot)
		RenderHud(player, hud_tex, highwayLength);
}

void gameplayRenderer::DrawHighwayMesh(float LengthMultiplier, bool Overdrive, float ActiveTime, float SongTime) {
	Vector3 HighwayPos{0,0,-0.2f};
	Vector3 HighwayScale{1,1,1.5f * LengthMultiplier};
	gprAssets.expertHighway.materials[0].maps->texture.height /= 2.0f;
	DrawModelEx(gprAssets.expertHighwaySides, HighwayPos, {0}, 0, HighwayScale, WHITE);
	DrawModelEx(gprAssets.expertHighway, HighwayPos, {0}, 0, HighwayScale, WHITE);
	unsigned char OverdriveAlpha = 255;
	double OverdriveAnimDuration = 0.25f;
	if (SongTime <= ActiveTime + OverdriveAnimDuration) {
		double TimeSinceOverdriveActivate = SongTime - ActiveTime;
		OverdriveAlpha = Remap(getEasingFunction(EaseOutQuint)(TimeSinceOverdriveActivate/OverdriveAnimDuration), 0, 1.0, 0, 255);
	} else OverdriveAlpha = 255;

	if (SongTime <= ActiveTime + OverdriveAnimDuration && SongTime > 0.0) {
		double TimeSinceOverdriveActivate = SongTime - ActiveTime;
		OverdriveAlpha = Remap(getEasingFunction(EaseOutQuint)(TimeSinceOverdriveActivate/OverdriveAnimDuration), 0, 1.0, 255, 0);
	} else if (!Overdrive) OverdriveAlpha = 0;

	if (Overdrive || SongTime <= ActiveTime + OverdriveAnimDuration) {
		DrawModelEx(gprAssets.odHighwayX, Vector3{0,0.005f,-0.2},{0},0,HighwayScale,Color{255,255,255,OverdriveAlpha});
	}
};

void gameplayRenderer::RenderExpertHighway(Player* player, Song song, double time, RenderTexture2D& highway_tex, RenderTexture2D& highwayStatus_tex, RenderTexture2D& smasher_tex)  {
	BeginTextureMode(highway_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel]);
	PlayerGameplayStats *stats = player->stats;
	rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);
	BeginBlendMode(BLEND_CUSTOM_SEPARATE);
	float diffDistance = 2.0f;
	float lineDistance = 1.5f;

	float highwayLength = (defaultHighwayLength * 1.5); //* player->HighwayLength;
	float highwayPosShit = ((20) * (1 - gprSettings.highwayLengthMult));

	DrawHighwayMesh(1, player->stats->Overdrive, player->stats->overdriveActiveTime, time);

	if (!player->ClassicMode)
		DrawCylinderEx(Vector3{ lineDistance-1.0f, 0, smasherPos}, Vector3{ lineDistance-1.0f, 0, (highwayLength *1.5f) + smasherPos }, 0.025f, 0.025f, 15, Color{ 128,128,128,128 });

	EndBlendMode();
	EndMode3D();
	EndTextureMode();
	SetTextureWrap(highway_tex.texture,TEXTURE_WRAP_CLAMP);
	highway_tex.texture.width = (float)GetScreenWidth();
	highway_tex.texture.height = (float)GetScreenHeight();
	Vector2 res = {(float)GetScreenWidth(), (float)GetScreenHeight()};
	SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, highway_tex.texture);
	SetShaderValue(gprAssets.fxaa, gprAssets.resLoc, &res, SHADER_UNIFORM_VEC2);
	BeginShaderMode(gprAssets.fxaa);
	DrawTexturePro(highway_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
EndShaderMode();

	BeginTextureMode(highwayStatus_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel]);
	rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);
	BeginBlendMode(BLEND_CUSTOM_SEPARATE);

	int UseIn = 1;
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &UseIn, SHADER_UNIFORM_INT);
	BeginShaderMode(gprAssets.HighwayFade);
	if (!song.beatLines.empty()) {
		DrawBeatlines(player, song, highwayLength, time);
	}

	if (!song.parts[player->Instrument]->charts[player->Difficulty].odPhrases.empty()) {
		DrawOverdrive(player, song.parts[player->Instrument]->charts[player->Difficulty], highwayLength, time);
	}
	if (!song.parts[player->Instrument]->charts[player->Difficulty].Solos.empty()) {
		DrawSolo(player, song.parts[player->Instrument]->charts[player->Difficulty], highwayLength, time);
	}
	float darkYPos = 0.015f;

	float HighwayFadeStart = highwayLength + (smasherPos * 2);
	float HighwayEnd = highwayLength + (smasherPos * 4);
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayFadeStartLoc, &HighwayFadeStart, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayFadeEndLoc, &HighwayEnd, SHADER_UNIFORM_FLOAT);
	BeginBlendMode(BLEND_ALPHA);
	DrawTriangle3D({-diffDistance-0.5f,darkYPos,smasherPos},{-diffDistance-0.5f,darkYPos,(highwayLength *1.5f) + smasherPos},{diffDistance+0.5f,darkYPos,smasherPos},Color{0,0,0,160});
	DrawTriangle3D({diffDistance+0.5f,darkYPos,(highwayLength *1.5f) + smasherPos},{diffDistance+0.5f,darkYPos,smasherPos},{-diffDistance-0.5f,darkYPos,(highwayLength *1.5f) + smasherPos},Color{0,0,0,160});
	EndShaderMode();
	int DontIn = 0;
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DontIn, SHADER_UNIFORM_INT);
	HighwayFadeStart = highwayLength + (smasherPos * 2);
	HighwayEnd = highwayLength + (smasherPos * 4);
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayFadeStartLoc, &HighwayFadeStart, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayFadeEndLoc, &HighwayEnd, SHADER_UNIFORM_FLOAT);
	EndBlendMode();
	EndMode3D();
	EndTextureMode();
	SetTextureWrap(highwayStatus_tex.texture,TEXTURE_WRAP_CLAMP);
	highwayStatus_tex.texture.width = (float)GetScreenWidth();
	highwayStatus_tex.texture.height = (float)GetScreenHeight();
	Vector2 res2 = {(float)GetScreenWidth(), (float)GetScreenHeight()};
	SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, highwayStatus_tex.texture);
	SetShaderValue(gprAssets.fxaa, gprAssets.resLoc, &res2, SHADER_UNIFORM_VEC2);
	BeginShaderMode(gprAssets.fxaa);
	DrawTexturePro(highwayStatus_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
EndShaderMode();

	BeginTextureMode(smasher_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel]);
	BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
	DrawModel(gprAssets.smasherBoard, Vector3{ 0, 0.004f, 0 }, 1.0f, WHITE);
	rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);

	BeginBlendMode(BLEND_CUSTOM_SEPARATE);
	for (int i = 0; i < 5;  i++) {
		Color NoteColor; // = gprMenu.hehe && player->Difficulty == 3 ? i == 0 || i == 4 ? SKYBLUE : i == 1 || i == 3 ? PINK : WHITE : accentColor;
		int noteColor = gprSettings.mirrorMode ? 4 - i : i;
		if (player->ClassicMode) {
			NoteColor = gprMenu.hehe ? TRANS[i] : GRYBO[i];
		}   else {
			NoteColor = gprMenu.hehe ? TRANS[i] : player->AccentColor;
		}

		gprAssets.smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
		gprAssets.smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;

		if (player->stats->HeldFrets[noteColor] || player->stats->HeldFretsAlt[noteColor]) {
			DrawModel(gprAssets.smasherPressed, Vector3{ diffDistance - (float)(i), 0.01f, smasherPos }, 1.0f, WHITE);
		}
		else {
			DrawModel(gprAssets.smasherReg, Vector3{ diffDistance - (float)(i), 0.01f, smasherPos }, 1.0f, WHITE);
		}
	}
	//DrawModel(gprAssets.lanes, Vector3 {0,0.1f,0}, 1.0f, WHITE);

	EndBlendMode();
	EndMode3D();
	EndTextureMode();
	SetTextureWrap(smasher_tex.texture,TEXTURE_WRAP_CLAMP);
	smasher_tex.texture.width = (float)GetScreenWidth();
	smasher_tex.texture.height = (float)GetScreenHeight();
	Vector2 res3 = {(float)GetScreenWidth(), (float)GetScreenHeight()};
	SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, smasher_tex.texture);
	SetShaderValue(gprAssets.fxaa, gprAssets.resLoc, &res3, SHADER_UNIFORM_VEC2);
	BeginShaderMode(gprAssets.fxaa);
	DrawTexturePro(smasher_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
	EndShaderMode();


}

void gameplayRenderer::RenderEmhHighway(Player* player, Song song, double time, RenderTexture2D &highway_tex) {
	BeginTextureMode(highway_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel]);

	float diffDistance = player->Difficulty == 3 ? 2.0f : 1.5f;
	float lineDistance = player->Difficulty == 3 ? 1.5f : 1.0f;

	float highwayLength = defaultHighwayLength * gprSettings.highwayLengthMult;
	float highwayPosShit = ((20) * (1 - gprSettings.highwayLengthMult));

	DrawModel(gprAssets.emhHighwaySides, Vector3{ 0,0,gprSettings.highwayLengthMult < 1.0f ? -(highwayPosShit* (0.875f)) : 0 }, 1.0f, WHITE);
	DrawModel(gprAssets.emhHighway, Vector3{ 0,0,gprSettings.highwayLengthMult < 1.0f ? -(highwayPosShit* (0.875f)) : 0 }, 1.0f, WHITE);
	if (gprSettings.highwayLengthMult > 1.0f) {
		DrawModel(gprAssets.emhHighway, Vector3{ 0,0,((highwayLength*1.5f)+smasherPos)-20 }, 1.0f, WHITE);
		DrawModel(gprAssets.emhHighwaySides, Vector3{ 0,0,((highwayLength*1.5f)+smasherPos)-20 }, 1.0f, WHITE);
		if (highwayLength > 23.0f) {
			DrawModel(gprAssets.emhHighway, Vector3{ 0,0,((highwayLength*1.5f)+smasherPos)-40 }, 1.0f, WHITE);
			DrawModel(gprAssets.emhHighwaySides, Vector3{ 0,0,((highwayLength*1.5f)+smasherPos)-40 }, 1.0f, WHITE);
		}
	}
	if (player->stats->Overdrive) {DrawModel(gprAssets.odHighwayEMH, Vector3{0,0.001f,0},1,WHITE);}

	DrawTriangle3D({-diffDistance-0.5f,0.02,smasherPos},{-diffDistance-0.5f,0.02,(highwayLength *1.5f) + smasherPos},{diffDistance+0.5f,0.02,smasherPos},Color{0,0,0,64});
	DrawTriangle3D({diffDistance+0.5f,0.02,(highwayLength *1.5f) + smasherPos},{diffDistance+0.5f,0.02,smasherPos},{-diffDistance-0.5f,0.02,(highwayLength *1.5f) + smasherPos},Color{0,0,0,64});

	DrawModel(gprAssets.smasherBoardEMH, Vector3{ 0, 0.001f, 0 }, 1.0f, WHITE);

	for (int i = 0; i < 4; i++) {
		Color NoteColor = gprMenu.hehe && player->Difficulty == 3 ? i == 0 || i == 4 ? SKYBLUE : i == 1 || i == 3 ? PINK : WHITE : player->AccentColor;

		gprAssets.smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
		gprAssets.smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;

		if (player->stats->HeldFrets[i] || player->stats->HeldFretsAlt[i]) {
			DrawModel(gprAssets.smasherPressed, Vector3{ diffDistance - (float)(i), 0.01f, smasherPos }, 1.0f, WHITE);
		}
		else {
			DrawModel(gprAssets.smasherReg, Vector3{ diffDistance - (float)(i), 0.01f, smasherPos }, 1.0f, WHITE);

		}
	}
	for (int i = 0; i < 3; i++) {
		float radius = (i == 1) ? 0.03 : 0.01;
		DrawCylinderEx(Vector3{ lineDistance - (float)i, 0, smasherPos + 0.5f }, Vector3{ lineDistance - (float)i, 0, (highwayLength *1.5f) + smasherPos }, radius,
					   radius, 4.0f, Color{ 128, 128, 128, 128 });
	}
	if (!song.beatLines.empty()) {
		DrawBeatlines(player, song, highwayLength, time);
	}
	if (!song.parts[player->Instrument]->charts[player->Difficulty].Solos.empty()) {
		DrawSolo(player, song.parts[player->Instrument]->charts[player->Difficulty], highwayLength, time);
	}
	if (!song.parts[player->Instrument]->charts[player->Difficulty].odPhrases.empty()) {
		DrawOverdrive(player, song.parts[player->Instrument]->charts[player->Difficulty], highwayLength, time);
	}

	EndBlendMode();
	EndMode3D();
	EndTextureMode();

	SetTextureWrap(highway_tex.texture,TEXTURE_WRAP_CLAMP);
	highway_tex.texture.width = (float)GetScreenWidth();
	highway_tex.texture.height = (float)GetScreenHeight();
	Vector2 res = {(float)GetScreenWidth(), (float)GetScreenHeight()};
	SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, highway_tex.texture);
	SetShaderValue(gprAssets.fxaa, gprAssets.resLoc, &res, SHADER_UNIFORM_VEC2);
	BeginShaderMode(gprAssets.fxaa);
	DrawTexturePro(highway_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {0,highwayLevel}, 0, WHITE );
	EndShaderMode();
}

void gameplayRenderer::DrawBeatlines(Player* player, Song song, float length, double musicTime) {
	std::vector<std::pair<double, bool>> beatlines = song.beatLines;
	Model beatline = gprAssets.beatline;
	beatline.materials[0].shader = gprAssets.HighwayFade;
	float yPos = -0.1f;
	if (beatlines.size() > 0) {
		for (int i = player->stats->curBeatLine; i < beatlines.size(); i++) {
			if (beatlines[i].first <= song.end) {
				Color BeatLineColor = {255,255,255,255};
				double relTime = GetNoteOnScreenTime(beatlines[i].first, musicTime, player->NoteSpeed, player->Difficulty, length);
				if (relTime < -1) continue;
				if (i > 0) {
					double secondLine = GetNoteOnScreenTime(((beatlines[i-1].first + beatlines[i].first)/2), musicTime, player->NoteSpeed, player->Difficulty, length);
					if (secondLine > 1.5) break;

					Color BeatLineColor2 = { 255, 255, 255, 255 };
					Vector3 SecondaryBeatlinePos = Vector3{ 0,yPos,smasherPos + (length * (float)secondLine)};
					beatline.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BeatLineColor2;
					beatline.meshes[0].colors = (unsigned char *)ColorToInt(BeatLineColor2);
					DrawModelEx(beatline, SecondaryBeatlinePos, {0}, 0, {1,1,0.5}, BeatLineColor2);
					/*DrawCylinderEx(Vector3{ -diffDistance - 0.5f,yPos,smasherPos + (length * (float)secondLine) + 0.02f },
								   Vector3{ diffDistance + 0.5f,yPos,smasherPos + (length * (float)secondLine) + 0.02f },
								   0.01f,
								   0.01f,
								   4,
								   BeatLineColor);*/
				}
				if (relTime > 1.5) break;

				float radius = beatlines[i].second ? 1.5f : 0.95f ;

				Color BeatLineColorA = (beatlines[i].second) ? Color{ 255, 255, 255, 255 } : Color{ 255, 255, 255, 255 };
				beatline.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BeatLineColorA;
				beatline.meshes[0].colors = (unsigned char *)ColorToInt(BeatLineColorA);
				Vector3 BeatlinePos = Vector3{ 0,yPos,smasherPos + (length * (float)relTime)};
				DrawModelEx(beatline, BeatlinePos, {0}, 0, {1,1,radius}, BeatLineColorA);
				/*DrawCylinderEx(Vector3{ -diffDistance - 0.5f,yPos,smasherPos + (length * (float)relTime) + (radius * 2) },
							   Vector3{ diffDistance + 0.5f,yPos,smasherPos + (length * (float)relTime) + (radius * 2) },
							   radius,
							   radius,
							   4,
							   BeatLineColor);*/

				// if (relTime < -1) break;
				if (relTime < -1 && player->stats->curBeatLine < beatlines.size() - 1) {
					player->stats->curBeatLine++;
				}
			}
		}
	}
}

void gameplayRenderer::DrawOverdrive(Player* player,  Chart& curChart, float length, double musicTime) {
	float DiffMultiplier = Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
	float start = curChart.odPhrases[player->stats->curODPhrase].start;
	float end = curChart.odPhrases[player->stats->curODPhrase].end;
	/* for unisons
		eDrawSides((player->NoteSpeed * DiffMultiplier), musicTime, start, end, length, 0.1, WHITE);
	*/
}

void gameplayRenderer::DrawSolo(Player* player, Chart& curChart, float length, double musicTime) {
	float DiffMultiplier = Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
	float start = curChart.Solos[player->stats->curSolo].start;
	float end = curChart.Solos[player->stats->curSolo].end;
	eDrawSides((player->NoteSpeed * DiffMultiplier), musicTime, start, end, length, 0.09, SKYBLUE);

	if (!curChart.Solos.empty() && musicTime >= curChart.Solos[player->stats->curSolo].start - 1 && musicTime <= curChart.Solos[player->stats->curSolo].end + 2.5) {
		int solopctnum = Remap(curChart.Solos[player->stats->curSolo].notesHit, 0, curChart.Solos[player->stats->curSolo].noteCount, 0, 100);
		Color accColor = solopctnum == 100 ? GOLD : WHITE;
		const char* soloPct = TextFormat("%i%%", solopctnum);
		float soloPercentLength = MeasureTextEx(gprAssets.rubikBold, soloPct, gprU.hinpct(0.09f), 0).x;

		Vector2 SoloBoxPos = {(GetScreenWidth()/2) - (soloPercentLength/2), gprU.hpct(0.2f)};

		//DrawTextEx(gprAssets.rubikBold, soloPct, SoloBoxPos, gprU.hinpct(0.09f), 0, accColor);

		const char* soloHit = TextFormat("%i/%i", curChart.Solos[player->stats->curSolo].notesHit, curChart.Solos[player->stats->curSolo].noteCount);

		//DrawTextEx(gprAssets.josefinSansItalic, soloHit, SoloHitPos, gprU.hinpct(0.04f), 0, accColor);

		float posY = -20;

		float height = -1;
		float pctDist = 1.2;
		float praiseDist = 0;
		float backgroundHeight = -2.2;
		float soloScale = 1.15;

		float fontSizee = 160;
		float fontSize = 75;

		gprAssets.SoloBox.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player->AccentColor;
		rlPushMatrix();
			rlRotatef(180, 0, 1, 0);
			rlRotatef(90, 1, 0, 0);
			//rlRotatef(90.0f, 0.0f, 0.0f, 1.0f);								//0, 1, 2.4
			float soloWidth = MeasureText3D(gprAssets.rubikBold, soloPct, fontSizee, 0, 0).x/2;
			float hitWidth = MeasureText3D(gprAssets.josefinSansItalic, soloHit, fontSize, 0, 0).x/2;
			DrawModel(gprAssets.SoloBox, Vector3{0, posY-1, backgroundHeight}, soloScale, WHITE);
			DrawText3D(gprAssets.rubikBold, soloPct, Vector3{ 0-soloWidth,posY,height-pctDist }, fontSizee, 0, 0, 1, accColor);
			if (musicTime <= curChart.Solos[player->stats->curSolo].end)
				DrawText3D(gprAssets.josefinSansItalic, soloHit, Vector3{ 0-hitWidth,posY,height }, fontSize, 0, 0, 1, accColor);
		rlPopMatrix();
		if (musicTime >= curChart.Solos[player->stats->curSolo].end && musicTime <= curChart.Solos[player->stats->curSolo].end + 2.5) {

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
			rlPushMatrix();
				rlRotatef(180, 0, 1, 0);
				rlRotatef(90, 1, 0, 0);
				//rlRotatef(90.0f, 0.0f, 0.0f, 1.0f);								//0, 1, 2.4
				float praiseWidth = MeasureText3D(gprAssets.josefinSansItalic, PraiseText, fontSize, 0, 0).x/2;
				DrawText3D(gprAssets.josefinSansItalic, PraiseText, Vector3{ 0-praiseWidth,posY,height-praiseDist }, fontSize, 0, 0, 1, accColor);
			rlPopMatrix();
			// DrawTextEx(gprAssets.josefinSansItalic, PraiseText, PraisePos, gprU.hinpct(0.05f), 0, accColor);
		}
	}
}

void gameplayRenderer::DrawFill(Player* player, Chart& curChart, float length, double musicTime) {
	float DiffMultiplier = Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);

	float start = curChart.fills[player->stats->curFill].start;
	float end = curChart.fills[player->stats->curFill].end;
	eDrawSides((player->NoteSpeed * DiffMultiplier), musicTime, start, end, length, 0.075, GREEN);
}

void gameplayRenderer::eDrawSides(float scrollPos, double time, double start, double end, float length, double radius, Color color) {
	float soloStart = (float)((start - time)) * scrollPos * (11.5f / length);
	float soloEnd = (float)((end - time)) * scrollPos * (11.5f / length);

	// horrifying.
	// main calc
	bool Beginning = (float)(smasherPos + (length * soloStart)) >= (length * 1.5f) + smasherPos;
	bool Ending = (float)(smasherPos + (length * soloEnd)) >= (length * 1.5f) + smasherPos;
	float HighwayEnd = (length * 1.5f) + smasherPos;

	// right calc
	float RightSideX = 2.7f;
	float StartPos = Beginning ?  HighwayEnd : (float)(smasherPos + (length * soloStart));
	float EndPos = Ending ? HighwayEnd : (float)(smasherPos + (length * soloEnd));
	Vector3 RightSideStart = {RightSideX ,-0.0025, StartPos};
	Vector3 RightSideEnd = { RightSideX,-0.0025, EndPos };

	// left calc
	float LeftSideX = -2.7f;

	Vector3 LeftSideStart = {LeftSideX ,-0.0025,StartPos };
	Vector3 LeftSideEnd = { LeftSideX,-0.0025, EndPos };

	// draw
	// if (soloEnd < -1) break;
	if (soloEnd > -1) {
		DrawCylinderEx(RightSideStart, RightSideEnd, radius, radius, 10, color);
		DrawCylinderEx(LeftSideStart, LeftSideEnd, radius, radius, 10, color);
	}
}

double gameplayRenderer::GetNoteOnScreenTime(double noteTime, double songTime, float noteSpeed, int Difficulty, float length) {
	return ((noteTime - songTime)) * (noteSpeed * HighwaySpeedDifficultyMultiplier(Difficulty)) * (11.5f / length);
}

double gameplayRenderer::HighwaySpeedDifficultyMultiplier(int Difficulty) {
	return Remap(Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
}

// classic drums
void gameplayRenderer::RenderPDrumsNotes(Player* player, Chart& curChart, double time, RenderTexture2D& notes_tex, float length) {
	float diffDistance = 2.0f;
	float DiffMultiplier = Remap(player->Difficulty, 0, 3, MinHighwaySpeed, MaxHighwaySpeed);
	BeginTextureMode(notes_tex);
	ClearBackground({ 0,0,0,0 });
	BeginMode3D(cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel]);
	// glDisable(GL_CULL_FACE);
	PlayerGameplayStats *stats = player->stats;

	for (auto& curNote : curChart.notes) {

		if (!curChart.Solos.empty()) {
			if (curNote.time >= curChart.Solos[stats->curSolo].start &&
				curNote.time < curChart.Solos[stats->curSolo].end) {
				if (curNote.hit) {
					if (curNote.hit && !curNote.countedForSolo) {
						curChart.Solos[stats->curSolo].notesHit++;
						curNote.countedForSolo = true;
					}
				}
			}
		}
		if (!curChart.odPhrases.empty()) {

			if (curNote.time >= curChart.odPhrases[stats->curODPhrase].start &&
				curNote.time < curChart.odPhrases[stats->curODPhrase].end &&
				!curChart.odPhrases[stats->curODPhrase].missed) {
				if (curNote.hit) {
					if (curNote.hit && !curNote.countedForODPhrase) {
						curChart.odPhrases[stats->curODPhrase].notesHit++;
						curNote.countedForODPhrase = true;
					}
				}
				curNote.renderAsOD = true;

			}
			if (curChart.odPhrases[stats->curODPhrase].missed) {
				curNote.renderAsOD = false;
			}
			if (curChart.odPhrases[stats->curODPhrase].notesHit ==
				curChart.odPhrases[stats->curODPhrase].noteCount &&
				!curChart.odPhrases[stats->curODPhrase].added && stats->overdriveFill < 1.0f) {
				stats->overdriveFill += 0.25f;
				if (stats->overdriveFill > 1.0f) stats->overdriveFill = 1.0f;
				if (stats->Overdrive) {
					stats->overdriveActiveFill = stats->overdriveFill;
					stats->overdriveActiveTime = time;
				}
				curChart.odPhrases[stats->curODPhrase].added = true;
			}
		}
		if (!curNote.hit && !curNote.accounted && curNote.time + goodBackend + player->InputCalibration < time &&
			!songEnded && stats->curNoteInt < curChart.notes.size() && !songEnded && !player->Bot) {
			TraceLog(LOG_INFO, TextFormat("Missed note at %f, note %01i", time, stats->curNoteInt));
			curNote.miss = true;
			FAS = false;
			stats->MissNote();
			if (!curChart.odPhrases.empty() && !curChart.odPhrases[stats->curODPhrase].missed &&
				curNote.time >= curChart.odPhrases[stats->curODPhrase].start &&
				curNote.time < curChart.odPhrases[stats->curODPhrase].end)
				curChart.odPhrases[stats->curODPhrase].missed = true;
			stats->Combo = 0;
			curNote.accounted = true;
			stats->curNoteInt++;
		}
		else if (player->Bot) {
			if (!curNote.hit && !curNote.accounted && curNote.time < time && stats->curNoteInt < curChart.notes.size() && !songEnded) {
				curNote.hit = true;
				player->stats->HitDrumsNote(false, !curNote.pDrumTom);
				if (gprPlayerManager.BandStats.Multiplayer) {
					gprPlayerManager.BandStats.DrumNotePoint(false, player->stats->noODmultiplier(), !curNote.pDrumTom);
				}
				if (curNote.len > 0) curNote.held = true;
				curNote.accounted = true;
				// stats->Combo++;
				curNote.accounted = true;
				curNote.hitTime = time;
				stats->curNoteInt++;
				if (player->stats->overdriveFill >= 0.25 && curNote.pDrumAct && !player->stats->Overdrive) {
					stats->overdriveActiveTime = time;
					stats->overdriveActiveFill = stats->overdriveFill;
					stats->Overdrive = true;
					stats->overdriveHitAvailable = true;
					stats->overdriveHitTime = time;
					if (gprPlayerManager.BandStats.Multiplayer) {
						gprPlayerManager.BandStats.PlayersInOverdrive += 1;
						gprPlayerManager.BandStats.Overdrive = true;
					}
				}
			}
		}

		double relTime = GetNoteOnScreenTime(curNote.time, time, player->NoteSpeed, player->Difficulty, length);
		if (relTime < -1) continue;

		std::vector<Color> DRUMS = {ORANGE, RED, YELLOW, BLUE, GREEN};
		Color NoteColor = DRUMS[curNote.lane];

		float notePosX = curNote.lane == KICK ? 0 : (diffDistance - (1.25f * (curNote.lane - 1)))-0.125f;
		float notePosY = curNote.lane == KICK ? 0.01f : 0;
		if (relTime > 1.5) {
			break;
		}

		Vector3 NoteScale = { 1.35f,1.35f,1.35f };
		Vector3 NotePos = { notePosX, notePosY, smasherPos + (length * (float)relTime) };
		if (!curNote.pDrumTom && !curNote.pSnare && !curNote.hit && curNote.lane != KICK) { // render cymbals
			Color OuterColor = ColorBrightness(NoteColor, -0.15);
			Color InnerColor  = RAYWHITE;
			Color BottomColor = DARKGRAY;
			NoteScale = {1.0f,1.0f,1.0f};
			if (curNote.renderAsOD) {
				InnerColor = WHITE;
				OuterColor = RAYWHITE;
				BottomColor = ColorBrightness(GOLD, -0.5);
			} else if (curNote.miss) {
				InnerColor = RED;
				OuterColor = RED;
				BottomColor = RED;
			}
			if (curNote.pDrumAct && player->stats->overdriveFill >= 0.25 && !player->stats->Overdrive) {
				NoteScale.y = 2.0f;
				OuterColor = GREEN;
				InnerColor = GREEN;
			}
			gprAssets.CymbalInner.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = InnerColor;
			gprAssets.CymbalOuter.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = OuterColor;
			gprAssets.CymbalBottom.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BottomColor;
			gprAssets.CymbalInner.materials[0].shader = gprAssets.HighwayFade;
			gprAssets.CymbalOuter.materials[0].shader = gprAssets.HighwayFade;
			gprAssets.CymbalBottom.materials[0].shader = gprAssets.HighwayFade;
			gprAssets.CymbalInner.materials[0].shader.locs[SHADER_LOC_COLOR_DIFFUSE] = gprAssets.HighwayColorLoc;
			gprAssets.CymbalOuter.materials[0].shader.locs[SHADER_LOC_COLOR_DIFFUSE] = gprAssets.HighwayColorLoc;
			gprAssets.CymbalBottom.materials[0].shader.locs[SHADER_LOC_COLOR_DIFFUSE] = gprAssets.HighwayColorLoc;
			DrawModelEx(gprAssets.CymbalInner, NotePos, {0}, 0, NoteScale, InnerColor);
			DrawModelEx(gprAssets.CymbalOuter, NotePos, {0}, 0, NoteScale, OuterColor);
			DrawModelEx(gprAssets.CymbalBottom, NotePos, {0}, 0, NoteScale, BottomColor);
		} else if (!curNote.hit) {
			Model TopModel = gprAssets.noteTopModel;
			Model BottomModel = gprAssets.noteBottomModel;

			Color TopColor = NoteColor;
			Color BottomColor = WHITE;
			if (curNote.lane == KICK) {
				TopModel = gprAssets.KickBottomModel;
				TopColor = NoteColor;
				BottomModel = gprAssets.KickSideModel;
				NoteScale = {1.0f,1.0f,1.0f};
			}
			if (curNote.miss) {
				TopColor = RED;
				BottomColor = RED;
			}
			if (curNote.renderAsOD) {
				TopColor = WHITE;
				BottomColor = GOLD;
			}
			if (curNote.pDrumAct && player->stats->overdriveFill >= 0.25 && !player->stats->Overdrive) {
				NoteScale.y = 2.0f;
				TopColor = GREEN;
			}
			TopModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = TopColor;
			BottomModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BottomColor;
			TopModel.materials[0].shader = gprAssets.HighwayFade;
			BottomModel.materials[0].shader = gprAssets.HighwayFade;
			TopModel.materials[0].shader.locs[SHADER_LOC_COLOR_DIFFUSE] = gprAssets.HighwayColorLoc;
			BottomModel.materials[0].shader.locs[SHADER_LOC_COLOR_DIFFUSE] = gprAssets.HighwayColorLoc;
			DrawModelEx(TopModel, NotePos,{0},0, NoteScale, TopColor);
			DrawModelEx(BottomModel, NotePos, { 0},0, NoteScale, BottomColor);
		}

		nDrawDrumsHitEffects(curNote, time, notePosX);
		NoteMultiplierEffect(time, curNote.time, curNote.miss, player);
	}
	EndMode3D();
	EndTextureMode();

	SetTextureWrap(notes_tex.texture, TEXTURE_WRAP_CLAMP);
	notes_tex.texture.width = (float)GetScreenWidth();
	notes_tex.texture.height = (float)GetScreenHeight();
	Vector2 res = {(float)GetScreenWidth(), (float)GetScreenHeight()};
	SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, notes_tex.texture);
	SetShaderValue(gprAssets.fxaa, gprAssets.resLoc, &res, SHADER_UNIFORM_VEC2);
	BeginShaderMode(gprAssets.fxaa);
	DrawTexturePro(notes_tex.texture, { 0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() }, { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, { renderPos,highwayLevel }, 0, WHITE);
	EndShaderMode();
}

void gameplayRenderer::RenderPDrumsHighway(Player* player, Song song, double time, RenderTexture2D& highway_tex, RenderTexture2D& highwayStatus_tex, RenderTexture2D& smasher_tex) {
	BeginTextureMode(highway_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel]);
	PlayerGameplayStats *stats = player->stats;
	rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);
	BeginBlendMode(BLEND_CUSTOM_SEPARATE);
	float diffDistance = 2.0f;
	float lineDistance = 1.5f;

	float highwayLength = (defaultHighwayLength * 1.5) * gprSettings.highwayLengthMult;
	float highwayPosShit = ((20) * (1 - gprSettings.highwayLengthMult));

	DrawHighwayMesh(1, player->stats->Overdrive, player->stats->overdriveActiveTime, time);
	EndBlendMode();

	EndMode3D();
	EndTextureMode();

	SetTextureWrap(highway_tex.texture,TEXTURE_WRAP_CLAMP);
	highway_tex.texture.width = (float)GetScreenWidth();
	highway_tex.texture.height = (float)GetScreenHeight();
	DrawTexturePro(highway_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
	EndShaderMode();

	BeginTextureMode(highwayStatus_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel]);
	rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);
	BeginBlendMode(BLEND_CUSTOM_SEPARATE);
	int DoIn = 1;
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DoIn, SHADER_UNIFORM_INT);
	BeginShaderMode(gprAssets.HighwayFade);
	if (!song.beatLines.empty()) {
		DrawBeatlines(player, song, highwayLength, time);
	}

	if (!song.parts[player->Instrument]->charts[player->Difficulty].odPhrases.empty()) {
		DrawOverdrive(player, song.parts[player->Instrument]->charts[player->Difficulty], highwayLength, time);
	}
	if (!song.parts[player->Instrument]->charts[player->Difficulty].Solos.empty()) {
		DrawSolo(player, song.parts[player->Instrument]->charts[player->Difficulty], highwayLength, time);
	}
	if (!song.parts[player->Instrument]->charts[player->Difficulty].fills.empty() && player->stats->overdriveFill >= 0.25 && !player->stats->Overdrive) {
		DrawFill(player, song.parts[player->Instrument]->charts[player->Difficulty], highwayLength, time);
	}
	float darkYPos = 0.015f;
	float HighwayFadeStart = highwayLength + (smasherPos * 2);
	float HighwayEnd = highwayLength + (smasherPos * 3);
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayFadeStartLoc, &HighwayFadeStart, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayFadeEndLoc, &HighwayEnd, SHADER_UNIFORM_FLOAT);
	DrawTriangle3D({-diffDistance-0.5f,darkYPos,smasherPos},{-diffDistance-0.5f,darkYPos,(highwayLength *1.5f) + smasherPos},{diffDistance+0.5f,darkYPos,smasherPos},Color{0,0,0,96});
	DrawTriangle3D({diffDistance+0.5f,darkYPos,(highwayLength *1.5f) + smasherPos},{diffDistance+0.5f,darkYPos,smasherPos},{-diffDistance-0.5f,darkYPos,(highwayLength *1.5f) + smasherPos},Color{0,0,0,96});
	EndShaderMode();
	int DontIn = 0;
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayAccentFadeLoc, &DontIn, SHADER_UNIFORM_INT);
	HighwayFadeStart = highwayLength + (smasherPos * 2);
	HighwayEnd = highwayLength + (smasherPos * 4);
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayFadeStartLoc, &HighwayFadeStart, SHADER_UNIFORM_FLOAT);
	SetShaderValue(gprAssets.HighwayFade, gprAssets.HighwayFadeEndLoc, &HighwayEnd, SHADER_UNIFORM_FLOAT);
	EndBlendMode();
	EndMode3D();
	EndTextureMode();
	SetTextureWrap(highwayStatus_tex.texture,TEXTURE_WRAP_CLAMP);
	highwayStatus_tex.texture.width = (float)GetScreenWidth();
	highwayStatus_tex.texture.height = (float)GetScreenHeight();
	Vector2 res = {(float)GetScreenWidth(), (float)GetScreenHeight()};
	SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, highwayStatus_tex.texture);
	SetShaderValue(gprAssets.fxaa, gprAssets.resLoc, &res, SHADER_UNIFORM_VEC2);
	BeginShaderMode(gprAssets.fxaa);
	DrawTexturePro(highwayStatus_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
	EndShaderMode();

	BeginTextureMode(smasher_tex);
	ClearBackground({0,0,0,0});
	BeginMode3D(cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel]);
	BeginBlendMode(BLEND_ALPHA_PREMULTIPLY);
	DrawModel(gprAssets.smasherBoard, Vector3{ 0, 0.004f, 0 }, 1.0f, WHITE);
	rlSetBlendFactorsSeparate(0x0302, 0x0303, 1, 0x0303, 0x8006, 0x8006);
	BeginBlendMode(BLEND_CUSTOM_SEPARATE);
	for (int i = 0; i < 4; i++) {
		Color NoteColor = RED;
		if (i == 1) NoteColor = YELLOW;
		else if (i == 2) NoteColor = BLUE;
		else if (i == 3) NoteColor = GREEN;
		gprAssets.smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;
		gprAssets.smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = NoteColor;

		if (stats->HeldFrets[i] || stats->HeldFretsAlt[i]) {
			DrawModel(gprAssets.smasherPressed, Vector3{ (diffDistance - (float)(i*1.25))-0.125f, 0.025f, smasherPos }, 1.25f, WHITE);
		}
		else {
			DrawModel(gprAssets.smasherReg, Vector3{ (diffDistance - (float)(i*1.25))-0.125f, 0.025f, smasherPos }, 1.25f, WHITE);
		}
	}
	EndBlendMode();
	EndMode3D();
	EndTextureMode();
	SetTextureWrap(smasher_tex.texture,TEXTURE_WRAP_CLAMP);
	smasher_tex.texture.width = (float)GetScreenWidth();
	smasher_tex.texture.height = (float)GetScreenHeight();
	Vector2 res2 = {(float)GetScreenWidth(), (float)GetScreenHeight()};
	SetShaderValueTexture(gprAssets.fxaa, gprAssets.texLoc, smasher_tex.texture);
	SetShaderValue(gprAssets.fxaa, gprAssets.resLoc, &res2, SHADER_UNIFORM_VEC2);
	BeginShaderMode(gprAssets.fxaa);
	DrawTexturePro(smasher_tex.texture, {0,0,(float)GetScreenWidth(), (float)-GetScreenHeight() },{ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() }, {renderPos,highwayLevel}, 0, WHITE );
	EndShaderMode();
}

void gameplayRenderer::nDrawDrumsHitEffects(Note note, double time, float notePosX) {
	// oh my good fucking lord
	double PerfectHitAnimDuration = 1.0f;
	if (note.hit && time <
		note.hitTime + HitAnimDuration) {

		double TimeSinceHit = time - note.hitTime;
		unsigned char HitAlpha = Remap(getEasingFunction(EaseInBack)(TimeSinceHit / HitAnimDuration), 0, 1.0, 196, 0);

		float Width = note.lane == KICK ? 5.0f: 1.3f;
		float Height = note.lane == KICK ? 0.125f : 0.25f;
		float Length = note.lane == KICK ? 0.5f : 0.75f;
		float yPos = note.lane == KICK ? 0 : 0.125f;
		Color BoxColor = note.perfect ? Color{ 255, 215, 0, HitAlpha } : Color{ 255, 255, 255, HitAlpha };
		DrawCube(Vector3{ notePosX, yPos, smasherPos }, Width, Height, Length, BoxColor);

		}
	EndBlendMode();
	float KickBounceDuration = 0.5f;
	if (note.hit && time <note.hitTime + PerfectHitAnimDuration && note.perfect) {
		double TimeSinceHit = time - note.hitTime;
		unsigned char HitAlpha = Remap(getEasingFunction(EaseOutQuad)(TimeSinceHit / PerfectHitAnimDuration), 0, 1.0, 255, 0);
		float HitPosLeft = Remap(getEasingFunction(EaseInOutBack)(TimeSinceHit / PerfectHitAnimDuration), 0, 1.0, 3.4, 3.0);

		Color InnerBoxColor = { 255,161,0,HitAlpha };
		Color OuterBoxColor = { 255,161,0,(unsigned char)(HitAlpha / 2) };
		float Width = 1.0f;
		float Height = 0.01f;
		DrawCube(Vector3{ HitPosLeft, -0.1f, smasherPos }, Width, Height, 0.5f, InnerBoxColor);
		DrawCube(Vector3{ HitPosLeft, -0.11f, smasherPos }, Width, Height, 1.0f, OuterBoxColor);
	}
	if (note.hit && note.lane == KICK && time < note.hitTime + KickBounceDuration) {
		double TimeSinceHit = time - note.hitTime;
		float height = 8.35f;
		if (gprPlayerManager.PlayersActive > 3) {
			height = 10;
		}
		float CameraPos = Remap(getEasingFunction(EaseOutBounce)(TimeSinceHit / KickBounceDuration), 0, 1.0, height-0.35f, height);
		cameraVectors[gprPlayerManager.PlayersActive-1][cameraSel].position.y = CameraPos;
	}
}

void gameplayRenderer::nDrawFiveLaneHitEffects(Note note, double time, float notePosX) {
	double PerfectHitAnimDuration = 1.0f;
	if (note.hit && time < note.hitTime + HitAnimDuration) {

		double TimeSinceHit = time - note.hitTime;
		unsigned char HitAlpha = Remap(getEasingFunction(EaseInBack)(TimeSinceHit/HitAnimDuration), 0, 1.0, 196, 0);
		Color PerfectColor = Color{ 255, 215, 0, HitAlpha };
		Color GoodColor = Color{ 255, 255, 255, HitAlpha };
		Color BoxColor = note.perfect ? PerfectColor : GoodColor;

		DrawCube(Vector3{notePosX, 0.125, smasherPos}, 1.0f, 0.25f, 0.5f, BoxColor);
	}

	if (note.hit && time < note.hitTime + PerfectHitAnimDuration && note.perfect) {

		double TimeSinceHit = time - note.hitTime;
		unsigned char HitAlpha = Remap(getEasingFunction(EaseOutQuad)(TimeSinceHit/PerfectHitAnimDuration), 0, 1.0, 255, 0);
		float HitPosLeft = Remap(getEasingFunction(EaseInOutBack)(TimeSinceHit/PerfectHitAnimDuration), 0, 1.0, 3.4, 3.0);

		DrawCube(Vector3{HitPosLeft, -0.1f, smasherPos}, 1.0f, 0.01f,
				 0.5f, Color{255,161,0,HitAlpha});
		DrawCube(Vector3{HitPosLeft, -0.11f, smasherPos}, 1.0f, 0.01f,
				 1.0f, Color{255,161,0,(unsigned char)(HitAlpha/2)});
	}
}

void gameplayRenderer::nDrawPlasticNote(Note note, Color noteColor, float notePosX, float noteTime) {
	// why the fuck did i separate sustains and non-sustain note drawing?????
	Model NoteTop = gprAssets.noteTopModel;
	Model NoteBottom = gprAssets.noteBottomModel;

	Vector3 NotePos = {notePosX, 0, noteTime};
	Color BottomColor = WHITE;
	Color TopColor = noteColor;

	if (note.pTap || note.phopo) {
		NoteTop = gprAssets.noteTopModelHP;
		NoteBottom = gprAssets.noteBottomModelHP;
	}
	if (note.phopo && !note.renderAsOD) {
		TopColor = noteColor;
		BottomColor = WHITE;
	} else if (note.pTap && !note.renderAsOD) {
		TopColor = BLACK;
		BottomColor = noteColor;
	}
	if (note.renderAsOD) {
		if (note.pTap) {
			TopColor = BLACK;
			BottomColor = GOLD;
		} else if (note.phopo) {
			TopColor = GOLD;
			BottomColor = WHITE;
		} else {
			NoteTop = gprAssets.noteTopModelOD;
			NoteBottom = gprAssets.noteBottomModelOD;
			TopColor = WHITE;
			BottomColor = GOLD;
		}
	}
	if (note.miss) {
		TopColor = RED;
		BottomColor = RED;
	}
	if (!note.hit) {
		NoteTop.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = TopColor;
		NoteBottom.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BottomColor;
		DrawModel(NoteTop, NotePos,1.1f, TopColor);
		DrawModel(NoteBottom, NotePos, 1.1f, BottomColor);
	}

}

void gameplayRenderer::nDrawPadNote(Note note, Color noteColor, float notePosX, float noteScrollPos) {
	Vector3 NotePos = {notePosX, 0, noteScrollPos};
	gprAssets.liftModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = noteColor;
	Color TopColor = noteColor;
	Color BottomColor = WHITE;
	if (note.lift && !note.hit) {
		if (note.renderAsOD) {
			TopColor = WHITE;
		} else if (note.miss) {
			TopColor = RED;
		}
		gprAssets.liftModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = TopColor;
		gprAssets.liftModel.materials[0].shader = gprAssets.HighwayFade;
		gprAssets.liftModel.materials[0].shader.locs[SHADER_LOC_COLOR_DIFFUSE] = gprAssets.HighwayColorLoc;
		DrawModel(gprAssets.liftModel, NotePos, 1.1f, TopColor);
	} else if (!note.hit) {
		Model NoteTop = gprAssets.noteTopModel;
		Model NoteBottom = gprAssets.noteBottomModelOD;
		TopColor = noteColor;
		BottomColor = WHITE;
		if (note.renderAsOD) {
			NoteTop = gprAssets.noteTopModelOD;
			NoteBottom = gprAssets.noteBottomModelOD;
			TopColor = WHITE;
			BottomColor = GOLD;
		} else if (note.miss) {
			TopColor = RED;
			BottomColor = RED;
		}
		NoteTop.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = TopColor;
		NoteBottom.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = BottomColor;
		NoteTop.materials[0].shader = gprAssets.HighwayFade;
		NoteBottom.materials[0].shader = gprAssets.HighwayFade;
		NoteTop.materials[0].shader.locs[SHADER_LOC_COLOR_DIFFUSE] = gprAssets.HighwayColorLoc;
		NoteBottom.materials[0].shader.locs[SHADER_LOC_COLOR_DIFFUSE] = gprAssets.HighwayColorLoc;
		DrawModel(NoteTop, NotePos, 1.1f, TopColor);
		DrawModel(NoteBottom, NotePos, 1.1f, BottomColor);
	}
}

void gameplayRenderer::nDrawSustain(Note note, Color noteColor, float notePosX, Matrix sustainMatrix) {
	BeginBlendMode(BLEND_ALPHA);
	Material Sustain = gprAssets.sustainMat;

	gprAssets.sustainMat.maps[MATERIAL_MAP_DIFFUSE].color = ColorTint(noteColor, {180, 180, 180, 255});
	gprAssets.sustainMatHeld.maps[MATERIAL_MAP_DIFFUSE].color = ColorBrightness(noteColor, 0.5f);

	// use default... by default
	if (note.held && !note.renderAsOD) // normal held
		Sustain = gprAssets.sustainMatHeld;
	else if (note.held && note.renderAsOD) // OD hold
		Sustain = gprAssets.sustainMatHeldOD;
	else if (!note.held && note.accounted) // released
		Sustain = gprAssets.sustainMatMiss;
	else if (note.renderAsOD) // not hit but OD
		Sustain = gprAssets.sustainMatOD;
	Sustain.shader = gprAssets.HighwayFade;
	Sustain.shader.locs[SHADER_LOC_COLOR_DIFFUSE] = gprAssets.HighwayColorLoc;
	DrawMesh(sustainPlane, Sustain, sustainMatrix);

	if (note.held)
		DrawCube(Vector3{notePosX, 0.1, smasherPos}, 0.4f, 0.2f, 0.4f, noteColor);

	EndBlendMode();
}
