#include <io/input_manager.hpp>

namespace gfx_testing::io {
    void InputManager::handleEvent(SDL_Event const &event) {
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
            case SDL_EVENT_KEY_DOWN: {
                if (event.key.key == SDLK_LSHIFT) {
                    mShift = true;
                }
                break;
            }
            case SDL_EVENT_KEY_UP: {
                if (!event.key.down && !event.key.repeat && event.key.key == SDLK_RETURN) {
                    mGameContext.mStopwatch.toggle();
                }
                if (!event.key.down && !event.key.repeat && event.key.key == SDLK_D) {
                    mImGuiContext.toggleOpen();
                }
                if (!event.key.down && !event.key.repeat && event.key.key == SDLK_LSHIFT) {
                    mShift = false;
                }
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                if ((event.motion.state & SDL_BUTTON_MMASK) != 0) {

                    auto &camera = mScene.getCamera();
                    auto const extent = mScene.getViewportExtent().asVec2();
                    if (mShift) {
                        glm::vec2 const dir = {-event.motion.xrel / extent.x, event.motion.yrel / extent.y};
                        auto const pivotDist = glm::length(camera.getPivot() - camera.getPosition());
                        camera.translate(dir * pivotDist);
                    } else {
                        constexpr auto RADS_PER_VIEWPORT = 4.f;
                        glm::vec2 const radians = RADS_PER_VIEWPORT * glm::vec2{-event.motion.yrel / extent.y,
                                                                                -event.motion.xrel / extent.x};
                        camera.pivot(radians);
                    }
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
