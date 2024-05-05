#pragma once
#include "raylib.h"
#include <filesystem>



class Assets {
private:
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());
public:
    Model smasherReg;
    Texture2D smasherRegTex;


    Model smasherBoard;
    Texture2D smasherBoardTex;
    Model smasherBoardEMH;

    Model lanes;
    Texture2D lanesTex;

    Model smasherPressed;
    Texture2D smasherPressTex;

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

    Model noteModel;
    Texture2D noteTexture;
    Texture2D emitTexture;

    Model noteModelOD;
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

    Font rubik32;

    Font rubikBoldItalic32;

    Font rubikBold32;

    //clapOD = LoadSound((directory / "Assets/highway/clap.ogg").string().c_str());
    //SetSoundVolume(clapOD, 0.375);

    Texture2D discord;
    Texture2D github;

	//Sound clapOD;
    void DrawTextRubik32(const char* text, float posX, float posY, Color color) const {
        DrawTextEx(rubik32, text, { posX,posY }, 32, 1, color);
    }
    void DrawTextRubik(const char* text, float posX, float posY, int fontSize, Color color)const  {
        DrawTextEx(rubik, text, { posX,posY }, fontSize, 1, color);
    }
    void DrawTextRHDI(const char* text, float posX, float posY, Color color)const  {
        DrawTextEx(redHatDisplayItalic, text, { posX,posY }, 48, 1, color);
    }
    int MeasureTextRubik32(const char* text) const {
        return MeasureTextEx(rubik32, text, 32, 1).x;
    }
    int MeasureTextRubik(const char* text, int fontSize) const {
        return MeasureTextEx(rubik, text, fontSize, 1).x;
    }
    int MeasureTextRHDI(const char* text) const {
        return MeasureTextEx(redHatDisplayItalic, text, 48, 1).x;
    }

    static Texture2D LoadTextureFilter(const std::filesystem::path& texturePath);

    void MaterialMapper();
};