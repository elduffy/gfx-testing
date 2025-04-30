#include <clipp.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <nlohmann/json.hpp>

#include "write.hpp"
#include "boost/algorithm/string/replace.hpp"


std::string getPackageName(std::string const &inputFile) {
    std::string result = std::filesystem::path(inputFile).filename().string();
    boost::replace_last(result, ".json", "");
    boost::algorithm::replace_all(result, ".", "_");
    return result;
}

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

    auto const json = nlohmann::json::parse(inputFileStream);
    spirv_header_gen::WriteProperties props{
            .mPackageName = getPackageName(inputFile),
    };

    spirv_header_gen::writeSpirvMeta(props, json, outputStream);
}
