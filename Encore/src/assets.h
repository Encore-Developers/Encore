#pragma once
#include "raylib.h"
#include "menus/uiUnits.h"
#include "util/enclog.h"

#include <cassert>
#include <atomic>
#include <filesystem>
#include <functional>
#include <map>
#include <thread>
#include <unordered_map>
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
    //Asset(Asset& other) {}
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
    size_t fileSize;
    void LoadFile();
    virtual void Load();
    void FreeFileBuffer();

public:
    char *fileBuffer;
    bool addNullTerminator = false;

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
    virtual ~FileAsset() {
        //FreeFileBuffer();
    }
};

class ShaderAsset : public Asset {
    FileAsset *fragmentCode;
    FileAsset *vertexCode;
    std::unordered_map<std::string, int> uniformPositions;
    const char *fStr;
    const char *vStr;
    std::function<void(Asset*)> postFinalizeFunc;
    Shader shader;
protected:
    virtual void Load();
    virtual void Finalize();
public:
    ShaderAsset(const std::string &fsPath, const std::string &vsPath, std::initializer_list<const char *> uniforms, std::function<void(Asset*)> postFinalizeFunc) : Asset(fsPath) {
        if (!fsPath.empty()) {
            fragmentCode = new FileAsset(fsPath);
            fragmentCode->addNullTerminator = true;
        }
        if (!vsPath.empty()) {
            vertexCode = new FileAsset(vsPath);
            vertexCode->addNullTerminator = true;
        }
        this->postFinalizeFunc = postFinalizeFunc;
        for (auto uniform : uniforms) {
            uniformPositions.emplace(uniform, 0);
        }
    }

    int GetUniformLoc(const std::string& uniformName) {
        if (!uniformPositions.contains(uniformName)) {
            Encore::EncoreLog(LOG_ERROR, TextFormat("Attempted to get unknown uniform %s on asset %s", uniformName.c_str(), id.c_str()));
            return -1;
        }
        auto found = uniformPositions.find(uniformName);
        return found->second;
    }

    Shader Fetch() {
        CheckForFetch();
        return shader;
    }

    operator Shader() {
        return Fetch();
    }
};

class TextureAsset : public FileAsset {
    Texture2D tex;
    Image image;
    virtual void Load();
    virtual void Finalize();
    bool filter;
public:
    int width = 0;
    int height = 0;
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

#define ASSET(varname) TheAssets.varname
#define ASSETPTR(varname) &TheAssets.varname


#define NEWFILEASSET(varname, path) FileAsset varname = FileAsset(path)
#define NEWFONTASSET(varname, path, size) FontAsset varname = FontAsset(path, size)
#define NEWTEXASSET(varname, path) TextureAsset varname = TextureAsset(path, true)
#define NEWTEXASSET_NOFILTER(varname, path) TextureAsset varname = TextureAsset(path, false)
// Have to use variadic macros here because the preprocessor can't understand that the
// commas in the initializer list are not parameter seperators
#define NEWSHADERASSET(varname, ...) ShaderAsset varname = ShaderAsset(__VA_ARGS__, {})
#define NEWSHADERASSET_POSTFINALIZE(varname, ...) ShaderAsset varname = ShaderAsset(__VA_ARGS__)



class Assets {
private:
    std::vector<Image> images;
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());
    Font
    LoadFontFilter(const std::filesystem::path &fontPath, int fontSize, int &loadedAssets);

public:
    Assets() {}

    static Assets &getInstance();

    void setDirectory(std::filesystem::path assetsDirectory) {
        directory = assetsDirectory;
    }

    std::filesystem::path getDirectory() {
        return directory;
    }

    Assets(const Assets &) = delete;
    void operator=(const Assets &) = delete;

    NEWFILEASSET(favicon, "encore_favicon-NEW.png");
    NEWTEXASSET(faviconTex, "encore_favicon-NEW.png");

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
    NEWTEXASSET(BaseRingTexture, "ui/hugh ring/rings.png");
    std::vector<Texture2D> InstIcons;

    Image icon;
    NEWTEXASSET(encoreWhiteLogo, "encore-white.png");
    Texture2D songBackground;

    NEWFONTASSET(redHatDisplayItalic, "fonts/RedHatDisplay-BlackItalic.ttf", 128);
    NEWFONTASSET(redHatDisplayBlack, "fonts/RedHatDisplay-Black.ttf", 128);
    NEWFONTASSET(redHatDisplayItalicLarge, "fonts/RedHatDisplay-Black.ttf", 128);
    NEWFONTASSET(josefinSansItalic, "fonts/JosefinSans-Italic.ttf", 128);
    NEWFONTASSET(josefinSansNormal, "fonts/JosefinSans-Normal.ttf", 128);
    NEWFONTASSET(josefinSansBold, "fonts/JosefinSans-Bold.ttf", 128);
    NEWFONTASSET(redHatMono, "fonts/RedHatMono-Bold.ttf", 128);
    NEWFONTASSET(rubik, "fonts/Rubik-Regular.ttf", 128);
    NEWFONTASSET(rubikItalic, "fonts/Rubik-Italic.ttf", 128);
    NEWFONTASSET(JetBrainsMono, "fonts/JetBrainsMonoNL-Regular.ttf", 64);
    NEWFONTASSET(rubikBoldItalic, "fonts/Rubik-BoldItalic.ttf", 128);

    NEWFONTASSET(rubikBold, "fonts/Rubik-Bold.ttf", 128);

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

    NEWSHADERASSET(sdfShader, "fonts/sdf.fs", "", {});
    NEWSHADERASSET(bgShader, "ui/wavy.fs", "", {"time", "test"});
    // Sound clapOD;
    void DrawTextRubik(
        const char *text, float posX, float posY, float fontSize, Color color
    ) {
        BeginShaderMode(sdfShader);
        DrawTextEx(rubik, text, { posX, posY }, fontSize, 1, color);
        EndShaderMode();
    }
    void DrawTextRHDI(const char* text, float x, float y, float fontSize, Color color) {
        DrawTextEx(redHatDisplayItalic, text, {x, y}, fontSize, 0, color);
        BeginShaderMode(sdfShader);
        EndShaderMode();
    }
    void DrawTextJSN(const char *text, float posX, float posY, Color color) {
        BeginShaderMode(sdfShader);
        DrawTextEx(josefinSansNormal, text, { posX, posY }, Units::getInstance().hinpct(0.05f), 1, color);
        EndShaderMode();
    }

    void DrawTextJSB(const char *text, float posX, float posY, Color color) {
        BeginShaderMode(sdfShader);
        DrawTextEx(josefinSansBold, text, { posX, posY }, Units::getInstance().hinpct(0.05f), 1, color);
        EndShaderMode();
    }

    void DrawTextJSI(const char *text, float posX, float posY, Color color) {
        BeginShaderMode(sdfShader);
        DrawTextEx(josefinSansItalic, text, { posX, posY }, Units::getInstance().hinpct(0.05f), 1, color);
        EndShaderMode();
    }
    // I don't like these measure functions. Why are they so inconsistent?
    // These would probably be better inlined... - Sulfrix
    float MeasureTextRubik(const char *text, float fontSize) {
        return MeasureTextEx(rubik, text, fontSize, 1).x;
    }
    float MeasureTextRHDI(const char *text) {
        return MeasureTextEx(redHatDisplayItalic, text, 48, 1).x;
    }
    float MeasureTextJSN(const char *text, float fontSize) {
        return MeasureTextEx(josefinSansNormal, text, 48, 1).x;
    }
    float MeasureTextJSB(const char *text, float fontSize) {
        return MeasureTextEx(josefinSansBold, text, 48, 1).x;
    }
    float MeasureTextJSI(const char *text, float fontSize) {
        return MeasureTextEx(josefinSansItalic, text, 48, 1).x;
    }
    static Texture2D
    LoadTextureFilter(const std::filesystem::path &texturePath, int &loadedAssets);
    static Model LoadModel_(const std::filesystem::path &modelPath, int &loadedAssets);
    void FirstAssets();
    void LoadAssets();
    void TempAssets(); // For development of the asset rework, this should be removed!
};

#undef NEWTEXASSET
#undef NEWTEXASSET_NOFILTER
#undef NEWFONTASSET
#undef NEWFILEASSET

extern Assets TheAssets;
/// Used for easily loading groups of assets and polling their state as one.
class AssetSet {
    std::vector<Asset *> assets;
public:

    AssetSet(std::initializer_list<Asset *> l) {
        assets = std::vector<Asset *>(l);
    }

    void AddAsset(Asset *asset) {
        assets.push_back(asset);
    }

    void StartLoad() {
        for (int i = 0; i < assets.size(); i++) {
            auto asset = assets[i];
            if (asset->state == UNLOADED) {
                asset->StartLoad();
            }
        }
    }

    bool PollLoaded(bool doFinalize = false) {
        bool loaded = true;
        for (int i = 0; i < assets.size(); i++) {
            auto asset = assets[i];
            if (!asset->CanFetch()) {
                loaded = false;
            }
            if (doFinalize && asset->state == PREFINALIZED) {
                asset->CheckForFetch();
            }
        }
        return loaded;
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
        for (int i = 0; i < assets.size(); i++) {
            // This finalizes any assets that need it
            // We're blocking anyways so why not
            assets[i]->CheckForFetch();
        }
    }
};
extern AssetSet initialSet;
extern AssetSet mainMenuSet;
