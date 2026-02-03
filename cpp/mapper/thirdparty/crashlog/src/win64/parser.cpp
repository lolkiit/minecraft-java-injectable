#include "../../include/win64/parser.hpp"

#include <DbgHelp.h>
#include <Windows.h>
#include <psapi.h>
#include <winnt.h>

#include <cstdint>
#include <vector>

#include "../../include/address.hpp"
#include "../../include/exception.hpp"
#include "../../include/win64/exception.hpp"
#include "../../include/win64/stacktrace.hpp"
#include "../../include/win64/symbols.hpp"

#pragma comment(lib, "DbgHelp.lib")

using namespace crashlog;

void win64::initialize()
{
	SymSetOptions(SYMOPT_LOAD_LINES);
	SymInitialize(GetCurrentProcess(), NULL, FALSE);

	crashlog::win64::symbols::loadAllSymbols();
}

AddressMetadata parseAddress(uintptr_t address)
{
	AddressMetadata metadata;
	metadata.address = address;
	auto [modName, modOffset] = win64::symbols::moduleInfo(address);
	metadata.moduleName = modName;
	metadata.moduleOffset = modOffset;
	metadata.symbol = win64::symbols::symbol(address);
	metadata.line = win64::symbols::line(address);
	return metadata;
}

void addAdditionalInfo(ExceptionMetadata& metadata, EXCEPTION_RECORD* record)
{
	bool isAccessViolation = record->ExceptionCode == EXCEPTION_ACCESS_VIOLATION;
	bool isInPageError = record->ExceptionCode == EXCEPTION_IN_PAGE_ERROR;
	if (!(isAccessViolation || isInPageError) || record->NumberParameters < 1)
	{
		return;
	}

	metadata.additionalInfo["violation_type"] = record->ExceptionInformation[0];
	metadata.additionalInfo["violation_type_name"] = win64::exception::violationType(record->ExceptionInformation[0]);
	metadata.additionalInfo["address"] = static_cast<uint64_t>(static_cast<uintptr_t>(record->ExceptionInformation[1]));

	if (isInPageError) 
	{
		metadata.additionalInfo["status_code"] = record->ExceptionInformation[2];
	}
}

Exception win64::parse(EXCEPTION_POINTERS* ptr) 
{
	EXCEPTION_RECORD* exceptionRecord = ptr->ExceptionRecord;
	CONTEXT* context = ptr->ContextRecord;

	ExceptionMetadata metadata;
	metadata.address = parseAddress(reinterpret_cast<uint64_t>(exceptionRecord->ExceptionAddress));
	metadata.exceptionCode = exceptionRecord->ExceptionCode;
	metadata.exceptionName = win64::exception::name(exceptionRecord->ExceptionCode);
	addAdditionalInfo(metadata, exceptionRecord);

	std::vector<uintptr_t> stackFrames = crashlog::win64::stacktrace::walkStack(ptr);
	std::vector<crashlog::AddressMetadata> addressMetadatas;
	for (uintptr_t frame : stackFrames)
	{
		addressMetadatas.push_back(parseAddress(frame));
	}

	Registers registers;
	registers["rax"] = context->Rax;
	registers["rbx"] = context->Rbx;
	registers["rcx"] = context->Rcx;
	registers["rdx"] = context->Rdx;
	registers["rsi"] = context->Rsi;
	registers["rdi"] = context->Rdi;
	registers["rbp"] = context->Rbp;
	registers["rsp"] = context->Rsp;
	registers["r8"] = context->R8;
	registers["r9"] = context->R9;
	registers["r10"] = context->R10;
	registers["r11"] = context->R11;
	registers["r12"] = context->R12;
	registers["r13"] = context->R13;
	registers["r14"] = context->R14;
	registers["r15"] = context->R15;
	registers["rip"] = context->Rip;

	return crashlog::Exception(metadata, addressMetadatas, registers);
}
