//
// Created by marie on 02/05/2024.
//

#include "assets.h"
#include <filesystem>
#include "raygui.h"
#include "util/enclog.h"

#include <fstream>
#include <iostream>

class Assets;
class Asset;

Assets TheAssets; // This is the single instance

const char *AssetStateName(AssetState state) {
    switch (state) {
    case UNLOADED:
        return "UNLOADED";
    case LOADING:
        return "LOADING";
    case PREFINALIZED:
        return "PREFINALIZED";
    case LOADED:
        return "LOADED";
    }
    return "INVALID";
}

Assets &Assets::getInstance() {
    return TheAssets;
}

void Asset::CheckForFetch() {
    switch (state) {
    case UNLOADED:
        Load();
        Encore::EncoreLog(LOG_WARNING,
                          TextFormat(
                              "Asset %s was fetched before it was loaded. Loading immediately on main thread...",
                              id.c_str()));
        if (state == PREFINALIZED) {
            // TODO: Don't duplicate this logic
            Encore::EncoreLog(LOG_INFO, TextFormat("Finalizing asset %s...", id.c_str()));
            Finalize();
        }
        break;
    case LOADING:
        Encore::EncoreLog(LOG_WARNING,
                          TextFormat(
                              "Asset %s was fetched while it is being loaded. Blocking until it is loaded...",
                              id.c_str()));
        while (state == LOADING) {
        } // spin spin spin
        if (state == PREFINALIZED) {
            Encore::EncoreLog(LOG_INFO, TextFormat("Finalizing asset %s...", id.c_str()));
            Finalize();
        }
        break;
    case PREFINALIZED:
        Encore::EncoreLog(LOG_INFO, TextFormat("Finalizing asset %s...", id.c_str()));
        Finalize();
        break;
    default: ;
    }
}
Asset::~Asset() {
    for (auto iter = TheAssets.assets.begin(); iter != TheAssets.assets.end(); ++iter) {
        auto asset = *iter;
        if (asset == this) {
            TheAssets.assets.erase(iter);
            break;
        }
    }

}
Asset::Asset(const std::string &id) {
    this->id = id;
    TheAssets.assets.push_back(this);
}
void Asset::StartLoad() {
    if (state == UNLOADED) {
        state = LOADING;
        loadingThread = std::thread([this]() { this->Load(); });
        loadingThread.detach();
        //Encore::EncoreLog(LOG_INFO, TextFormat("Loading asset %s...", id.c_str()));
    }
}

void Asset::LoadImmediate() {
    Encore::EncoreLog(LOG_INFO,
                      TextFormat("Loading asset %s immediately...", id.c_str()));
    StartLoad();
    while (state == LOADING) {
    }
}

void FileAsset::LoadFile() {
    std::ifstream file(GetPath(), std::ios::binary | std::ios::ate);
    fileSize = file.tellg();
    int realFileSize = fileSize;
    if (addNullTerminator)
        fileSize++;
    file.seekg(0, std::ios::beg);

    fileBuffer = (char *)malloc(fileSize);
    file.read(fileBuffer, realFileSize);
    file.close();
    if (addNullTerminator)
        fileBuffer[fileSize - 1] = '\0';
}

void FileAsset::FreeFileBuffer() {
    if (fileBuffer != nullptr) {
        free(fileBuffer);
    }
}

const std::filesystem::path FileAsset::GetBaseDirectory() {
    return TheAssets.getDirectory();
}

void FileAsset::Load() {
    LoadFile();
    state = LOADED;
}

void FileAsset::Unload() {
    FreeFileBuffer();
    state = UNLOADED;
}

size_t FileAsset::GetFileSize() {
    CheckForFetch();
    return fileSize;
}

char *FileAsset::FetchRaw() {
    CheckForFetch();
    return fileBuffer;
}

void LegacyModelAsset::Finalize() {
    model = LoadModel((TheAssets.getDirectory() / id).c_str());
    postFinalizeFunc(&model);
    state = LOADED;
}

void LegacyModelAsset::Unload() {
    UnloadModel(model);
    state = UNLOADED;
}

void ShaderAsset::Load() {
    AssetSet code = {};
    if (fragmentCode)
        code.AddAsset(fragmentCode);
    if (vertexCode)
        code.AddAsset(vertexCode);
    code.StartLoad();
    code.BlockUntilLoaded();
    state = PREFINALIZED;
}

void ShaderAsset::Finalize() {
    const char *fragString = fragmentCode ? fragmentCode->FetchRaw() : nullptr;
    const char *vertString = vertexCode ? vertexCode->FetchRaw() : nullptr;
    shader = LoadShaderFromMemory(vertString, fragString);
    for (auto &uniform : uniformPositions) {
        uniform.second = GetShaderLocation(shader, uniform.first.c_str());
    }
    if (postFinalizeFunc) postFinalizeFunc(&shader);
    state = LOADED;
    // These destructors probably don't free the file buffers. Sad!
    delete fragmentCode;
    delete vertexCode;

}
void ShaderAsset::SetUniform(const std::string &uniformName, float value) {
    SetUniform(uniformName, &value, SHADER_UNIFORM_FLOAT);
}

void ShaderAsset::SetUniform(const std::string &uniformName, Color value) {
    Vector4 vec4 = {
        value.r / 255.0f, value.g / 255.0f, value.b / 255.0f, value.a / 255.0f
    };
    SetUniform(uniformName, &vec4, SHADER_UNIFORM_VEC4);
}
void ShaderAsset::SetUniform(
    const std::string &uniformName, void *value, ShaderUniformDataType type
) {
    CheckForFetch();
    SetShaderValue(shader, GetUniformLoc(uniformName), value, type);
}

void ShaderAsset::Unload() {
    if (fragmentCode) {
        fragmentCode->Unload();
    }
    if (vertexCode) {
        vertexCode->Unload();
    }
    UnloadShader(shader);
    state = UNLOADED;
}

void TextureAsset::Load() {
    LoadFile();
    image = LoadImageFromMemory(
        reinterpret_cast<const char *>(GetPath().extension().generic_u8string().c_str()),
        (const unsigned char *)fileBuffer,
        fileSize);
    width = image.width;
    height = image.height;
    FreeFileBuffer();
    state = PREFINALIZED;
}

void TextureAsset::Finalize() {
    tex = LoadTextureFromImage(image);
    if (filter) {
        GenTextureMipmaps(&tex);
        SetTextureFilter(tex, TEXTURE_FILTER_TRILINEAR);
    }
    UnloadImage(image);
    state = LOADED;
}

void TextureAsset::Unload() {
    UnloadTexture(tex);
    FileAsset::Unload();
}

void FontAsset::Load() {
    auto start = std::chrono::high_resolution_clock::now();
    LoadFile();
    font = {};
    font.baseSize = fontSize;
    font.glyphCount = 250;
    font.glyphPadding = 4;
    font.glyphs = LoadFontData((const unsigned char *)fileBuffer,
                               fileSize,
                               fontSize,
                               nullptr,
                               250,
                               FONT_SDF);
    atlas = GenImageFontAtlas(font.glyphs, &font.recs, 250, fontSize, 4, 0);
    for (int i = 0; i < font.glyphCount; i++) {
        UnloadImage(font.glyphs[i].image);
        font.glyphs[i].image = ImageFromImage(atlas, font.recs[i]);
    }
    if (!keepBuffer) {
        FreeFileBuffer();
    }
    auto end = std::chrono::high_resolution_clock::now();
    Encore::EncoreLog(LOG_INFO,
                      TextFormat("Generated font data for %s in %i microseconds.",
                                 id.c_str(),
                                 (std::chrono::duration_cast<std::chrono::microseconds>(
                                     end - start).count())));
    state = PREFINALIZED;
}

void FontAsset::Finalize() {
    font.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
    state = LOADED;
}

void FontAsset::Unload() {
    UnloadFont(font);
    FileAsset::Unload();
}


// Assets that need to be loaded in order to display the cache loading screen and the
// title screen. Adding assets to this set will affect boot times (especially fonts)
AssetSet initialSet = { ASSETPTR(encoreWhiteLogo),
                        ASSETPTR(rubik),
                        ASSETPTR(favicon),
                        ASSETPTR(faviconTex),
                        ASSETPTR(redHatDisplayBlack),
                        ASSETPTR(sdfShader),
                        ASSETPTR(josefinSansItalic),
                        ASSETPTR(bgShader),
                        ASSETPTR(JetBrainsMono),
                        ASSETPTR(rubikBold) };
// Assets that are queued to load at boot but aren't critical for displaying the title
// screen. Adding assets to this set results in a smaller impact on boot times
AssetSet mainMenuSet = { ASSETPTR(redHatDisplayItalic),
                         ASSETPTR(BaseRingTexture),
                         ASSETPTR(rubikBoldItalic),
                         ASSETPTR(rubikItalic),
                         ASSETPTR(discord),
                         ASSETPTR(github),
                         ASSETPTR(emptyStar),
                         ASSETPTR(star),
                         ASSETPTR(goldStar),
                         ASSETPTR(goldStarUnfilled),
                         ASSETPTR(redHatMono),
                         ASSETPTR(Timerbox),
                         ASSETPTR(TimerboxOutline),
                         ASSETPTR(Scorebox),
                         ASSETPTR(highwayTexture),
                         ASSETPTR(regularNoteTex),
                         ASSETPTR(regularNote),
                         ASSETPTR(hopoNoteTex),
                         ASSETPTR(hopoNote),
                         ASSETPTR(hopoMaskTex),
                         ASSETPTR(regularMaskTex)
};


void Assets::AddRingsAndInstruments() {
    for (int i = 1; i <= 6; i++) {
        TextureAsset *tex = new TextureAsset(TextFormat("ui/hugh ring/rings-%i.png", i),
                                             true);
        YargRings.push_back(tex);
        // Grabbing from the vec because it moved
        mainMenuSet.AddAsset(tex);
    }
    InstIcons.push_back(new TextureAsset("ui/hugh ring/drums-inv.png", true));
    InstIcons.push_back(new TextureAsset("ui/hugh ring/bass-inv.png", true));
    InstIcons.push_back(new TextureAsset("ui/hugh ring/lead-inv.png", true));
    InstIcons.push_back(new TextureAsset("ui/hugh ring/keys-inv.png", true));
    InstIcons.push_back(new TextureAsset("ui/hugh ring/vox-inv.png", true));
    for (auto icon : InstIcons) {
        mainMenuSet.AddAsset(icon);
    }
}
