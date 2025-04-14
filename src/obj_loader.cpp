#define TINYOBJLOADER_IMPLEMENTATION
#include <filesystem>
#include <iostream>
#include <tiny_obj_loader.h>
#include <SDL3/SDL_log.h>

namespace gfx_testing::model {
    void loadObjFile(const std::filesystem::path& path) {
        SDL_Log("Loading obj file %s", path.c_str());

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;
        const bool ret = LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str(), path.parent_path().c_str());

        if (!err.empty()) {
            std::cerr << err << std::endl;
        }
        if (!warn.empty()) {
            std::cerr << warn << std::endl;
        }
        if (!ret) {
            throw std::runtime_error("Failed to load obj file");
        }

        SDL_Log("Successfully loaded obj file %s with %zu shapes, %zu materials", path.c_str(), shapes.size(), materials.size());

        for (auto const& shape : shapes) {
            SDL_Log("Shape %s has %zu indices", shape.name.c_str(), shape.mesh.indices.size());
        }
    }
}
