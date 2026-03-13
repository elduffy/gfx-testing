#include <io/input_manager.hpp>

namespace gfx_testing::io {
    void InputManager::doPivot(glm::vec2 nd) {
        constexpr auto RADS_PER_VIEWPORT = 4.f;
        mScene.getCamera().pivot(RADS_PER_VIEWPORT * glm::vec2{-nd.y, -nd.x});
    }

    void InputManager::doPan(glm::vec2 nd) {
        auto &camera = mScene.getCamera();
        auto const pivotDist = glm::length(camera.getPivot() - camera.getPosition());
        camera.translate(glm::vec2{-nd.x, nd.y} * pivotDist);
    }

    void InputManager::doZoom(float delta) { mScene.getCamera().approach(delta); }

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
                    auto const extent = mScene.getViewportExtent().asVec2();
                    glm::vec2 const nd = {event.motion.xrel / extent.x, event.motion.yrel / extent.y};
                    if (mShift) {
                        doPan(nd);
                    } else {
                        doPivot(nd);
                    }
                }
                break;
            }
            case SDL_EVENT_MOUSE_WHEEL: {
                constexpr auto UNITS_PER_MOUSE_CLICK = 0.5f;
                const float delta = (event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? -1.f : 1.f) *
                                    UNITS_PER_MOUSE_CLICK * event.wheel.y;
                doZoom(delta);
                break;
            }
            case SDL_EVENT_FINGER_DOWN: {
                mFingers[event.tfinger.fingerID] = {event.tfinger.x, event.tfinger.y};
                break;
            }
            case SDL_EVENT_FINGER_UP:
            case SDL_EVENT_FINGER_CANCELED: {
                mFingers.erase(event.tfinger.fingerID);
                break;
            }
            case SDL_EVENT_FINGER_MOTION: {
                if (mFingers.size() == 1) {
                    doPivot({event.tfinger.dx, event.tfinger.dy});
                } else if (mFingers.size() == 2) {
                    // Find the other finger
                    SDL_FingerID otherID{};
                    for (auto const &[id, _]: mFingers) {
                        if (id != event.tfinger.fingerID) {
                            otherID = id;
                            break;
                        }
                    }
                    glm::vec2 const oldPos = mFingers[event.tfinger.fingerID];
                    glm::vec2 const newPos = {event.tfinger.x, event.tfinger.y};
                    glm::vec2 const otherPos = mFingers[otherID];

                    constexpr float ZOOM_SCALE = 10.f;
                    doZoom((glm::length(oldPos - otherPos) - glm::length(newPos - otherPos)) * ZOOM_SCALE);
                    doPan((newPos - oldPos) * 0.5f);
                }
                mFingers[event.tfinger.fingerID] = {event.tfinger.x, event.tfinger.y};
                break;
            }
            default: {
                // SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Unhandled event type: 0x%x", event.type);
                break;
            }
        }
    }
} // namespace gfx_testing::io
