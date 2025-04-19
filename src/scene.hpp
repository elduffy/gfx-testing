#pragma once

#include <sdl.hpp>
#include <filesystem>
#include <shader_models.hpp>
#include <glm/mat4x4.hpp>

namespace gfx_testing::scene {
    class Scene {
    public:
        Scene(sdl::SdlContext const &context, std::filesystem::path const &projectRoot);

        void draw(sdl::SdlContext const &context) const;

    private:
        glm::mat4x4 mProjection;
        glm::mat4x4 mView;
        glm::mat4x4 mModel;
        shader::MeshData mMeshData; // TODO we don't need this to match the lifetime of the Scene
        sdl::SdlGfxPipeline mPipeline;
        sdl::SdlGpuBuffer mVertexBuffer;
        sdl::SdlGpuBuffer mIndexBuffer;
    };
}
