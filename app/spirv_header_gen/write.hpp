#pragma once
#include <iosfwd>

#include "nlohmann/adl_serializer.hpp"


namespace spirv_header_gen {

    struct WriteProperties {
        std::string mPackageName;
    };


    void writeSpirvMeta(WriteProperties const &writeProperties, nlohmann::json const &json, std::ostream *ostream);
}
