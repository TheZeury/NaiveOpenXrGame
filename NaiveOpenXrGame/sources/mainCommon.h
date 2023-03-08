#pragma once

import std;

#pragma once

// #define _ERRNO_
// #define _FENV_11
// #define _FLOAT_
// #define _INTTYPES_
// #define _ISO646_
// #define _LIMITS_
// #define _LOCALE_
// #define _MATH_
// #define _SETJMP_
// #define _SIGNAL_
// #define _STDALIGN_
// #define _STDARG_
// #define _STDBOOL_
// #define _STDDEF_
// #define _STDINT_
// #define _STDIO_
// #define _STDLIB_
// #define _STRING_
// #define _TGMATH_
// #define _TIME_
// #define _UCHAR_
// #define _WCHAR_
// #define _WCTYPE_

#define _CASSERT_
#define _CCOMPLEX_
#define _CCTYPE_
#define _CERRNO_
#define _CFENV_
#define _CFLOAT_
#define _CINTTYPES_
#define _CISO646_
#define _CLIMITS_
#define _CLOCALE_
#define _CMATH_
#define _CSETJMP_
#define _CSIGNAL_
#define _CSTDALIGN_
#define _CSTDARG_
#define _CSTDBOOL_
#define _CSTDDEF_
#define _CSTDINT_
#define _CSTDIO_
#define _CSTDLIB_
// #define _CSTRING_
#define _CTGMATH_
#define _CTIME_
#define _CCUCHAR_
#define _CWCHAR_
#define _CWCTYPE_

#define _ALGORITHM_
#define _ANY_
#define _ARRAY_
#define _ATOMIC_
#define _BIT_
#define _BITSET_
#define _CHARCONV_
#define _CHRONO_
#define _CODECVT_
#define _CONCEPTS_
#define _COMPARE_
#define _COMPLEX_
#define _CONDITION_VARIABLE_
#define _CONTRACT_
#define _COROUTINE_
#define _DEQUE_
#define _EXCEPTION_
#define _EXECUTION_
#define _FILESYSTEM_
#define _FORMAT_
#define _FORWARD_LIST_
#define _FSTREAM_
#define _FUNCTIONAL_
#define _FUTURE_
#define _INITIALIZER_LIST_
#define _IOMANIP_
#define _IOS_
#define _IOSFWD_
#define _IOSTREAM_
#define _ISTREAM_
#define _ITERATOR_
#define _LIMITS_
#define _LIST_
#define _LOCALE_
#define _MAP_
#define _MEMORY_
#define _MEMORY_RESOURCE_
#define _MUTEX_
#define _NEW_
#define _NUMERIC_
#define _OPTIONAL_
#define _OSTREAM_
#define _QUEUE_
#define _RANDOM_
#define _RANGES_
#define _RATIO_
#define _REGEX_
#define _SCOPED_ALLOCATOR_
#define _SPAN_
#define _SET_
#define _SHARED_MUTEX_
#define _SSTREAM_
#define _STACK_
#define _STDEXCEPT_
#define _STREAMBUF_
#define _STRING_
#define _STRING_VIEW_
#define _STRSTREAM_
#define _SYSTEM_ERROR_
#define _SYNC_STREAM_
#define _THREAD_
#define _TUPLE_
#define _TYPE_TRAITS_
#define _TYPEINDEX_
#define _TYPEINFO_
#define _UNORDERED_MAP_
#define _UNORDERED_SET_
#define _UTILITY_
#define _VALARRAY_
#define _VARIANT_
#define _VECTOR_
#define _XMEMORY_
#define _XSTRING_
#define _XUTILITY_
// #define _VERSION_ Probably should leave this out as we want feature test macros

#define XR_USE_GRAPHICS_API_VULKAN
#include <vulkan/vulkan.hpp>
#include "..\external\xr_linear.h"
#include <openxr/openxr_platform.h>
#include <openxr/openxr.hpp>
#define _SILENCE_CXX20_CISO646_REMOVED_WARNING
#include <PxPhysicsAPI.h>
using namespace physx;
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

import stb;
#include "..\external\tiny_obj_loader.h"

#define LOGMESSAGE
#if !defined(LOGMESSAGE)
#if !defined(NDEBUG)
#define LOGMESSAGE
#endif
#endif

#if defined(LOGMESSAGE)
#define LOG_STEP(INSTANCE, STEP) LogStep(INSTANCE, STEP)
#define LOG_SUCCESS() LogSuccess()
#define LOG_FAILURE() LogFailure()
#define LOG_INFO(INSTANCE, INFO, TAB) LogInformation(INSTANCE, INFO, TAB)
#define LOG_ERRO(ERR) LogError(ERR)
#else
#define LOG_STEP(INSTANCE, STEP) 
#define LOG_SUCCESS()
#define LOG_FAILURE()
#define LOG_INFO(INSTANCE, INFO, TAB)
#define LOG_ERRO(ERR)
#endif

#define MAKE_HANDLE(CLASS_NAME) \
class CLASS_NAME; \
namespace hd \
{ \
	using CLASS_NAME = std::shared_ptr<CLASS_NAME>; \
} \
namespace rf \
{ \
	using CLASS_NAME = std::weak_ptr<CLASS_NAME>; \
}

//#define MIRROR_WINDOW
//#define MIDDLE_EYE_MIRRORING

void inline LogStep(std::string instance, std::string step)
{
	std::cout << "\033[0m[\033[1;36mSTEP\033[0m] " << "\033[1;34m" + instance + "\t\033[0m" + step + "...\t";
}

void inline LogSuccess()
{
	std::cout << "\033[1;32mSuccess\033[0m\n";
}

void inline LogFailure()
{
	std::cout << "\033[1;31mFailure\033[0m\n";
}

void inline LogInformation(std::string instance, std::string information, size_t table)
{
	std::cout << "\033[0m[\033[1;33mINFO\033[0m] " << "\033[1;34m" + instance;
	while (table--) std::cout << '\t';
	std::cout << "\t\033[0m" + information << std::endl;
}

void inline LogError(std::string errorMessage)
{
	std::cout << "\033[0m[\033[1;31mERRO\033[0m] " << errorMessage << std::endl;
}

namespace Noxg
{
	template<typename P> concept SharedPtr = requires (P ptr) { ptr.use_count(); };
	template<typename P, typename... Types> requires SharedPtr<P>
	P make_new(Types... args)
	{
		return std::make_shared<typename P::element_type>(args...);
	}
}