#pragma once

#include <sdl.hpp>
#include <filesystem>
#include "glm/mat4x4.hpp"

namespace gfx_testing::scene {
    class Scene {
    public:
        Scene(sdl::SdlContext const &context, std::filesystem::path const &projectRoot);

        void Update(sdl::SdlContext const &context);

    private:
        glm::mat4x4 mProjection;
        glm::mat4x4 mView;
        glm::mat4x4 mModel;
        sdl::SdlGfxPipeline mPipeline;
        sdl::SdlGpuBuffer mBuffer;
    };
}
