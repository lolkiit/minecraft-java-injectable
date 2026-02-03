#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>

#include "../address.hpp"

namespace crashlog::win64::symbols
{
	void loadAllSymbols();

	std::pair<std::string, uint64_t> moduleInfo(uintptr_t address);

	std::optional<std::string> symbol(uintptr_t address);

	std::optional<LineInfo> line(uintptr_t address);
};
