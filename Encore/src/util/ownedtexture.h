#pragma once
#include "raylib.h"

class OwnedTexture {
    Texture texture;
public:

    static bool allowDispose;

    OwnedTexture(Texture texture);
    ~OwnedTexture();

    Texture GetTexture();

    operator Texture() {
        return GetTexture();
    }
};
