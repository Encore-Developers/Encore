#include "box.h"
#include "graphicsState.h"
#include "render/pipelines.h"

using namespace UI;

BoxMesh Box::squircleMesh = {};
BoxMesh Box::circularMesh = {};
BoxMesh Box::angularMesh = {};
bool Box::meshesInitialized = false;
void Box::InitializeMeshes(SDL_GPUCopyPass *copyPass) {
    if (meshesInitialized) {
        return;
    }
    GenerateRoundedMesh(copyPass, &squircleMesh, 16);
    GenerateRoundedMesh(copyPass, &circularMesh, 16, 1);
    GenerateRoundedMesh(copyPass, &angularMesh, 1, 1);
    meshesInitialized = true;
}
/// Generates a mesh used by all rounded rectangles so that the curve doesn't need to be
/// calculated in the vertex shader
void Box::GenerateRoundedMesh(SDL_GPUCopyPass *copyPass, BoxMesh* mesh, int detail, float r) {

    if (mesh->vertexBuffer) {
        SDL_ReleaseGPUBuffer(TheGPU, mesh->vertexBuffer);
        mesh->vertexBuffer = nullptr;
    }

    // Each corner has detail+1 vertices, so that there's a vertex at θ=0 and θ=π/2
    int vertexCount = (detail+1)*4;

    SDL_GPUTransferBufferCreateInfo createInfo = {
        SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        (unsigned int)(sizeof(BoxVertex) * vertexCount * 4)
    };
    auto transferBuffer = SDL_CreateGPUTransferBuffer(TheGPU, &createInfo);

    std::vector<Vector2> curvePoints;
    curvePoints.reserve((detail+1)*2);

    for (int i = 0; i <= detail; i++) {
        float t = ((float)i/(float)detail) * glm::half_pi<float>();

        float x = glm::pow(glm::clamp(glm::cos(t), 0.0f, 1.0f), r);
        float y = glm::pow(glm::clamp(glm::sin(t), 0.0f, 1.0f), r);
        Vector2 base(x, y);
        curvePoints.push_back(Vector2(1, 1) - base);
    }

    // Mirror points over the x axis
    for (int i = detail; i >= 0; i--) {
        auto flip = curvePoints[i];
        flip.x = 1 - flip.x;
        curvePoints.push_back(flip);
    }

    std::vector<BoxVertex> vertices;
    vertices.reserve(vertexCount);


    // TODO: fix these uvs

    vertices.push_back({curvePoints[0], curvePoints[0], Vector2(0.0f, 0.0f)});
    for (int i = 0; i < curvePoints.size()-1; i++) {
        auto flip = curvePoints[i];
        flip.y = 1 - flip.y;
        vertices.push_back({flip, flip, Vector2(i<=detail ? 0 : 1, 1)});
        vertices.push_back({curvePoints[i+1], curvePoints[i+1], Vector2((i+1)<=detail ? 0 : 1, 0)});
    }
    auto flip = curvePoints[curvePoints.size()-1];
    flip.y = 1 - flip.y;
    vertices.push_back({flip, flip, Vector2(1.0f, 1.0f)});

    assert(vertices.size() == vertexCount);

    auto data = SDL_MapGPUTransferBuffer(TheGPU, transferBuffer, false);
    memcpy(data, vertices.data(), vertices.size()*sizeof(BoxVertex));
    SDL_UnmapGPUTransferBuffer(TheGPU, transferBuffer);

    SDL_GPUBufferCreateInfo vertCreateInfo = {
        SDL_GPU_BUFFERUSAGE_VERTEX,
        (unsigned int)(vertices.size()*sizeof(BoxVertex)),
        0
    };
    mesh->vertexBuffer = SDL_CreateGPUBuffer(TheGPU, &vertCreateInfo);
    mesh->vertexCount = vertices.size();

    SDL_GPUTransferBufferLocation srcLoc = {
        transferBuffer,
        0
    };
    SDL_GPUBufferRegion destLoc = {
        mesh->vertexBuffer,
        0,
        (unsigned int)(vertices.size()*sizeof(BoxVertex))
    };
    SDL_UploadToGPUBuffer(copyPass, &srcLoc, &destLoc, false);
    SDL_ReleaseGPUTransferBuffer(TheGPU, transferBuffer);
}

void Box::PushUniforms(
    SDL_GPUCommandBuffer *cmdBuf,
    BoxVertexUniform vertexUniform,
    BoxFragmentUniform fragmentUniform
) {
    SDL_PushGPUVertexUniformData(cmdBuf, 1, &vertexUniform, sizeof(vertexUniform));
    SDL_PushGPUFragmentUniformData(cmdBuf, 0, &fragmentUniform, sizeof(fragmentUniform));
}

BoxMesh *Box::GetMesh() const {
    switch (cornerType) {
    case SQUIRCLE:
        return &squircleMesh;
    case CIRCULAR:
        return &circularMesh;
    case ANGULAR:
        return &angularMesh;
    }
    return &squircleMesh;
}
void Box::Draw(SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* cmdBuf, Vector2 pos, Vector2 size) {
    SDL_BindGPUGraphicsPipeline(renderPass, PIPELINE(boxPipeline));
    BoxMesh *mesh = GetMesh();
    auto binding = mesh->GetBinding();
    SDL_BindGPUVertexBuffers(renderPass, 0, &binding, 1);
    auto currentCornerSizes = cornerSizes;
    for (auto &border : borders) {

        PushUniforms(cmdBuf, {pos, size, currentCornerSizes}, {border.color});

        currentCornerSizes -= border.cornerShrink;
        currentCornerSizes = glm::max(currentCornerSizes, 0.0f);
        pos.x += border.sizes[LEFT];
        size.x -= border.sizes[LEFT];
        size.x -= border.sizes[RIGHT];

        pos.y += border.sizes[TOP];
        size.y -= border.sizes[TOP];
        size.y -= border.sizes[BOTTOM];

        SDL_DrawGPUPrimitives(renderPass, mesh->vertexCount, 1, 0, 0);
    }
    PushUniforms(cmdBuf, {pos, size, currentCornerSizes}, {color});
    SDL_DrawGPUPrimitives(renderPass, mesh->vertexCount, 1, 0, 0);
}