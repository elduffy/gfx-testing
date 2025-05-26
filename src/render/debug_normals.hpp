#pragma once
#include <game.hpp>
#include <render/render_object.hpp>

namespace gfx_testing::render {
    class DebugNormals {
    public:
        struct Options {
            float mLineLength = 0.1;
            glm::vec4 mLineColor{0, 1, .72, 1};
        };

        DebugNormals(game::GameContext &gameContext, RenderObject &targetObject, Options const &options);

        void update();

        sdl::SdlContext const &mSdlContext;
        RenderObject mRenderObject;

    private:
        RenderObject const &mTargetObject;
    };
} // namespace gfx_testing::render
