//
// Created by marie on 02/05/2024.
//

#include "raylib.h"
#include <filesystem>



Model smasherReg;
Texture2D smasherRegTex;

Image icon;
Texture2D encoreWhiteLogo;
Texture2D songBackground;
Model smasherBoard;
Texture2D smasherBoardTex;
Model smasherBoardEMH;

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

Model odHighwayX;
Model odHighwayEMH;

Model expertHighwaySides;
Model expertHighway;
Model emhHighwaySides;
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
Font redHatDisplayLarge;
Font rubikBoldItalic32;
Font rubikBold32;

Texture2D discord;
Texture2D github;

static void loadAssets(const std::filesystem::path& directory);
