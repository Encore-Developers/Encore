//
// Created by maria on 28/01/2025.
//

#ifndef RAYLIB_3D_TEXT_H
#define RAYLIB_3D_TEXT_H



// code from raylib examples lol
void DrawTextCodepoint3D(
    Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint
);

void DrawText3D(
    Font font,
    const char *text,
    Vector3 position,
    float fontSize,
    float fontSpacing,
    float lineSpacing,
    bool backface,
    Color tint
);

Vector3 MeasureText3D(
    Font font, const char *text, float fontSize, float fontSpacing, float lineSpacing
);
#endif //RAYLIB_3D_TEXT_H
