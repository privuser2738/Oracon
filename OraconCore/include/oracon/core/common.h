#ifndef ORACON_CORE_COMMON_H
#define ORACON_CORE_COMMON_H

#include "types.h"
#include <string>
#include <stdexcept>

namespace oracon {
namespace core {

// Version information
constexpr u32 ORACON_VERSION_MAJOR = 0;
constexpr u32 ORACON_VERSION_MINOR = 1;
constexpr u32 ORACON_VERSION_PATCH = 0;

String getVersionString();

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #ifndef ORACON_PLATFORM_WINDOWS
        #define ORACON_PLATFORM_WINDOWS
    #endif
#elif defined(__linux__)
    #ifndef ORACON_PLATFORM_LINUX
        #define ORACON_PLATFORM_LINUX
    #endif
#elif defined(__APPLE__) && defined(__MACH__)
    #ifndef ORACON_PLATFORM_MACOS
        #define ORACON_PLATFORM_MACOS
    #endif
#else
    #ifndef ORACON_PLATFORM_UNKNOWN
        #define ORACON_PLATFORM_UNKNOWN
    #endif
#endif

// Compiler detection
#if defined(__clang__)
    #define ORACON_COMPILER_CLANG
#elif defined(__GNUC__) || defined(__GNUG__)
    #define ORACON_COMPILER_GCC
#elif defined(_MSC_VER)
    #define ORACON_COMPILER_MSVC
#endif

// Debug macros
#ifdef NDEBUG
    #define ORACON_DEBUG 0
#else
    #define ORACON_DEBUG 1
#endif

// Assert macro
#if ORACON_DEBUG
    #define ORACON_ASSERT(condition, message) \
        do { \
            if (!(condition)) { \
                throw std::runtime_error("Assertion failed: " + std::string(message)); \
            } \
        } while (false)
#else
    #define ORACON_ASSERT(condition, message) ((void)0)
#endif

// Unused parameter macro
#define ORACON_UNUSED(x) (void)(x)

// Force inline
#if defined(ORACON_COMPILER_MSVC)
    #define ORACON_FORCE_INLINE __forceinline
#else
    #define ORACON_FORCE_INLINE inline __attribute__((always_inline))
#endif

// No inline
#if defined(ORACON_COMPILER_MSVC)
    #define ORACON_NO_INLINE __declspec(noinline)
#else
    #define ORACON_NO_INLINE __attribute__((noinline))
#endif

} // namespace core
} // namespace oracon

#endif // ORACON_CORE_COMMON_H
