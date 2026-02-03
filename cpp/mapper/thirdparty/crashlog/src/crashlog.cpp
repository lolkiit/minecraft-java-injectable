#include "../include/crashlog.hpp"

#ifdef CRASHLOG_WIN64
#include "../include/win64/parser.hpp"
#endif

void crashlog::initialize()
{
#ifdef CRASHLOG_WIN64
	crashlog::win64::initialize();
#endif
}

#ifdef CRASHLOG_WIN64
crashlog::Exception crashlog::parse(EXCEPTION_POINTERS* ptr) 
{
	return crashlog::win64::parse(ptr);
}
#endif
