#pragma once
#include <clipp.h>
#include <filesystem>
#include <game.hpp>
#include <iostream>
#include <toml.hpp>

namespace gfx_testing {
    class Cli {
        static constexpr float DEFAULT_TARGET_FPS = 120.f;
        static constexpr bool LOG_TOML = false;
        using toml_doc = toml::basic_value<toml::type_config>;

        static std::optional<float> findOptionalFloatLenient(toml_doc const &parsed, char const *name,
                                                             float defaultValue) {
            if (!parsed.contains(name)) {
                return defaultValue;
            }
            auto const parsedVal = parsed.at(name);
            switch (parsedVal.type()) {
                case toml::value_t::floating:
                    return static_cast<float>(parsedVal.as_floating());
                case toml::value_t::integer:
                    return static_cast<float>(parsedVal.as_integer());
                case toml::value_t::string: {
                    auto const &strVal = parsedVal.as_string();
                    if (strVal == "none") {
                        return std::nullopt;
                    }
                    FAIL("Illegal string value \"{}\" for config key \"{}\"", strVal.c_str(), name);
                }
                default:
                    FAIL("Illegal type {} for \"{}\"", toml::to_string(parsedVal.type()), name);
            }
        }

        static void applyConfigFile(toml_doc const &parsed, game::GameSettings &gameSettings) {
            gameSettings.mTargetFps = findOptionalFloatLenient(parsed, "target_fps", DEFAULT_TARGET_FPS);
            gameSettings.mVsyncDisabled = toml::find_or(parsed, "vsync_disabled", gameSettings.mVsyncDisabled);
        }

    public:
        Cli(int argc, char *argv[]) :
            mConfigFilePath("config.toml"),
            mGroup(clipp::option("-c", "--config") & clipp::value("config toml file", mConfigFilePath)) {

            if (!clipp::parse(argc, argv, mGroup)) {
                std::cout << clipp::make_man_page(mGroup, std::filesystem::path{argv[0]}.filename().string());
                std::exit(1);
            }
        }

        [[nodiscard]] game::GameSettings loadGameSettings() const {
            SDL_Log("Loading game settings at %s", std::filesystem::absolute(mConfigFilePath).c_str());

            const auto maybeParsed = toml::try_parse(mConfigFilePath, toml::spec::v(1, 1, 0));

            game::GameSettings gameSettings{
                    .mTargetFps = DEFAULT_TARGET_FPS,
            };

            if (maybeParsed.is_ok()) {
                auto const &parsed = maybeParsed.as_ok();
                if constexpr (LOG_TOML) {
                    std::stringstream ss;
                    ss << parsed;
                    SDL_Log("Loaded contents of %s:\n%s", mConfigFilePath.c_str(), ss.str().c_str());
                }
                applyConfigFile(parsed, gameSettings);
            } else {
                SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to load settings from %s. Using defaults.",
                            mConfigFilePath.c_str());
            }
            {
                std::stringstream ss;
                ss << gameSettings;
                SDL_Log("Game settings:\n%s", ss.str().c_str());
            }
            return gameSettings;
        }

    private:
        std::string mConfigFilePath;
        clipp::group mGroup;
    };
} // namespace gfx_testing
