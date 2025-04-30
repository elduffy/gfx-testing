#include <parse.hpp>

#include <format>

#include "nlohmann/json.hpp"

namespace spirv_header_gen {

    void fillEntryPoints(nlohmann::json const &json, gfx_testing::shader::SpirvMeta &meta) {
        for (auto const &elem: json.at("entryPoints")) {
            auto &[mName, mMode] = meta.mEntryPoints.emplace_back();
            mName = elem.at("name");
            std::string mode = elem.at("mode");
            if (mode == "vert") {
                mMode = gfx_testing::shader::ShaderMode::Vertex;
            } else if (mode == "frag") {
                mMode = gfx_testing::shader::ShaderMode::Fragment;
            } else {
                throw std::runtime_error(std::format("Unsupported shader mode: {}", mode));
            }
        }
    }

    void fillTypeData(nlohmann::json const &json, gfx_testing::shader::TypeMember &member) {
        if (json.contains("matrix_stride") || json.contains("row_major")) {
            member.mTypeData = gfx_testing::shader::MatrixData{
                    .mMaxtrixStride = json.at("matrix_stride"),
                    .mRowMajor = json.at("row_major")
            };
        }
    }

    void fillTypes(nlohmann::json const &json, gfx_testing::shader::SpirvMeta &meta) {
        for (auto const &[key, jsonObj]: json.at("types").items()) {
            gfx_testing::shader::Type type{};
            type.mName = jsonObj.at("name");
            for (auto const &jsonMember: jsonObj.at("members")) {
                auto &mem = type.mMembers.emplace_back();
                mem.mName = jsonMember.at("name");
                mem.mType = jsonMember.at("type");
                mem.mOffset = jsonMember.at("offset");
                fillTypeData(jsonMember, mem);
            }
            meta.mTypes.emplace(key, type);
        }
    }

    void fillInputs(nlohmann::json const &json, gfx_testing::shader::SpirvMeta &meta) {
        for (auto const &elem: json.at("inputs")) {
            auto &[mName, mType, mLocation] = meta.mInputs.emplace_back();
            mName = elem.at("name");
            mType = elem.at("type");
            mLocation = elem.at("location");
        }
    }

    void fillOutputs(nlohmann::json const &json, gfx_testing::shader::SpirvMeta &meta) {
        for (auto const &elem: json.at("outputs")) {
            auto &[mName, mType, mLocation] = meta.mOutputs.emplace_back();
            mName = elem.at("name");
            mType = elem.at("type");
            mLocation = elem.at("location");
        }
    }

    void fillUbos(nlohmann::json::const_reference json, gfx_testing::shader::SpirvMeta &meta) {
        if (!json.contains("ubos")) {
            return;
        }
        for (auto const &elem: json.at("ubos")) {
            auto &[mName, mType, mBlockSize, mSet, mBinding] = meta.mUbos.emplace_back();
            mName = elem.at("name");
            mType = elem.at("type");
            mBlockSize = elem.at("block_size");
            mSet = elem.at("set");
            mBinding = elem.at("binding");
        }
    }

    void fillSeparateImages(nlohmann::json::const_reference json, gfx_testing::shader::SpirvMeta &meta) {
        if (!json.contains("separate_images")) {
            return;
        }
        for (auto const &elem: json.at("separate_images")) {
            auto &[mName, mType, mSet, mBinding] = meta.mSeparateImages.emplace_back();
            mName = elem.at("name");
            mType = elem.at("type");
            mSet = elem.at("set");
            mBinding = elem.at("binding");
        }
    }

    void fillSeparateSamplers(nlohmann::json::const_reference json, gfx_testing::shader::SpirvMeta &meta) {
        if (!json.contains("separate_samplers")) {
            return;
        }
        for (auto const &elem: json.at("separate_samplers")) {
            auto &[mName, mType, mSet, mBinding] = meta.mSeparateSamplers.emplace_back();
            mName = elem.at("name");
            mType = elem.at("type");
            mSet = elem.at("set");
            mBinding = elem.at("binding");
        }
    }

    gfx_testing::shader::SpirvMeta parseSpirvMeta(std::ifstream &ifstream) {
        const nlohmann::json json = nlohmann::json::parse(ifstream);
        gfx_testing::shader::SpirvMeta meta;

        fillEntryPoints(json, meta);
        fillTypes(json, meta);
        fillInputs(json, meta);
        fillOutputs(json, meta);
        fillUbos(json, meta);
        fillSeparateImages(json, meta);
        fillSeparateSamplers(json, meta);

        return meta;
    }
}
