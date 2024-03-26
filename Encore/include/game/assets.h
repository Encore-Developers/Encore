#pragma once
#include "raylib.h"
#include <filesystem>
class Assets {
public:
	Model smasherReg;
	Texture2D smasherRegTex;

	Model smasherBoard;
	Texture2D smasherBoardTex;

	Model smasherPressed;
	Texture2D smasherPressTex;

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

	Model expertHighway;
	Model emhHighway;
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
	//Sound clapOD;
	void loadAssets(std::filesystem::path directory) {
		smasherReg = LoadModel((directory / "Assets/highway/smasher.obj").string().c_str());
		smasherRegTex = LoadTexture((directory / "Assets/highway/smasher_reg.png").string().c_str());
		smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherRegTex;
		smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
		smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherRegTex;
		smasherReg.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

		smasherBoard = LoadModel((directory / "Assets/highway/board.obj").string().c_str());
		smasherBoardTex = LoadTexture((directory / "Assets/highway/smasherBoard.png").string().c_str());
		smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherBoardTex;
		smasherBoard.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

		smasherPressed = LoadModel((directory / "Assets/highway/smasher.obj").string().c_str());
		smasherPressTex = LoadTexture((directory / "Assets/highway/smasher_press.png").string().c_str());
		smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = smasherPressTex;
		smasherPressed.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;

		odFrame = LoadModel((directory / "Assets/ui/od_frame.obj").string().c_str());
		odBar = LoadModel((directory / "Assets/ui/od_fill.obj").string().c_str());
		multFrame = LoadModel((directory / "Assets/ui/multcircle_frame.obj").string().c_str());
		multBar = LoadModel((directory / "Assets/ui/multcircle_fill.obj").string().c_str());
		multCtr3 = LoadModel((directory / "Assets/ui/multbar_3.obj").string().c_str());
		multCtr5 = LoadModel((directory / "Assets/ui/multbar_5.obj").string().c_str());
		multNumber = LoadModel((directory / "Assets/ui/mult_number_plane.obj").string().c_str());
		odMultFrame = LoadTexture((directory / "Assets/ui/mult_base.png").string().c_str());
		odMultFill = LoadTexture((directory / "Assets/ui/mult_fill.png").string().c_str());
		odMultFillActive = LoadTexture((directory / "Assets/ui/mult_fill_od.png").string().c_str());
		multNumberTex = LoadTexture((directory / "Assets/ui/mult_number.png").string().c_str());
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

		expertHighway = LoadModel((directory / "Assets/highway/expert.obj").string().c_str());
		emhHighway = LoadModel((directory / "Assets/highway/emh.obj").string().c_str());
		highwayTexture = LoadTexture((directory / "Assets/highway/highway.png").string().c_str());
		highwayTextureOD = LoadTexture((directory / "Assets/highway/highway_od.png").string().c_str());
		expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
		expertHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
		emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = highwayTexture;
		emhHighway.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
		noteModel = LoadModel((directory / "Assets/notes/note.obj").string().c_str());
		noteTexture = LoadTexture((directory / "Assets/notes/note_d.png").string().c_str());
		emitTexture = LoadTexture((directory / "Assets/notes/note_e.png").string().c_str());
		noteModel.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = noteTexture;
		noteModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
		noteModel.materials[0].maps[MATERIAL_MAP_EMISSION].texture = emitTexture;
		noteModel.materials[0].maps[MATERIAL_MAP_EMISSION].color = WHITE;
		noteModelOD = LoadModel((directory / "Assets/notes/note.obj").string().c_str());
		noteTextureOD = LoadTexture((directory / "Assets/notes/note_od_d.png").string().c_str());
		emitTextureOD = LoadTexture((directory / "Assets/notes/note_od_e.png").string().c_str());
		noteModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = noteTextureOD;
		noteModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].color = WHITE;
		noteModelOD.materials[0].maps[MATERIAL_MAP_EMISSION].texture = emitTextureOD;
		noteModelOD.materials[0].maps[MATERIAL_MAP_EMISSION].color = WHITE;
		liftModel = LoadModel((directory / "Assets/notes/lift.obj").string().c_str());
		liftModel.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Color{ 172,82,217,127 };
		liftModelOD = LoadModel((directory / "Assets/notes/lift.obj").string().c_str());
		liftModelOD.materials[0].maps[MATERIAL_MAP_ALBEDO].color = Color{ 217, 183, 82 ,127 };
		//clapOD = LoadSound((directory / "Assets/highway/clap.ogg").string().c_str());
		//SetSoundVolume(clapOD, 0.375);
	}
};