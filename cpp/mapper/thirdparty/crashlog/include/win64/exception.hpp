#pragma once

#include <cstdint>
#include <string>

namespace crashlog::win64::exception
{
    std::string name(uint32_t code);
    std::string violationType(uint32_t code);
}
