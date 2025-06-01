#include <io/input_manager.hpp>

namespace gfx_testing::io {
    void InputManager::handleEvent(SDL_Event const &event) const {
        if (mImGuiContext.processEvent(event)) {
            return;
        }
        switch (event.type) {
            case SDL_EVENT_WINDOW_RESIZED: {
                mScene.onResize({
                        boost::safe_numerics::checked::cast<uint32_t>(event.window.data1),
                        boost::safe_numerics::checked::cast<uint32_t>(event.window.data2),
                });
                break;
            }
            case SDL_EVENT_KEY_UP: {
                if (!event.key.down && !event.key.repeat && event.key.key == SDLK_RETURN) {
                    mGameContext.mStopwatch.toggle();
                }
                if (!event.key.down && !event.key.repeat && event.key.key == SDLK_D) {
                    mImGuiContext.toggleOpen();
                }
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                if ((event.motion.state & SDL_BUTTON_MMASK) != 0) {
                    constexpr auto RADS_PER_VIEWPORT_DIMENSIONS = 4.f;
                    auto const extent = mScene.getViewportExtent().asVec2() / RADS_PER_VIEWPORT_DIMENSIONS;
                    glm::vec2 const &radians = {-event.motion.yrel / extent.y, -event.motion.xrel / extent.x};
                    mScene.getCamera().pivot(radians);
                }
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: {
                constexpr auto UNITS_PER_MOUSE_CLICK = 0.5f;
                const float deltaRadius = (event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? -1.f : 1.f) *
                                          UNITS_PER_MOUSE_CLICK * event.wheel.y;
                mScene.getCamera().approach(deltaRadius);
                break;
            }
            default: {
                // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
                break;
            }
        }
    }
} // namespace gfx_testing::io
