#define TINYOBJLOADER_IMPLEMENTATION
#include <filesystem>
#include <iostream>
#include <tiny_obj_loader.h>
#include <SDL3/SDL_log.h>

namespace gfx_testing::model {
    void loadObjFile(const std::filesystem::path &path) {
        SDL_Log("Loading obj file %s", path.c_str());

        tinyobj::ObjReaderConfig reader_config;
        reader_config.mtl_search_path = path.parent_path();
        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(path.string(), reader_config)) {
            if (!reader.Error().empty()) {
                std::cerr << reader.Error() << std::endl;
            }
            throw std::runtime_error("Failed to load obj file");
        }
        if (!reader.Warning().empty()) {
            std::cerr << reader.Warning() << std::endl;
        }

        // auto const &attrib = reader.GetAttrib();
        auto const &shapes = reader.GetShapes();
        auto const &materials = reader.GetMaterials();

        SDL_Log("Successfully loaded obj file %s with %zu shapes, %zu materials", path.c_str(), shapes.size(),
                materials.size());

        for (auto const &shape: shapes) {
            SDL_Log("Shape %s has %zu indices", shape.name.c_str(), shape.mesh.indices.size());
        }
    }
}
