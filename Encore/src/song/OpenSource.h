#pragma once
#include "assets.h"
#include "raylib.h"

#include <filesystem>
#include <vector>
#include <string>

class SourceIcon {
    Texture texture;
public:
    std::string iconId;
    std::string name;

    Texture GetTexture();
    operator Texture() {
        return GetTexture();
    }
};

class OpenSource {
public:

    std::unordered_map<std::string, SourceIcon*> idToIcon;
    std::unordered_map<std::string, std::filesystem::path> iconIds;

    void InitIcons(const std::filesystem::path& dir);
    SourceIcon* GetIcon(const std::string& sourceId);
    const std::string& GetSourceName(const std::string& sourceId);

    SourceIcon* operator [](const std::string& sourceId) {
        return GetIcon(sourceId);
    }
};

extern OpenSource TheSourceIcons;