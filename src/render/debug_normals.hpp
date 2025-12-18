#pragma once
#include <game.hpp>
#include <render/render_object.hpp>
#include <util/ref.hpp>

namespace gfx_testing::render {
    class DebugNormals {
    public:
        struct Options {
            SHADER_ALIGN glm::vec3 mLineColor{0, 1, .72};
            float mLineLength = 0.1;
        };
        static_assert(sizeof(Options) == 16);

        bool areEnabled() const { return mRenderObject.has_value(); }
        void update();
        void enable(game::GameContext const &gameContext, RenderObject const &targetObject, Options const &options);
        void disable();

        std::optional<RenderObject> mRenderObject;

    private:
        util::cref_opt<RenderObject> mTargetObject;
    };
} // namespace gfx_testing::render
