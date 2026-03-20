#include <fstream>
#include <io/scene_loader.hpp>
#include <nlohmann/json.hpp>
#include <util/util.hpp>

namespace gfx_testing::io {

    static glm::vec3 parseVec3(nlohmann::json const &j, glm::vec3 defaultVal = {0, 0, 0}) {
        if (!j.is_array() || j.size() != 3) {
            return defaultVal;
        }
        return {j[0].get<float>(), j[1].get<float>(), j[2].get<float>()};
    }

    SceneDefinition loadSceneDefinition(std::filesystem::path const &path) {
        std::ifstream file(path);
        CHECK(file.is_open()) << "Could not open scene file: " << path;

        auto const json = nlohmann::json::parse(file);
        SceneDefinition scene;

        for (auto const &obj : json.at("objects")) {
            auto const type = obj.at("type").get<std::string>();

            if (type == "mesh") {
                MeshObjectDef def;
                def.mName = obj.at("name").get<std::string>();
                def.mModel = obj.at("model").get<std::string>();
                def.mPipeline = obj.at("pipeline").get<std::string>();
                if (obj.contains("position")) {
                    def.mPosition = parseVec3(obj["position"]);
                }
                if (obj.contains("scale")) {
                    def.mScale = parseVec3(obj["scale"], {1, 1, 1});
                }
                if (obj.contains("untextured")) {
                    def.mUntextured = obj["untextured"].get<bool>();
                }
                if (obj.contains("behavior")) {
                    def.mBehavior = obj["behavior"].get<std::string>();
                }
                if (obj.contains("behaviorParams")) {
                    auto const &params = obj["behaviorParams"];
                    if (params.contains("radsPerSecond")) {
                        def.mBehaviorRadsPerSecond = params["radsPerSecond"].get<float>();
                    }
                }
                scene.mObjects.emplace_back(std::move(def));
            } else if (type == "skybox") {
                SkyboxDef def;
                def.mName = obj.at("name").get<std::string>();
                def.mCubemap = obj.at("cubemap").get<std::string>();
                scene.mObjects.emplace_back(std::move(def));
            } else if (type == "point_lights") {
                PointLightsDef def;
                def.mName = obj.at("name").get<std::string>();
                if (obj.contains("count")) {
                    def.mCount = obj["count"].get<int>();
                }
                if (obj.contains("model")) {
                    def.mModel = obj["model"].get<std::string>();
                }
                if (obj.contains("orbitRadius")) {
                    def.mOrbitRadius = obj["orbitRadius"].get<float>();
                }
                scene.mObjects.emplace_back(std::move(def));
            } else {
                FAIL("Unknown scene object type: '{}'", type);
            }
        }

        return scene;
    }

} // namespace gfx_testing::io
