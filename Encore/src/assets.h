#pragma once
#include "raylib.h"
#include "menus/uiUnits.h"

#include <cassert>
#include <atomic>
#include <filesystem>
#include <map>
#include <thread>
#include <vector>

enum AssetState {
    UNLOADED,
    LOADING,
    PREFINALIZED,
    LOADED
};


class Asset {
protected:
    virtual void Load() {}
    virtual void Finalize() {}
public:
    std::atomic<AssetState> state = UNLOADED;
    std::string id;
    std::thread loadingThread ;
    Asset(const std::string &id) {
        this->id = id;
    }

    /// Starts loading this asset.
    virtual void StartLoad();

    /// Starts loading this asset and blocks until it is loaded.
    virtual void LoadImmediate();
    /// Checks if this asset is loaded. Only use in the render thread!
    void CheckForFetch();

    /// Call when you're polling the asset's for when it's loaded.
    bool CanFetch() const {
        return state == LOADED || state == PREFINALIZED;
    }

};

class FileAsset : public Asset {
    std::filesystem::path path;
protected:
    char *fileBuffer;
    size_t fileSize;
    void LoadFile();
    virtual void Load();
    void FreeFileBuffer();

public:
    FileAsset(const std::string &id) : Asset(id) {
        path = std::filesystem::path("Assets") / id;
    }
    std::filesystem::path &GetPath() {
        return path;
    }
    size_t GetFileSize();
    char *FetchRaw();
    operator const unsigned char*() {
        return (const unsigned char *)FetchRaw();
    }
};

class TextureAsset : public FileAsset {
    Texture2D tex;
    Image image;
    virtual void Load();
    virtual void Finalize();
    bool filter;
public:
    TextureAsset(const std::string &id, bool filter) : FileAsset(id) {
        this->filter = filter;
    }
    Texture2D Fetch() {
        CheckForFetch();
        return tex;
    }
    operator Texture2D() {
        return Fetch();
    }
};

class FontAsset : public FileAsset {
    Font font;
    int fontSize;
    Image atlas;
    virtual void Load();
    virtual void Finalize();
public:
    FontAsset(const std::string &id, int fontSize) : FileAsset(id) {
        this->fontSize = fontSize;
    }
    Font Fetch() {
        CheckForFetch();
        return font;
    }
    operator Font() {
        return Fetch();
    }
};

class Assets {
private:
    Assets() {}
    std::vector<Image> images;
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());
    Font
    LoadFontFilter(const std::filesystem::path &fontPath, int fontSize, int &loadedAssets);
    std::map<std::string, Asset*> assetMap = {};

public:
    Asset *RegisterAsset(Asset *asset) {
        assetMap.emplace(asset->id, asset);
        return asset;
    }

    void RegisterAllAssets();

    template<class A> static A *Get(const std::string &id) {
        Assets &instance = getInstance();
        return static_cast<A *>(instance.assetMap.at(id));
    }


    static Assets &getInstance() {
        static Assets instance; // This is the single instance
        return instance;
    }

    void setDirectory(std::filesystem::path assetsDirectory) {
        directory = assetsDirectory;
    }

    std::filesystem::path getDirectory() {
        return directory;
    }

    Assets(const Assets &) = delete;
    void operator=(const Assets &) = delete;

    int loadedAssets;
    int totalAssets = 32;
    Model smasherInner;
    Model smasherOuter;
    Texture2D smasherInnerTex;
    Texture2D smasherOuterTex;
    Texture2D smasherTopPressedTex;
    Texture2D smasherTopUnpressedTex;

    Model smasherBoard;
    Texture2D smasherBoardTex;
    Model smasherBoardEMH;

    Model beatline;
    Texture2D beatlineTex;

    Model lanes;
    Texture2D lanesTex;

    Model smasherPressed;
    Texture2D smasherPressTex;

    Texture2D goldStarUnfilled;
    Texture2D star;
    Texture2D goldStar;
    Texture2D emptyStar;

    Texture2D Scorebox;
    Texture2D Timerbox;
    Texture2D TimerboxOutline;

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

    Texture2D MultFillBase;
    Texture2D MultFCTex1;
    Texture2D MultFCTex2;
    Texture2D MultFCTex3;

    Model MultInnerDot;
    Model MultFill;
    Model MultOuterFrame;
    Model MultInnerFrame;

    int MultTextureLoc;
    int MultiplierColorLoc;
    int FillPercentageLoc;

    Shader MultiplierFill;
    Shader FullComboIndicator;
    int BottomTextureLoc;
    int MiddleTextureLoc;
    int TopTextureLoc;
    int BasicColorLoc;
    int TimeLoc;
    int FCColorLoc;
    int FCIndLoc;
    Shader Highway;
    int HighwayTexShaderLoc;
    int HighwayTimeShaderLoc;
    int HighwayColorShaderLoc;
    int HighwayScrollFadeStartLoc;
    int HighwayScrollFadeEndLoc;
    int CurveMaxLoc;
    Shader HighwayFade;
    int HighwayFadeStartLoc;
    int HighwayFadeEndLoc;
    int HighwayColorLoc;
    int HighwayAccentFadeLoc;

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
    Model DarkerHighwayThing;
    Texture2D highwayTexture;
    Texture2D highwayTextureOD;
    Texture2D highwaySidesTexture;

    Model noteBottomModelHP;
    Model noteTopModelHP;

    Model noteBottomModel;
    Model noteTopModel;
    Texture2D noteTexture;
    Texture2D emitTexture;

    Model KickBottomModel;
    Model KickSideModel;
    Texture2D KickBottom;
    Texture2D KickSide;

    Model CymbalInner;
    Model CymbalOuter;
    Model CymbalBottom;

    Model SoloBox;
    Texture2D SoloBackground;

    Model noteBottomModelOD;
    Model noteTopModelOD;
    Texture2D noteTextureOD;
    Texture2D emitTextureOD;

    Model liftModel;
    Model liftModelOD;

    std::vector<Texture2D> YargRings;
    Texture2D BaseRingTexture;
    std::vector<Texture2D> InstIcons;

    Image icon;
    Texture2D encoreWhiteLogo;
    Texture2D songBackground;

    Font redHatDisplayItalic;
    Font redHatDisplayBlack;
    Font redHatDisplayItalicLarge;
    Font josefinSansItalic;
    Font josefinSansNormal;
    Font josefinSansBold;
    Font redHatMono;
    Font rubik;
    Font rubikItalic;
    Font JetBrainsMono;
    Font rubikBoldItalic;

    Font rubikBold;

    // clapOD = LoadSound((directory / "Assets/highway/clap.ogg").string().c_str());
    // SetSoundVolume(clapOD, 0.375);

    Texture2D discord;
    Texture2D github;

    Texture2D sustainTexture;
    Texture2D sustainHeldTexture;
    Texture2D soloTexture;
    Material soloMat;
    Material sustainMat;
    Material sustainMatOD;
    Material sustainMatHeld;
    Material sustainMatHeldOD;
    Material sustainMatMiss;
    
    Material SoloSides;
    Material CodaLane;
    Texture2D CodaLaneTex;

    Shader sdfShader;
    Shader bgShader;
    int bgTimeLoc;
    // Sound clapOD;
    void DrawTextRubik(
        const char *text, float posX, float posY, float fontSize, Color color
    ) const {
        BeginShaderMode(sdfShader);
        DrawTextEx(rubik, text, { posX, posY }, fontSize, 1, color);
        EndShaderMode();
    }
    void DrawTextRHDI(const char* text, float x, float y, float fontSize, Color color) {
        DrawTextEx(redHatDisplayItalic, text, {x, y}, fontSize, 0, color);
        BeginShaderMode(sdfShader);
        EndShaderMode();
    }
    void DrawTextJSN(const char *text, float posX, float posY, Color color) const {
        BeginShaderMode(sdfShader);
        DrawTextEx(josefinSansNormal, text, { posX, posY }, Units::getInstance().hinpct(0.05f), 1, color);
        EndShaderMode();
    }

    void DrawTextJSB(const char *text, float posX, float posY, Color color) const {
        BeginShaderMode(sdfShader);
        DrawTextEx(josefinSansBold, text, { posX, posY }, Units::getInstance().hinpct(0.05f), 1, color);
        EndShaderMode();
    }

    void DrawTextJSI(const char *text, float posX, float posY, Color color) const {
        BeginShaderMode(sdfShader);
        DrawTextEx(josefinSansItalic, text, { posX, posY }, Units::getInstance().hinpct(0.05f), 1, color);
        EndShaderMode();
    }
    float MeasureTextRubik(const char *text, float fontSize) const {
        return MeasureTextEx(rubik, text, fontSize, 1).x;
    }
    float MeasureTextRHDI(const char *text) const {
        return MeasureTextEx(redHatDisplayItalic, text, 48, 1).x;
    }
    float MeasureTextJSN(const char *text, float fontSize) const {
        return MeasureTextEx(josefinSansNormal, text, 48, 1).x;
    }
    float MeasureTextJSB(const char *text, float fontSize) const {
        return MeasureTextEx(josefinSansBold, text, 48, 1).x;
    }
    float MeasureTextJSI(const char *text, float fontSize) const {
        return MeasureTextEx(josefinSansItalic, text, 48, 1).x;
    }
    static Texture2D
    LoadTextureFilter(const std::filesystem::path &texturePath, int &loadedAssets);
    static Model LoadModel_(const std::filesystem::path &modelPath, int &loadedAssets);
    void FirstAssets();
    void LoadAssets();
};

/// Used for easily loading groups of assets and polling their state as one.
class AssetSet {
    std::vector<Asset *> assets;
public:

    AssetSet(std::initializer_list<const char *> l) {
        const auto names = std::vector(l);
        assets.reserve(l.size());
        for (int i = 0; i < l.size(); i++) {
            assets.push_back(Assets::Get<Asset>(names[i]));
        }
    }

    void StartLoad() {
        for (int i = 0; i < assets.size(); i++) {
            auto asset = assets[i];
            if (asset->state == UNLOADED) {
                asset->StartLoad();
            }
        }
    }

    bool PollLoaded() {
        for (int i = 0; i < assets.size(); i++) {
            auto asset = assets[i];
            if (!asset->CanFetch()) {
                return false;
            }
        }
        return true;
    }

    int CountLoaded() {
        int loaded = 0;
        for (int i = 0; i < assets.size(); i++) {
            if (assets[i]->CanFetch()) {
                loaded++;
            }
        }
        return loaded;
    }

    int AssetCount() {
        return assets.size();
    }

    float GetProgress() {
        return static_cast<float>(CountLoaded()) / static_cast<float>(AssetCount());
    }

    void BlockUntilLoaded() {
        while (!PollLoaded()) {}
    }
};