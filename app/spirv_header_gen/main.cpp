#include <clipp.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <nlohmann/json.hpp>

#include "parse.hpp"
#include "spirv_header.hpp"
#include "write.hpp"


int main(int argc, char *argv[]) {
    using namespace clipp;
    std::string inputFile, outputFile;
    const auto cli = (
        value("input json file", inputFile),
        option("-o", "--output") & value("output header file", outputFile)
    );
    if (!parse(argc, argv, cli)) {
        std::cout << make_man_page(cli, std::filesystem::path{argv[0]}.filename().string());
        return 1;
    }

    std::ifstream inputFileStream{inputFile};

    std::optional<std::ofstream> outputFileStream{};
    std::ostream *outputStream;
    if (outputFile.empty() || outputFile == "-") {
        outputStream = &std::cout;
    } else {
        outputFileStream.emplace(std::ofstream{outputFile});
        outputStream = &*outputFileStream;
    }

    auto const spirvMeta = spirv_header_gen::parseSpirvMeta(inputFileStream);
    spirv_header_gen::writeSpirvMeta(spirvMeta, outputStream);
}
