#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "address.hpp"

namespace crashlog 
{
	struct ExceptionMetadata {
		AddressMetadata address;
		uint32_t exceptionCode;
		std::string exceptionName;
		std::unordered_map<std::string, std::variant<std::string, int64_t, uint64_t>> additionalInfo;
	};

	typedef std::vector<AddressMetadata> StackTrace;
	typedef std::unordered_map<std::string, uint64_t> Registers;

	struct Exception {
		ExceptionMetadata exceptionMetadata;
		StackTrace stacktrace;
		Registers registers;
	};
};