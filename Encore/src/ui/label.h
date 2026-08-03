#pragma once

#include "assets.h"
#include "hb.h"
#include "render/gpudynamicbuffer.h"

namespace UI {
    struct LabelVertex {
        Vector2 position;
        Vector2 normal;
        Vector2 uv;
        unsigned int glyphIndex;
    };

    class Label {
        hb_buffer_t* buffer;
        hb_font_t* hbfont = nullptr;
        std::string text;
        float size = 20.0f;
    public:
        GPUDynamicBuffer<LabelVertex> vertices = {SDL_GPU_BUFFERUSAGE_VERTEX, true};
        GPUDynamicBuffer<uint32_t> indices = {SDL_GPU_BUFFERUSAGE_INDEX, true};
        std::shared_ptr<FontAsset> font;
        bool dirty = false;

        Label();
        Label(const Label& other) = delete;
        void SetFont(hb_face_t* face);

        void BuildMesh(const RenderState& state);
        void Draw(const RenderState& state);

        void SetText(const std::string& newText);

        ~Label();
    };
}

