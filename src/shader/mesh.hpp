#pragma once
#include <array>
#include <ostream>
#include <sstream>
#include <vector>

#include <SDL3/SDL.h>
#include <absl/log/check.h>
#include <glm/mat4x4.hpp>
#include <gooch.frag.hpp>
#include <shader/vertex.hpp>
#include <util/util.hpp>


namespace gfx_testing::shader {

    class IndexList {
        template<typename index_t>
        std::unique_ptr<uint8_t> copyVector(std::vector<index_t> const &indices) {
            std::unique_ptr<uint8_t> data(new uint8_t[indices.size() * sizeof(index_t)]);
            index_t *dest = reinterpret_cast<index_t *>(data.get());
            std::copy(indices.begin(), indices.end(), dest);
            return data;
        }

    public:
        NO_COPY_DEFAULT_MOVE(IndexList);

        explicit IndexList(std::vector<uint16_t> const &indices) :
            mCount(indices.size()), mBuffer(copyVector(indices)), mElementSize(SDL_GPU_INDEXELEMENTSIZE_16BIT) {}

        explicit IndexList(std::vector<uint32_t> const &indices) :
            mCount(indices.size()), mBuffer(copyVector(indices)), mElementSize(SDL_GPU_INDEXELEMENTSIZE_32BIT) {}

        size_t count() const { return mCount; }

        template<typename index_t>
        index_t const *as() const;

        uint16_t const *asUint16() const {
            CHECK(mElementSize == SDL_GPU_INDEXELEMENTSIZE_16BIT) << "Attempt to access 32-bit indices as 16-bit";
            return reinterpret_cast<uint16_t const *>(mBuffer.get());
        }

        uint32_t const *asUint32() const {
            CHECK(mElementSize == SDL_GPU_INDEXELEMENTSIZE_32BIT) << "Attempt to access 16-bit indices as 32-bit";
            return reinterpret_cast<uint32_t const *>(mBuffer.get());
        }

        template<typename index_t>
        std::vector<index_t> asVector() const {
            std::vector<index_t> result;
            auto const *p = as<index_t>();
            std::copy_n(p, mCount, std::back_inserter(result));
            return result;
        }

        size_t elementSize() const {
            switch (mElementSize) {
                case SDL_GPU_INDEXELEMENTSIZE_16BIT:
                    return sizeof(uint16_t);
                case SDL_GPU_INDEXELEMENTSIZE_32BIT:
                    return sizeof(uint32_t);
                default:
                    FAIL("Unexpected element size {}", static_cast<uint32_t>(mElementSize));
            }
        }

        size_t bufferSize() const { return count() * elementSize(); }

        std::string toString() const {
            std::stringstream ss;
            ss << "[";
            switch (mElementSize) {
                case SDL_GPU_INDEXELEMENTSIZE_16BIT: {
                    auto const *p = asUint16();
                    for (size_t i = 0; i < mCount; ++i) {
                        ss << p[i];
                        ss << ", ";
                    }
                    break;
                }
                case SDL_GPU_INDEXELEMENTSIZE_32BIT: {
                    auto const *p = asUint32();
                    for (size_t i = 0; i < mCount; ++i) {
                        ss << p[i];
                        ss << ", ";
                    }
                    break;
                }
            }
            ss << "]";
            return ss.str();
        }

    private:
        size_t const mCount;
        std::unique_ptr<uint8_t const> mBuffer;

    public:
        SDL_GPUIndexElementSize const mElementSize;
    };

    template<>
    inline uint16_t const *IndexList::as() const {
        return asUint16();
    }

    template<>
    inline uint32_t const *IndexList::as() const {
        return asUint32();
    }

    inline uint32_t getVertexBufferSize(size_t vertexCount) {
        return util::narrow_u32(vertexCount * sizeof(VertexData));
    }

    struct MeshData {
        NO_COPY_DEFAULT_MOVE(MeshData);

        MeshData(std::vector<VertexData> vertices, IndexList indices) :
            mVertices(std::move(vertices)), mIndices(std::move(indices)) {}

        [[nodiscard]] uint32_t getVertexBufferSize() const { return shader::getVertexBufferSize(mVertices.size()); }

        [[nodiscard]] uint32_t getIndexBufferSize() const { return util::narrow_u32(mIndices.bufferSize()); }

        [[nodiscard]] std::string toString() const {
            std::stringstream ss;
            ss << "MeshData {\n";
            ss << "\tmVertices(";
            ss << mVertices.size();
            ss << ") = [\n";
            for (const auto &v: mVertices) {
                ss << "\t\t";
                ss << v.toString();
                ss << ",\n";
            }
            ss << "\t],\n";
            ss << "\tmIndices(";
            ss << mIndices.count();
            ss << ") = ";
            ss << mIndices.toString();
            ss << "\n";
            ss << "}";
            return ss.str();
        }

        std::vector<VertexData> mVertices;
        IndexList mIndices;
    };

    struct MeshDataBuilder {

        MeshData build() {
            CHECK(!mVertices.empty()) << "MeshDataBuilder: no vertices set";
            CHECK(!mIndices16.empty() || !mIndices32.empty()) << "MeshDataBuilder: no indices set";
            if (mIndices32.size() > 0) {
                return {std::move(mVertices), IndexList(mIndices32)};
            }
            return {std::move(mVertices), IndexList(mIndices16)};
        }

        void addIndex(uint32_t index) {
            if (mIndices32.size() > 0) {
                mIndices32.push_back(index);
            } else if (index <= std::numeric_limits<uint16_t>::max()) {
                mIndices16.push_back(index);
            } else {
                std::ranges::move(mIndices16, std::back_inserter(mIndices32));
                mIndices16.clear();
            }
        }

        std::vector<VertexData> mVertices;

    private:
        std::vector<uint16_t> mIndices16;
        std::vector<uint32_t> mIndices32;
    };
} // namespace gfx_testing::shader
