#pragma once
#include <game.hpp>
#include <render/render_object.hpp>

namespace gfx_testing::render {
    class DebugNormals {
    public:
        struct Options {
            SHADER_ALIGN glm::vec3 mLineColor{0, 1, .72};
            float mLineLength = 0.1;
        };
        static_assert(sizeof(Options) == 16);

        DebugNormals(game::GameContext const &gameContext, RenderObject &targetObject, bool enabled,
                     Options const &options);

        void update();

        sdl::SdlContext const &mSdlContext;
        std::optional<RenderObject> mRenderObject;

    private:
        RenderObject const &mTargetObject;
    };
} // namespace gfx_testing::render
