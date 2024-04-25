#pragma once
#include "raylib.h"
#include <filesystem>
#include "player.h"

player player;

class Assets {
private:
	Texture2D LoadTextureFilter(std::filesystem::path texturePath) {
		Texture2D tex = LoadTexture(texturePath.string().c_str());
		SetTextureFilter(tex, TEXTURE_FILTER_ANISOTROPIC_16X);
		return tex;
	}
public:
	Model smasherReg;
	Texture2D smasherRegTex;

    Image icon;
    Texture2D encoreWhiteLogo;
    Texture2D songBackground;
	Model smasherBoard;
	Texture2D smasherBoardTex;

	Model smasherPressed;
	Texture2D smasherPressTex;

    Model lanes;
    Texture2D lanesTex;
	Model odFrame;
	Model odBar;
	Model multFrame;
	Model multBar;
	Model multCtr3;
	Model multCtr5;
	Model multNumber;
	Texture2D odMultFrame;
	Texture2D odMultFill;
	Texture2D odMultFillActive;
	Texture2D multNumberTex;
	Shader odMultShader;
	Shader multNumberShader;
	int odLoc;
	int comboCounterLoc;
	int multLoc;
	int isBassOrVocalLoc;
	int uvOffsetXLoc;
	int uvOffsetYLoc;

    Model expertHighwaySides;
	Model expertHighway;
	Model emhHighway;
    Texture2D highwaySidesTexture;
	Texture2D highwayTexture;
	Texture2D highwayTextureOD;
	Model noteModel;
	Texture2D noteTexture;
	Texture2D emitTexture;
	Model noteModelOD;
	Texture2D noteTextureOD;
	Texture2D emitTextureOD;
	Model liftModel;
	Model liftModelOD;

    Texture2D emptyStar;
    Texture2D star;
    Texture2D goldStar;

    Font rubik32;
	Font rubik;
    Font redHatDisplayItalic;

	//Sound clapOD;
	void loadAssets(std::filesystem::path directory) {
		smasherReg = LoadModel((directory / "Assets/highway/smasher.obj").string().c_str());
		smasherRegTex = LoadTextureFilter(directory / "Assets/highway/smasher_reg.png");
		smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherRegTex;
		smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
		smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherRegTex;
		smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

		smasherBoard = LoadModel((directory / "Assets/highway/new_board.obj").string().c_str());
		smasherBoardTex = LoadTextureFilter(directory / "Assets/highway/smasher_board_new.png");
		smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherBoardTex;
		smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;

        lanes = LoadModel((directory / "Assets/highway/lanes.obj").string().c_str());
        lanesTex = LoadTextureFilter(directory / "Assets/highway/lanes.png");
        lanes.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = lanesTex;
        lanes.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

		smasherPressed = LoadModel((directory / "Assets/highway/smasher.obj").string().c_str());
		smasherPressTex = LoadTextureFilter(directory / "Assets/highway/smasher_press.png");
		smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherPressTex;
		smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

        star = LoadTextureFilter(directory/ "Assets/ui/star.png");
        goldStar = LoadTextureFilter(directory/ "Assets/ui/gold-star.png");
        emptyStar = LoadTextureFilter(directory/ "Assets/ui/empty-star.png");

		odFrame = LoadModel((directory / "Assets/ui/od_frame.obj").string().c_str());
		odBar = LoadModel((directory / "Assets/ui/od_fill.obj").string().c_str());
		multFrame = LoadModel((directory / "Assets/ui/multcircle_frame.obj").string().c_str());
		multBar = LoadModel((directory / "Assets/ui/multcircle_fill.obj").string().c_str());
		multCtr3 = LoadModel((directory / "Assets/ui/multbar_3.obj").string().c_str());
		multCtr5 = LoadModel((directory / "Assets/ui/multbar_5.obj").string().c_str());
		multNumber = LoadModel((directory / "Assets/ui/mult_number_plane.obj").string().c_str());
		odMultFrame = LoadTextureFilter(directory / "Assets/ui/mult_base.png");
		odMultFill = LoadTextureFilter(directory / "Assets/ui/mult_fill.png");
		odMultFillActive = LoadTextureFilter(directory / "Assets/ui/mult_fill_od.png");
		multNumberTex = LoadTextureFilter(directory / "Assets/ui/mult_number.png");
		odMultShader = LoadShader(0, "Assets/ui/odmult.fs");
		multNumberShader = LoadShader(0, "Assets/ui/multnumber.fs");
		odFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
		odBar.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
		multFrame.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
		multBar.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
		multCtr3.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
		multCtr5.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = odMultFrame;
		odBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
		multBar.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
		multCtr3.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
		multCtr5.materials[0].maps[MATERIAL_MAP_EMISSION].texture = odMultFill;
		odBar.materials[0].shader = odMultShader;
		multBar.materials[0].shader = odMultShader;
		multCtr3.materials[0].shader = odMultShader;
		multCtr5.materials[0].shader = odMultShader;
		odLoc = GetShaderLocation(odMultShader, "overdrive");
		comboCounterLoc = GetShaderLocation(odMultShader, "comboCounter");
		multLoc = GetShaderLocation(odMultShader, "multBar");
		isBassOrVocalLoc = GetShaderLocation(odMultShader, "isBassOrVocal");
		odMultShader.locs[SHADER_LOC_MAP_EMISSION] = GetShaderLocation(odMultShader, "fillTex");

		multNumber.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = multNumberTex;
		multNumber.materials[0].shader = multNumberShader;
		uvOffsetXLoc = GetShaderLocation(multNumberShader, "uvOffsetX");
		uvOffsetYLoc = GetShaderLocation(multNumberShader, "uvOffsetY");

        expertHighwaySides = LoadModel(((directory / "Assets/highway/sides.obj").string().c_str()));
		expertHighway = LoadModel((directory / "Assets/highway/expert.obj").string().c_str());
		emhHighway = LoadModel((directory / "Assets/highway/emh.obj").string().c_str());
		highwayTexture = LoadTextureFilter(directory / "Assets/highway/highway_new.png");
		highwayTextureOD = LoadTextureFilter(directory / "Assets/highway/highway_od.png");
        highwaySidesTexture = LoadTextureFilter(directory/"Assets/highway/highwaysides_new.png");
        expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwaySidesTexture;
        expertHighwaySides.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
		expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
		expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
		emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
		emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
		noteModel = LoadModel((directory / "Assets/notes/note.obj").string().c_str());
		noteTexture = LoadTextureFilter(directory / "Assets/notes/note.png");
		emitTexture = LoadTextureFilter(directory / "Assets/notes/note_e_new.png");
		noteModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = noteTexture;
		noteModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
		noteModel.materials[0].maps[MATERIAL_MAP_EMISSION].texture = emitTexture;
		noteModel.materials[0].maps[MATERIAL_MAP_EMISSION].color = WHITE;
		noteModelOD = LoadModel((directory / "Assets/notes/note.obj").string().c_str());
		noteTextureOD = LoadTextureFilter(directory / "Assets/notes/note.png");
		emitTextureOD = LoadTextureFilter(directory / "Assets/notes/note_e_new.png");
		// noteModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = noteTextureOD;
		noteModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
		noteModelOD.materials[0].maps[MATERIAL_MAP_EMISSION].texture = emitTextureOD;
		noteModelOD.materials[0].maps[MATERIAL_MAP_EMISSION].color = WHITE;
		liftModel = LoadModel((directory / "Assets/notes/lift.obj").string().c_str());
		liftModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = player.accentColor;
		liftModelOD = LoadModel((directory / "Assets/notes/lift.obj").string().c_str());
		liftModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

        icon = LoadImage((directory / "Assets/encore_favicon-NEW.png").string().c_str());
        encoreWhiteLogo = LoadTexture((directory / "Assets/encore-white.png").string().c_str());
        songBackground = LoadTexture((directory / "Assets/background.png").string().c_str());

        redHatDisplayItalic = LoadFontEx((directory/"Assets/fonts/RedHatDisplay-BlackItalic.ttf").string().c_str(), 48,0,0);
		rubik = LoadFontEx((directory / "Assets/fonts/Rubik-Regular.ttf").string().c_str(), 100, 0, 0);
        rubik32 = LoadFontEx((directory / "Assets/fonts/Rubik-Regular.ttf").string().c_str(), 32, 0, 0);
		//clapOD = LoadSound((directory / "Assets/highway/clap.ogg").string().c_str());
		//SetSoundVolume(clapOD, 0.375);
	}
};