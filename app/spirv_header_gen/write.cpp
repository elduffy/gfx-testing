#include <format>
#include <write.hpp>

#include <inja/inja.hpp>

namespace spirv_header_gen {
    std::filesystem::path getProjectRoot() {
        // TODO: share with gfx_testing::util::getProjectRoot
        std::filesystem::path path = std::filesystem::current_path();
        while (!is_empty(path)) {
            if (path.filename() == "gfx-testing") {
                return path;
            }
            path = path.parent_path();
        }
        throw std::runtime_error("Could not find project root directory.");
    }

    std::string getShaderStage(inja::Arguments const &args) {
        auto arg = args.at(0)->get<std::string>();
        if (arg == "vert") {
            return "SDL_GPU_SHADERSTAGE_VERTEX";
        }
        if (arg == "frag") {
            return "SDL_GPU_SHADERSTAGE_FRAGMENT";
        }
        throw std::runtime_error(std::format("Unsupported shader mode: {}", arg));
    }

    void writeSpirvMeta(WriteProperties const &writeProperties,
                        nlohmann::json const &json, std::ostream *ostream) {
        inja::Environment env;
        env.add_callback("getShaderStage", 1, getShaderStage);

        auto const templatePath = getProjectRoot() / "app/spirv_header_gen/template.hpp.inja";
        const inja::Template templ = env.parse_template(templatePath);

        // JSON model should look like:
        // https://github.com/KhronosGroup/SPIRV-Cross/blob/7918775748c5e2f5c40d9918ce68825035b5a1e1/spirv_reflect.cpp#L535-L550
        nlohmann::json model = json;
        model["package_name"] = writeProperties.mPackageName;

        env.render_to(*ostream, templ, model);
    }
}
