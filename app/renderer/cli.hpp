#pragma once
#include <clipp.h>
#include <filesystem>
#include <game.hpp>
#include <iostream>

namespace gfx_testing {
    class Cli {
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
            std::cout << "Path is: " << std::filesystem::absolute(mConfigFilePath).string() << std::endl;
            // TODO: load
            return game::GameSettings{.mTargetFps = 120};
        }

    private:
        std::string mConfigFilePath;
        clipp::group mGroup;
    };
} // namespace gfx_testing
