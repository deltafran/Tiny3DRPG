#pragma once

#if defined( _WIN64 )
#define PLATFORM_64BITS					1
#else
#define PLATFORM_64BITS					0
#endif

#if _WIN32 || _WIN64
#define PLATFORM_WINDOWS 1
#endif

#if !defined(PLATFORM_WINDOWS)
#define PLATFORM_WINDOWS 0
#endif

#if !defined(PLATFORM_MAC)
#define PLATFORM_MAC 0
#endif

#if !defined(PLATFORM_IOS)
#define PLATFORM_IOS 0
#endif

#if !defined(PLATFORM_ANDROID)
#define PLATFORM_ANDROID 0
#endif

#if !defined(PLATFORM_LINUX)
#define PLATFORM_LINUX 0
#endif

#if PLATFORM_WINDOWS
#	define FORCEINLINE __inline
#else
#	define FORCEINLINE inline
#endif