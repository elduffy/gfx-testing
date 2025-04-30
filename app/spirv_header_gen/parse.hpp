#pragma once

#include <fstream>

#include "spirv_header.hpp"


namespace spirv_header_gen {
    gfx_testing::shader::SpirvMeta parseSpirvMeta(std::ifstream &ifstream);
}
