#include <format>
#include <inja/inja.hpp>
#include <write.hpp>

#include "boost/algorithm/string/classification.hpp"
#include "boost/algorithm/string/detail/trim.hpp"

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

    std::string getTypeVariableNameImpl(std::string const &typeStr) {
        auto const newStart =
                boost::algorithm::detail::trim_begin(typeStr.begin(), typeStr.end(), boost::is_any_of("type."));
        return std::string{&*newStart};
    }

    std::string getTypeVariableName(inja::Arguments const &args) {
        const auto typeStr = args.at(0)->get<std::string>();
        return getTypeVariableNameImpl(typeStr);
    }

    std::string lookupTypeVariableName(inja::Arguments const &args) {
        const auto typeMap = args.at(0)->get<nlohmann::json::object_t>();
        const auto keyStr = args.at(1)->get<std::string>();

        auto const &typeObj = typeMap.at(keyStr);
        const auto typeName = typeObj["name"].get<std::string>();

        return getTypeVariableNameImpl(typeName);
    }

    std::string getGenTime() {
        time_t now;
        time(&now);
        char buf[sizeof "2011-10-08T07:07:09Z"];
        strftime(buf, sizeof buf, "%FT%TZ", gmtime(&now));
        return {buf};
    }

    void writeHeader(WriteProperties const &writeProperties, nlohmann::json const &json, std::ostream *ostream) {
        inja::Environment env;
        env.add_callback("getShaderStage", 1, getShaderStage);
        env.add_callback("getTypeVariableName", 1, getTypeVariableName);
        env.add_callback("lookupTypeVariableName", 2, lookupTypeVariableName);

        auto const templatePath = getProjectRoot() / "app/spirv_header_gen/template.hpp.inja";
        const inja::Template templ = env.parse_template(templatePath);

        // JSON model should look like:
        // https://github.com/KhronosGroup/SPIRV-Cross/blob/7918775748c5e2f5c40d9918ce68825035b5a1e1/spirv_reflect.cpp#L535-L550
        nlohmann::json model = json;
        model["package_name"] = writeProperties.mPackageName;
        model["source_spv_filename"] = writeProperties.mSourceSpvFilename;
        model["gen_time"] = getGenTime();
        if (json["entryPoints"].size() != 1) {
            throw std::runtime_error("Only a single entry point is supported.");
        }
        model["entryPoint"] = json["entryPoints"].at(0);
        model.erase("entryPoints");

        // array defaults
        for (auto const *field:
             {"inputs", "outputs", "ubos", "separate_images", "separate_samplers", "ssbos", "images"}) {
            if (!model.contains(field)) {
                model[field] = nlohmann::json::array();
            }
        }
        // object defaults
        for (auto const *field: {"types"}) {
            if (!model.contains(field)) {
                model[field] = nlohmann::json::object();
            }
        }

        env.render_to(*ostream, templ, model);
    }
} // namespace spirv_header_gen
