//
// Created by maria on 31/05/2026.
//

#ifndef ENCORE_TEXTDISPLAY_H
#define ENCORE_TEXTDISPLAY_H
#include "assets.h"
#include "menus/main/MainMenu.h"

enum TextAlign {
    LEFT,
    CENTER,
    RIGHT
};

namespace Encore
{
    struct TextDisplay {
        Font font = ASSET(rubik);
        Vector2 pos {0,0};
        float fontSize = 0;
        Color color{255, 255, 255, 255};
        TextAlign align = LEFT;
        float width = 0;
        float height = 0;
        Vector2 padding {0,0};

        TextDisplay& Fnt(const Font &_font);
        TextDisplay& Pos(Vector2 _pos);
        TextDisplay& Pos(float x, float y);
        TextDisplay& AddPos(float x, float y);
        TextDisplay& AddPos(Vector2 _pos);
        TextDisplay& AddX(float x);
        TextDisplay& AddY(float y);
        TextDisplay& Size(float _fontSize);
        TextDisplay& Col(Color _color);
        TextDisplay& Align(TextAlign _align);
        TextDisplay& Bounds(Vector2 _wh);
        TextDisplay& Bounds(float _width, float _height);
        TextDisplay& Padding(float x, float y);
        TextDisplay& Padding(Vector2 _padding);

        void lDrawText(const std::string &localizeKey) const;
        void DrawText(const std::string &text) const;

        float lTextWidth(const std::string &localeKey) const;
        float TextWidth(const std::string &text) const;
        float lTextHeight(const std::string &localeKey) const;
        float TextHeight(const std::string &text) const;
    };

    namespace Text {
        void DrawText(
            const Font &font,
            const std::string &text,
            Vector2 pos,
            float fontSize,
            Color color,
            TextAlign align
        );
        void lDrawText(
            const Font &font,
            const std::string &localizationKey,
            Vector2 pos,
            float fontSize,
            Color color,
            TextAlign align
        );

        // like i mean if you really wanna
        float TextWidth(const Font &font, const std::string &text, float fontSize);
        float lTextWidth(const Font &font, const std::string &localeKey, float fontSize);
    }
} // Encore

#endif //ENCORE_TEXTDISPLAY_H