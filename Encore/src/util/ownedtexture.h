#pragma once
#include "raylib.h"

class OwnedTexture {
    Texture texture;
public:

    OwnedTexture(Texture texture);
    ~OwnedTexture();

    Texture GetTexture();

    operator Texture() {
        return GetTexture();
    }
};
