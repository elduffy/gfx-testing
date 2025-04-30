#pragma once
#include <iosfwd>

#include "spirv_header.hpp"


namespace spirv_header_gen {
    void writeSpirvMeta(gfx_testing::shader::SpirvMeta const &meta, std::ostream *ostream);
}
