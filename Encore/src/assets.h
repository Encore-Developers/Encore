#pragma once
#include "raylib.h"
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

const char* AssetStateName(AssetState state);

class Asset {
protected:
    virtual void Load() {
    }

    virtual void Finalize() {
    }

    //Asset(Asset& other) {}
public:
    std::atomic<AssetState> state = UNLOADED;
    std::string id;
    std::thread loadingThread;
    /// Used for assets created by another and not stored in TheAssets (ShaderAsset)
    Asset* parent;

    Asset(const std::string &id);

    /// Empty constructor provided so vectors can initialize. Do not use!
    Asset() {
    }

    /// Starts loading this asset.
    virtual void StartLoad();

    /// Starts loading this asset and blocks until it is loaded.
    virtual void LoadImmediate();
    /// Checks if this asset is loaded. Only use in the render thread!
    void CheckForFetch();
    void SetAssetParent(Asset* newParent);

    /// Removes the asset from the internal list, hiding it from the debug Assets window
    void DelistAsset();

    /// Unloads this asset. Only use in render thread!
    virtual void Unload() {
    }

    /// Call when you're polling the asset's for when it's loaded.
    bool CanFetch() const {
        return state == LOADED || state == PREFINALIZED;
    }

    Asset(Asset &&other) noexcept
        : id(std::move(other.id)),
          state(state.load()),
          loadingThread() {
    }

    ~Asset();
};

class FileAsset : public Asset {

protected:
    size_t fileSize;
    void LoadFile();
    virtual void Load();
    void FreeFileBuffer();

public:
    char *fileBuffer;
    bool addNullTerminator = false;

    static const std::filesystem::path GetBaseDirectory();

    FileAsset(const std::string &id)
        : Asset(id) {
    }

    FileAsset() {
    }

    virtual void Unload();

    std::filesystem::path GetPath() {
        return GetBaseDirectory() / id;
    }

    size_t GetFileSize();
    char *FetchRaw();

    operator const unsigned char *() {
        return (const unsigned char *)FetchRaw();
    }

    virtual ~FileAsset() {
        //FreeFileBuffer();
    }

    FileAsset(FileAsset &&other) noexcept
        :
          fileSize(other.fileSize),
          fileBuffer(other.fileBuffer) {
    }
};

class LegacyModelAsset : public Asset {
    Model model;
    std::function<void(Model *)> postFinalizeFunc;

protected:
    virtual void Load() {
        this->state = PREFINALIZED;
    };
    virtual void Finalize();

public:
    LegacyModelAsset(const std::string &id, std::function<void(Model *)> postFinalizeFunc)
        : Asset(id) {
        this->postFinalizeFunc = postFinalizeFunc;
    };

    Model Fetch() {
        CheckForFetch();
        return model;
    };

    virtual void Unload();

    operator Model() {
        return Fetch();
    }
};

class ShaderAsset : public Asset {
    FileAsset *fragmentCode;
    FileAsset *vertexCode;
    std::unordered_map<std::string, int> uniformPositions;
    const char *fStr;
    const char *vStr;
    std::function<void(Shader *)> postFinalizeFunc;
    Shader shader;

protected:
    virtual void Load();
    virtual void Finalize();

public:
    ShaderAsset(const std::string &fsPath,
                const std::string &vsPath,
                std::initializer_list<const char *> uniforms,
                std::function<void(Shader *)> postFinalizeFunc)
        : Asset(fsPath) {
        if (!fsPath.empty()) {
            fragmentCode = new FileAsset(fsPath);
            fragmentCode->addNullTerminator = true;
            fragmentCode->SetAssetParent(this);
        }
        if (!vsPath.empty()) {
            vertexCode = new FileAsset(vsPath);
            vertexCode->addNullTerminator = true;
            vertexCode->SetAssetParent(this);
        }
        this->postFinalizeFunc = postFinalizeFunc;
        for (auto uniform : uniforms) {
            uniformPositions.emplace(uniform, 0);
        }
    }

    void SetUniform(const std::string &uniformName, float value);
    void SetUniform(const std::string &uniformName, Color value);
    void SetUniform(const std::string &uniformName, Vector4 value);
    void SetUniform(const std::string &uniformName, void* value, ShaderUniformDataType type);

    int GetUniformLoc(const std::string &uniformName) {
        if (!uniformPositions.contains(uniformName)) {
            Encore::EncoreLog(LOG_ERROR,
                              TextFormat(
                                  "Attempted to get unknown uniform %s on asset %s",
                                  uniformName.c_str(),
                                  id.c_str()));
            return -1;
        }
        auto found = uniformPositions.find(uniformName);
        return found->second;
    }

    virtual void Unload();

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

    TextureAsset(const std::string &id, bool filter)
        : FileAsset(id) {
        this->filter = filter;
    }

    TextureAsset() {
    }

    virtual void Unload();

    Texture2D Fetch() {
        CheckForFetch();
        return tex;
    }

    operator Texture2D() {
        return Fetch();
    }

    TextureAsset(TextureAsset &&other) noexcept
        : FileAsset(std::move(other)),
          tex(other.tex),
          image(other.image),
          filter(other.filter) {
    }
};

class FontAsset : public FileAsset {
    Font font;
    int fontSize;
    Image atlas;
    bool keepBuffer = false;
    virtual void Load();
    virtual void Finalize();

public:
    FontAsset(const std::string &id, int fontSize)
        : FileAsset(id) {
        this->fontSize = fontSize;
    }

    FontAsset(const std::string &id, int fontSize, bool keepBuffer)
        : FileAsset(id) {
        this->fontSize = fontSize;
        this->keepBuffer = true;
    }

    virtual void Unload();

    Font Fetch() {
        CheckForFetch();
        return font;
    }

    operator Font() {
        return Fetch();
    }

    char* RawData() {
        CheckForFetch();
        return fileBuffer;
    }

    size_t RawDataSize() {
        CheckForFetch();
        return fileSize;
    }
};

#define ASSET(varname) TheAssets.varname
#define ASSETPTR(varname) &TheAssets.varname


#define NEWFILEASSET(varname, path) FileAsset varname = FileAsset(path)
#define NEWFONTASSET(varname, path, size) FontAsset varname = FontAsset(path, size)
#define NEWFONTASSET_KEEPRAW(varname, path, size) FontAsset varname = FontAsset(path, size, true)
#define NEWTEXASSET(varname, path) TextureAsset varname = TextureAsset(path, true)
#define NEWTEXASSET_NOFILTER(varname, path) TextureAsset varname = TextureAsset(path, false)
// Have to use variadic macros here because the preprocessor can't understand that the
// commas in the initializer list are not parameter seperators
#define NEWSHADERASSET(varname, ...) ShaderAsset varname = ShaderAsset(__VA_ARGS__, {})
#define NEWSHADERASSET_POSTFINALIZE(varname, ...) ShaderAsset varname = ShaderAsset(__VA_ARGS__)
#define NEWLEGACYMODELASSET(varname, ...) LegacyModelAsset varname = LegacyModelAsset(__VA_ARGS__)


class Assets {
private:
    std::filesystem::path directory = std::filesystem::path(GetPrevDirectoryPath(GetApplicationDirectory())) / "Assets";

public:
    std::vector<Asset*> assets; // Stored for debugging
    Assets() {
    }

    void AddRingsAndInstruments();

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


    NEWTEXASSET(goldStarUnfilled, "ui/gold-star_unfilled.png");
    NEWTEXASSET(star, "ui/star.png");
    NEWTEXASSET(goldStar, "ui/gold-star.png");
    NEWTEXASSET(emptyStar, "ui/empty-star.png");

    NEWTEXASSET(Scorebox, "gameplay/ui/Scorebox.png");
    NEWTEXASSET(Timerbox, "gameplay/ui/Timerbox.png");
    NEWTEXASSET(TimerboxOutline, "gameplay/ui/TimerboxOutline.png");


    NEWTEXASSET(BaseRingTexture, "ui/hugh ring/rings.png");
    std::vector<TextureAsset *> YargRings;
    std::vector<TextureAsset *> InstIcons;

    Image icon;
    NEWTEXASSET(encoreWhiteLogo, "encore-white.png");
    Texture2D songBackground;

    // Used as a default background for the menu
    NEWTEXASSET(highwayTexture, "gameplay/highway/highway.png");

    NEWFONTASSET(redHatDisplayItalic, "fonts/RedHatDisplay-BlackItalic.ttf", 128);
    NEWFONTASSET(redHatDisplayBlack, "fonts/RedHatDisplay-Black.ttf", 128);
    NEWFONTASSET(redHatDisplayItalicLarge, "fonts/RedHatDisplay-Black.ttf", 128);
    NEWFONTASSET(josefinSansItalic, "fonts/JosefinSans-Italic.ttf", 128);
    NEWFONTASSET(josefinSansNormal, "fonts/JosefinSans-Normal.ttf", 128);
    NEWFONTASSET(josefinSansBold, "fonts/JosefinSans-Bold.ttf", 128);
    NEWFONTASSET(redHatMono, "fonts/RedHatMono-Bold.ttf", 128);
    NEWFONTASSET(rubik, "fonts/Rubik-Regular.ttf", 128);
    NEWFONTASSET(rubikItalic, "fonts/Rubik-Italic.ttf", 128);
    NEWFONTASSET_KEEPRAW(JetBrainsMono, "fonts/JetBrainsMonoNL-Regular.ttf", 64);
    NEWFONTASSET(rubikBoldItalic, "fonts/Rubik-BoldItalic.ttf", 128);

    NEWFONTASSET(rubikBold, "fonts/Rubik-Bold.ttf", 128);

    NEWTEXASSET(discord, "ui/discord-mark-white.png");
    NEWTEXASSET(github, "ui/github-mark-white.png");


    NEWSHADERASSET(sdfShader, "fonts/sdf.fs", "", {});
    NEWSHADERASSET(bgShader, "ui/wavy.fs", "", {"time"});
    NEWSHADERASSET(trackCurveShader,
                   "gameplay/track/fade.fsh",
                   "gameplay/track/trackCurve.vsh",
                   {"trackLength",
                   "fadeSize",
                   "curveFac",
                   "offset",
                   "scale"});

    NEWSHADERASSET_POSTFINALIZE(noteShader,
                   "gameplay/track/notes/noteShader.fsh",
                   "gameplay/track/trackCurve.vsh",
                   {"trackLength",
                   "fadeSize",
                   "maskTexture",
                   "noteColor",
                   "frameColor",
                   "curveFac",
                   "offset",
                   "scale"}, [this](Shader* asset) {
                       asset->locs[SHADER_LOC_MAP_EMISSION] = noteShader.GetUniformLoc("maskTexture");
                   });

    NEWSHADERASSET(highwayScrollShader,
                   "gameplay/track/surface/trackScroll.fsh",
                   "gameplay/track/trackCurve.vsh",
                   {"trackLength",
                   "fadeSize",
                   "time",
                   "curveFac",
                   "offset",
                   "scale"});

    NEWTEXASSET(regularNoteTex, "gameplay/track/notes/normal/diffuse.png");
    NEWTEXASSET(hopoNoteTex, "gameplay/track/notes/hopo/diffuse.png");
    NEWTEXASSET(kickNoteTex, "gameplay/track/notes/kick/diffuse.png");
    NEWTEXASSET(openNoteTex, "gameplay/track/notes/open/diffuse.png");

    NEWTEXASSET(regularMaskTex, "gameplay/track/notes/normal/color_mask.png");
    NEWTEXASSET(hopoMaskTex, "gameplay/track/notes/hopo/color_mask.png");
    NEWTEXASSET(kickMaskTex, "gameplay/track/notes/kick/color_mask.png");
    NEWTEXASSET(openMaskTex, "gameplay/track/notes/open/color_mask.png");



    NEWLEGACYMODELASSET(regularNote, "gameplay/track/notes/normal/model.obj", [this](Model* model) {
        SetTextureWrap(regularNoteTex, TEXTURE_WRAP_CLAMP);
        SetTextureWrap(regularMaskTex, TEXTURE_WRAP_CLAMP);
        model->materials[0].maps[0].texture = regularNoteTex;
        // const Texture2D mask = regularMaskTex.Fetch();

        model->materials[0].shader = noteShader;
        model->materials[0].maps[MATERIAL_MAP_EMISSION].texture = regularMaskTex;
    });

    NEWLEGACYMODELASSET(hopoNote, "gameplay/track/notes/hopo/model.obj", [this](Model* model) {
        SetTextureWrap(hopoNoteTex, TEXTURE_WRAP_CLAMP);
        SetTextureWrap(hopoMaskTex, TEXTURE_WRAP_CLAMP);
        model->materials[0].maps[0].texture = hopoNoteTex;
        // const Texture2D mask = hopoMaskTex.Fetch();

        model->materials[0].shader = noteShader;
        model->materials[0].maps[MATERIAL_MAP_EMISSION].texture = hopoMaskTex;
    });

    NEWLEGACYMODELASSET(openNote, "gameplay/track/notes/open/model.obj", [this](Model* model) {
        SetTextureWrap(openNoteTex, TEXTURE_WRAP_CLAMP);
        SetTextureWrap(openMaskTex, TEXTURE_WRAP_CLAMP);
        model->materials[0].maps[0].texture = openNoteTex;
        // const Texture2D mask = hopoMaskTex.Fetch();

        model->materials[0].shader = noteShader;
        model->materials[0].maps[MATERIAL_MAP_EMISSION].texture = openMaskTex;
    });

    NEWLEGACYMODELASSET(kickNote, "gameplay/track/notes/kick/model.obj", [this](Model* model) {
        SetTextureWrap(kickNoteTex, TEXTURE_WRAP_CLAMP);
        SetTextureWrap(kickMaskTex, TEXTURE_WRAP_CLAMP);
        model->materials[0].maps[0].texture = kickNoteTex;
        // const Texture2D mask = hopoMaskTex.Fetch();

        model->materials[0].shader = noteShader;
        model->materials[0].maps[MATERIAL_MAP_EMISSION].texture = kickMaskTex;
    });


    NEWTEXASSET(smasherOffTex, "gameplay/track/smashers/normal/piston_off.png");
    NEWTEXASSET(smasherOnTex, "gameplay/track/smashers/normal/piston_on.png");
    NEWTEXASSET(smasherFrameTex, "gameplay/track/smashers/normal/frame.png");
    NEWTEXASSET(kickFrameTex, "gameplay/track/smashers/kick/frame.png");
    NEWTEXASSET(trackRailsTex, "gameplay/track/surface/rails.png");

    NEWLEGACYMODELASSET(smasherPiston, "gameplay/track/smashers/normal/piston.obj",
        [this](Model* model) {
            SetTextureWrap(smasherOffTex, TEXTURE_WRAP_CLAMP);
            SetTextureWrap(smasherOnTex, TEXTURE_WRAP_CLAMP);
            model->materials[0].maps[0].texture = smasherOffTex;
            model->materials[0].shader = trackCurveShader;
        });

    NEWLEGACYMODELASSET(smasherFrame, "gameplay/track/smashers/normal/frame.obj",
        [this](Model* model) {
            SetTextureWrap(smasherFrameTex, TEXTURE_WRAP_CLAMP);
            model->materials[0].maps[0].texture = smasherFrameTex;
            model->materials[0].shader = trackCurveShader;
        });

    NEWLEGACYMODELASSET(trackSurface, "gameplay/track/surface/track.obj",
        [this](Model* model) {
            SetTextureWrap(smasherFrameTex, TEXTURE_WRAP_CLAMP);
            model->materials[0].maps[0].texture = highwayTexture;
            model->materials[0].shader = highwayScrollShader;
        });

    NEWLEGACYMODELASSET(rails, "gameplay/track/surface/rails.obj",
        [this](Model* model) {
            SetTextureWrap(trackRailsTex, TEXTURE_WRAP_CLAMP);
            model->materials[0].maps[0].texture = trackRailsTex;
            model->materials[0].shader = trackCurveShader;
        });

    NEWLEGACYMODELASSET(kickFrame, "gameplay/track/smashers/kick/frame.obj",
        [this](Model* model) {
            SetTextureWrap(kickFrameTex, TEXTURE_WRAP_CLAMP);
            model->materials[0].maps[0].texture = kickFrameTex;
            model->materials[0].shader = trackCurveShader;
        });

    NEWLEGACYMODELASSET(kickPiston, "gameplay/track/smashers/kick/piston.obj",
        [this](Model* model) {
            model->materials[0].shader = trackCurveShader;
        });

    NEWTEXASSET(hitFlareTex, "gameplay/track/particles/hit_flare.png");
    NEWTEXASSET(hitFlareInnerTex, "gameplay/track/particles/hit_flare_inner.png");
    NEWTEXASSET(shockwaveTex, "gameplay/track/particles/shockwave.png");

    void DrawTextRHDI(const char *text, float x, float y, float fontSize, Color color) {
        DrawTextEx(redHatDisplayItalic, text, { x, y }, fontSize, 0, color);
        BeginShaderMode(sdfShader);
        EndShaderMode();
    }

    float MeasureTextRubik(const char *text, float fontSize) {
        return MeasureTextEx(rubik, text, fontSize, 1).x;
    }
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
        while (!PollLoaded()) {
        }
        for (int i = 0; i < assets.size(); i++) {
            // This finalizes any assets that need it
            // We're blocking anyways so why not
            assets[i]->CheckForFetch();
        }
    }
};

extern AssetSet initialSet;
extern AssetSet mainMenuSet;
