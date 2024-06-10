#pragma once
#include "raylib.h"
#include <filesystem>
#include <vector>



class Assets {
private:
    Assets() {}
    std::vector<Image> images;
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());
    Font LoadFontFilter(const std::filesystem::path& fontPath, int fontSize, int& loadedAssets);
public:
    static Assets& getInstance() {
        static Assets instance; // This is the single instance
        return instance;
    }

    Assets(const Assets&) = delete;
    void operator=(const Assets&) = delete;

    int loadedAssets;
    int totalAssets = 32;
    Model smasherReg;
    Texture2D smasherRegTex;


    Model smasherBoard;
    Texture2D smasherBoardTex;
    Model smasherBoardEMH;

    Model lanes;
    Texture2D lanesTex;

    Model smasherPressed;
    Texture2D smasherPressTex;

    Texture2D goldStarUnfilled;
    Texture2D star;
    Texture2D goldStar;
    Texture2D emptyStar;

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
    Shader fxaa;

    int texLoc;
    int resLoc;

    int odLoc;
    int comboCounterLoc;
    int multLoc;
    int isBassOrVocalLoc;
    int uvOffsetXLoc;
    int uvOffsetYLoc;


    Model expertHighwaySides;
    Model expertHighway;
    Model emhHighwaySides;
    Model emhHighway;
    Model odHighwayEMH;
    Model odHighwayX;
    Texture2D highwayTexture;
    Texture2D highwayTextureOD;
    Texture2D highwaySidesTexture;

    Model noteBottomModel;
    Model noteTopModel;
    Texture2D noteTexture;
    Texture2D emitTexture;

    Model noteBottomModelOD;
    Model noteTopModelOD;
    Texture2D noteTextureOD;
    Texture2D emitTextureOD;

    Model liftModel;
    Model liftModelOD;


    Image icon;
    Texture2D encoreWhiteLogo;
    Texture2D songBackground;

    Font redHatDisplayItalic;
    Font redHatDisplayBlack;
    Font redHatDisplayItalicLarge;
    Font josefinSansItalic;


    Font rubik;
    Font rubikItalic;

    Font rubikBoldItalic;

    Font rubikBold;

    //clapOD = LoadSound((directory / "Assets/highway/clap.ogg").string().c_str());
    //SetSoundVolume(clapOD, 0.375);

    Texture2D discord;
    Texture2D github;

    Texture2D sustainTexture;
    Material sustainMat;
    Material sustainMatOD;
    Material sustainMatHeld;
    Material sustainMatHeldOD;
    Material sustainMatMiss;

    Shader sdfShader;
    Shader bgShader;
    int bgTimeLoc;
	//Sound clapOD;
    void DrawTextRubik(const char* text, float posX, float posY, float fontSize, Color color)const  {
        BeginShaderMode(sdfShader);
        DrawTextEx(rubik, text, { posX,posY }, fontSize, 1, color);
        EndShaderMode();
    }
    void DrawTextRHDI(const char* text, float posX, float posY, Color color)const  {
        BeginShaderMode(sdfShader);
        DrawTextEx(redHatDisplayItalic, text, { posX,posY }, 48, 1, color);
        EndShaderMode();
    }
    float MeasureTextRubik(const char* text, float fontSize) const {
        return MeasureTextEx(rubik, text, fontSize, 1).x;
    }
    float MeasureTextRHDI(const char* text) const {
        return MeasureTextEx(redHatDisplayItalic, text, 48, 1).x;
    }

    static Texture2D LoadTextureFilter(const std::filesystem::path& texturePath, int& loadedAssets);
    static Model LoadModel_(const std::filesystem::path& modelPath, int& loadedAssets);
    void FirstAssets();
    void LoadAssets();
};
