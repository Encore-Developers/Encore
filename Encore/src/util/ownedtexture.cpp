#include "ownedtexture.h"
OwnedTexture::OwnedTexture(Texture texture) : texture(texture) {
}
OwnedTexture::~OwnedTexture() {
    UnloadTexture(texture);
}
Texture OwnedTexture::GetTexture() {
    return texture;
}