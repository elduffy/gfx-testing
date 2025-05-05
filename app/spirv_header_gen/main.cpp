#include <clipp.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <optional>

#include "boost/algorithm/string/replace.hpp"
#include "write.hpp"


std::string getPackageName(std::string const &inputFile) {
    std::string result = std::filesystem::path(inputFile).filename().string();
    boost::replace_last(result, ".json", "");
    boost::algorithm::replace_all(result, ".", "_");
    return result;
}

std::string getSourceSpvFilename(std::string const &inputFile) {
    std::string result = std::filesystem::path(inputFile).filename().string();
    boost::replace_last(result, ".json", ".spv");
    return result;
}

int main(int argc, char *argv[]) {
    using namespace clipp;
    std::string inputFile, outputFile;
    const auto cli =
            (value("input json file", inputFile), option("-o", "--output") & value("output header file", outputFile));
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

    auto const json = nlohmann::json::parse(inputFileStream);
    auto sourceSpvFilename = getSourceSpvFilename(inputFile);
    spirv_header_gen::WriteProperties props{
            .mPackageName = getPackageName(inputFile),
            .mSourceSpvFilename = sourceSpvFilename,
    };

    spirv_header_gen::writeHeader(props, json, outputStream);
}
