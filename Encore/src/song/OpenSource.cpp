#include "OpenSource.h"

#include <fstream>
#include <nlohmann/json.hpp>
#include "assets.h"

using json = nlohmann::json;

Texture SourceIcon::GetTexture() {
    if (texture.id == 0) {
        texture = LoadTexture(TheSourceIcons.iconIds[iconId].c_str());
        GenTextureMipmaps(&texture);
        SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
    }
    return texture;
}
void OpenSource::InitIcons(const std::filesystem::path& dir) {
    std::ifstream f(dir / "index.json");
    json index = nlohmann::json::parse(f);
    for (auto& [key, value] : index["sources"].items()) {
        auto* icon = new SourceIcon();
        icon->iconId = value["icon"].get<std::string>();
        icon->name = value["names"]["en-US"].get<std::string>();
        for (auto& [key, value] : value["ids"].items()) {
            idToIcon[value.get<std::string>()] = icon;
        }
    }
    for (const auto &entry : std::filesystem::directory_iterator(dir/ "icons")) {
        iconIds[entry.path().stem()] = entry.path();
    };
}
SourceIcon *OpenSource::GetIcon(const std::string& sourceId) {
    if (!idToIcon.contains(sourceId)) {
        return GetIcon("$DEFAULT$");
    }
    return idToIcon[sourceId];
}