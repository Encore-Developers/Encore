#include "ownedtexture.h"
bool OwnedTexture::allowDispose = true;

OwnedTexture::OwnedTexture(Texture texture) : texture(texture) {
}
OwnedTexture::~OwnedTexture() {
    if (allowDispose) {
        UnloadTexture(texture);
    }
}
Texture OwnedTexture::GetTexture() {
    return texture;
}