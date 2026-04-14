#pragma once
#include "SDL3/SDL_gpu.h"
#include "math/glm.h"

namespace UI {

    struct BoxVertex {
        Vector2 position;
        Vector2 uv;
        Vector2 corner;
    };

    class BoxMesh {
    public:
        SDL_GPUBuffer* vertexBuffer = nullptr;
        int vertexCount = 0;

        SDL_GPUBufferBinding GetBinding() {
            return {vertexBuffer, 0};
        }
    };

    enum CornerType {
        SQUIRCLE,
        CIRCULAR,
        ANGULAR
    };

    enum Corner {
        TOP_LEFT = 0,
        TOP_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_RIGHT
    };

    enum Side {
        LEFT = 0,
        RIGHT,
        TOP,
        BOTTOM
    };

    struct BoxVertexUniform {
        Vector2 boxPos;
        Vector2 boxSize;
        Vector4 cornerSizes;
    };

    class BoxColor {
    public:
        Color top;
        Color bottom;

        BoxColor(Color col) : top(col), bottom(col) {}
        BoxColor(Color top, Color bottom) : top(top), bottom(bottom) {}
    };

    struct BoxFragmentUniform {
        BoxColor color;
    };

    struct BoxBorder {
        float sizes[4];
        float cornerShrink;
        BoxColor color;
    };

    class Box {
        /// Mesh with smoothed rounded corners (Apple-style)
        static BoxMesh squircleMesh;
        /// Mesh with typical circular rounded corners
        static BoxMesh circularMesh;
        /// Mesh with straight angular corners.
        static BoxMesh angularMesh;
        static bool meshesInitialized;
        static void GenerateRoundedMesh(SDL_GPUCopyPass* copyPass, BoxMesh* mesh, int detail, float r = 2/2.5);
        static void PushUniforms(SDL_GPUCommandBuffer* cmdBuf, BoxVertexUniform vertexUniform, BoxFragmentUniform fragmentUniform);
    public:
        static void InitializeMeshes(SDL_GPUCopyPass* copyPass);

        BoxColor color;
        CornerType cornerType;
        Vector4 cornerSizes;
        std::vector<BoxBorder> borders;


        Box(BoxColor color) : color(color), cornerType(SQUIRCLE), cornerSizes{0} {}

        BoxMesh* GetMesh() const;
        void Draw(SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmdBuf, Vector2 pos, Vector2 size);
    };
}