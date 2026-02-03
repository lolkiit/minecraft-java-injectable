#include "../include/address.hpp"
#include <format>
#include <string>

std::string crashlog::addressToString(crashlog::AddressMetadata addr)
{
    std::string result;
    std::string mod = std::format("{} + {:#x}", addr.moduleName, addr.moduleOffset);

    if (addr.symbol)
    {
        result += std::format("{} ({})", *addr.symbol, mod);
    } 
    else 
    {
        result += mod;
    }

    if (addr.line)
    {
        result += std::format(" [{}:{}]", addr.line->fileName, addr.line->lineNo);
    }

    return result;
}