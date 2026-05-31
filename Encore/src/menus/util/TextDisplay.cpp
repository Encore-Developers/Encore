//
// Created by maria on 31/05/2026.
//

#include "TextDisplay.h"

namespace Encore
{
    TextDisplay & TextDisplay::Fnt(const Font &_font) {
        this->font = _font;
        return *this;
    }
    TextDisplay & TextDisplay::Pos(const Vector2 _pos) {
        this->pos = _pos;
        return *this;
    }
    TextDisplay & TextDisplay::Pos(float x, float y) {
        this->pos = {x,y};
        return *this;
    }
    TextDisplay & TextDisplay::Size(float _fontSize) {
        this->fontSize = _fontSize;
        return *this;
    }
    TextDisplay & TextDisplay::Col(const Color _color) {
        this->color = _color;
        return *this;
    }
    TextDisplay & TextDisplay::Align(const TextAlign _align) {
        this->align = _align;
        return *this;
    }
    TextDisplay & TextDisplay::Bounds(const float _width, const float _height) {
        this->width = _width;
        this->height = _height;
        return *this;
    }
    TextDisplay & TextDisplay::Bounds(const Vector2 _wh) {
        this->width = _wh.x;
        this->height = _wh.y;
        return *this;
    }

    void TextDisplay::lDrawText(const std::string &localizeKey) const {
        DrawText(LOCALISE(localizeKey));
    }

    void TextDisplay::DrawText(const std::string &text) const {
        float textLeftPos = pos.x;
        float textWidth = TextWidth(text); //MeasureTextEx(font, text.c_str(), fontSize, 0).x;
        float textHeight = TextHeight(text);

        // bro
        float size = fontSize;
        float top = pos.y;
        float left = pos.x;
        if (width != 0 && height != 0) {
            if (textWidth > width) {
                size = width / (textWidth / textHeight);
                textWidth = MeasureTextEx(font, text.c_str(), size, 0).x;
                top += (fontSize/2) - (size/2);

            }
            if (align != LEFT) {
                left += width / 2;
            }
        }
        switch (align) {
        case CENTER: {
            textLeftPos = left - (textWidth / 2);
            break;
        }
        case RIGHT: {
            textLeftPos = left - (textWidth);
            break;
        }
        }
        BeginShaderMode(ASSET(sdfShader));
        DrawTextEx(font, text.c_str(), { textLeftPos, top }, size, 0, color);
        EndShaderMode();
    }

    float TextDisplay::lTextWidth(const std::string &localeKey) const {
        return TextWidth(LOCALISE(localeKey));
    }

    float TextDisplay::TextWidth(const std::string &text) const {
        return MeasureTextEx(font, text.c_str(), fontSize, 0).x;
    }

    float TextDisplay::lTextHeight(const std::string &localeKey) const {
        return TextWidth(LOCALISE(localeKey));
    }

    float TextDisplay::TextHeight(const std::string &text) const {
        return MeasureTextEx(font, text.c_str(), fontSize, 0).y;
    }


    namespace Text {
        void DrawText(
            const Font &font,
            const std::string &text,
            Vector2 pos,
            float fontSize,
            Color color,
            TextAlign align
        ) {
            float textLeftPos = pos.x;
            float TextWidth = MeasureTextEx(font, text.c_str(), fontSize, 0).x;

            switch (align) {
            case CENTER: {
                textLeftPos = pos.x - (TextWidth / 2);
                break;
            }
            case RIGHT: {
                textLeftPos = pos.x - (TextWidth);
                break;
            }
            }
            BeginShaderMode(ASSET(sdfShader));
            DrawTextEx(font, text.c_str(), { textLeftPos, pos.y }, fontSize, 0, color);
            EndShaderMode();
        }

        void lDrawText(
            const Font &font, const std::string &localizationKey, Vector2 pos, float fontSize, Color color, TextAlign align) {
            DrawText(font, LOCALISE(localizationKey), pos, fontSize, color, align);
        }

        float TextWidth(const Font &font, const std::string &text, float fontSize) {
            return MeasureTextEx(font, text.c_str(), fontSize, 0).x;
        }
        float lTextWidth(const Font &font, const std::string &text, float fontSize) {
            return TextWidth(font, LOCALISE(text), fontSize);
        }
    }
}
// Encore