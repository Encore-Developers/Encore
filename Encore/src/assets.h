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
    /// Empty constructor provided so vectors can initialize. Do not use!
    Asset() {}

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

    Asset(Asset &&other) noexcept : id(std::move(other.id)),
                                    state(state.load()),
                                    loadingThread() {}

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

    static const std::filesystem::path GetBaseDirectory();

    FileAsset(const std::string &id) : Asset(id) {
        path = GetBaseDirectory() / id;
    }
    FileAsset() {}
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
    FileAsset(FileAsset&& other) noexcept : path(std::move(other.path)),
                                            fileSize(other.fileSize),
                                            fileBuffer(other.fileBuffer) {}
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
    TextureAsset() {}
    Texture2D Fetch() {
        CheckForFetch();
        return tex;
    }
    operator Texture2D() {
        return Fetch();
    }

    TextureAsset(TextureAsset &&other) noexcept : FileAsset(std::move(other)),
                                                  tex(other.tex),
                                                  image(other.image),
                                                  filter(other.filter) {}
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
    std::filesystem::path directory = GetPrevDirectoryPath(GetApplicationDirectory());

public:
    Assets() {}
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
    std::vector<TextureAsset*> YargRings;
    std::vector<TextureAsset*> InstIcons;

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
    NEWFONTASSET(JetBrainsMono, "fonts/JetBrainsMonoNL-Regular.ttf", 64);
    NEWFONTASSET(rubikBoldItalic, "fonts/Rubik-BoldItalic.ttf", 128);

    NEWFONTASSET(rubikBold, "fonts/Rubik-Bold.ttf", 128);

    NEWTEXASSET(discord, "ui/discord-mark-white.png");
    NEWTEXASSET(github, "ui/github-mark-white.png");


    NEWSHADERASSET(sdfShader, "fonts/sdf.fs", "", {});
    NEWSHADERASSET(bgShader, "ui/wavy.fs", "", {"time"});

    void DrawTextRHDI(const char* text, float x, float y, float fontSize, Color color) {
        DrawTextEx(redHatDisplayItalic, text, {x, y}, fontSize, 0, color);
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
