#ifndef ASSETS_H
#define ASSETS_H

#include <string>
#ifdef PLATFORM_NX
    #include <string>
#else
    #include <filesystem>
#endif
#ifndef PLATFORM_NX
    #include "raygui.h"
#endif
#include "raylib.h"

#include <vector>

#ifdef PLATFORM_NX
    #define PATH_TYPE std::string
    #define ASSET_PATH "/switch/Encore/Assets/"
    #define PATH_CONCAT(path) (directory + path)
    #define PATH_TO_CSTR(path) path.c_str()
#else
    #define PATH_TYPE std::filesystem::path
    #define PATH_CONCAT(path) (directory / path)
    #define PATH_TO_CSTR(path) path.string().c_str()
#endif

class Assets {
public:
    Image icon;
    Texture2D encoreWhiteLogo;
    Font rubik, redHatDisplayItalic, redHatDisplayItalicLarge, redHatDisplayBlack;
    Font rubikBoldItalic, rubikBold, rubikItalic;
    Font josefinSansItalic, josefinSansBold, josefinSansNormal;
    Model smasherInner, smasherOuter, smasherBoard, smasherBoardEMH, lanes, smasherPressed;
    Texture2D smasherInnerTex, smasherOuterTex, smasherTopPressedTex, smasherTopUnpressedTex;
    Texture2D smasherBoardTex, lanesTex, smasherPressTex;
    Texture2D star, goldStar, goldStarUnfilled, emptyStar;
    Shader Highway, odMultShader, multNumberShader, FullComboIndicator, MultiplierFill, HighwayFade;
    int HighwayTexShaderLoc, HighwayTimeShaderLoc, HighwayColorShaderLoc;
    int HighwayScrollFadeStartLoc, HighwayScrollFadeEndLoc;
    Model odFrame, odBar, multFrame, multBar, multCtr3, multCtr5, multNumber;
    Texture2D odMultFrame, odMultFill, odMultFillActive, multNumberTex;
    Model MultInnerDot, MultFill, MultOuterFrame, MultInnerFrame;
    Texture2D MultFillBase, MultFCTex1, MultFCTex2, MultFCTex3;
    int BottomTextureLoc, MiddleTextureLoc, TopTextureLoc, TimeLoc, FCColorLoc, FCIndLoc, BasicColorLoc;
    int MultTextureLoc, MultiplierColorLoc, FillPercentageLoc;
    int odLoc, comboCounterLoc, multLoc, isBassOrVocalLoc, uvOffsetXLoc, uvOffsetYLoc;
    int HighwayFadeStartLoc, HighwayFadeEndLoc, HighwayColorLoc, HighwayAccentFadeLoc;
    Model expertHighwaySides, DarkerHighwayThing, expertHighway, emhHighwaySides, emhHighway;
    Model odHighwayEMH, odHighwayX;
    Texture2D highwayTexture, highwayTextureOD, highwaySidesTexture;
    Model noteTopModel, noteBottomModel, KickBottomModel, KickSideModel;
    Texture2D KickBottom, KickSide;
    Model CymbalInner, CymbalOuter, CymbalBottom;
    Material CodaLane, SoloSides;
    Texture2D CodaLaneTex;
    std::vector<Texture2D> YargRings;
    Texture2D BaseRingTexture;
    std::vector<Texture2D> InstIcons;
    Model SoloBox;
    Texture2D SoloBackground, Scorebox, Timerbox, TimerboxOutline;
    Model noteTopModelOD, noteBottomModelOD, noteTopModelHP, noteBottomModelHP;
    Texture2D noteTextureOD, emitTextureOD;
    Model liftModel, liftModelOD;
    Model beatline;
    Texture2D beatlineTex, songBackground;
    Font redHatMono;
    Shader fxaa, sdfShader, bgShader;
    int texLoc, resLoc, bgTimeLoc;
    Sound clapOD;
    Texture2D discord, github;
    Texture2D soloTexture, sustainTexture, sustainHeldTexture;
    Material sustainMat, soloMat, sustainMatHeld, sustainMatOD, sustainMatHeldOD, sustainMatMiss;
    int loadedAssets = 0;
    PATH_TYPE directory;

    static Assets& getInstance(); // Singleton pattern
    static Texture2D LoadTextureFilter(PATH_TYPE texturePath, int &loadedAssets);
    static Model LoadModel_(PATH_TYPE modelPath, int &loadedAssets);
    static Font LoadFontFilter(PATH_TYPE fontPath, int fontSize, int &loadedAssets);
    void FirstAssets();
    void LoadAssets();
};

#endif // ASSETS_H