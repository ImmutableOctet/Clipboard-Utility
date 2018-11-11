#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "types.hpp"

// A bit of a C-ism, but I'm too lazy to declare
// a separate source fiile for this:
#ifdef WIN32
	#define CLIP_PLATFORM_WINDOWS clip::platform::Windows
	
	#define CLIP_PLATFORM CLIP_PLATFORM_WINDOWS
#else
	#define CLIP_PLATFORM platform::Unknown
#endif

namespace clip
{
	namespace platform
	{
		enum
		{
			// Unknown / Unsupported platform.
			Unknown = 0,

			// Microsoft Windows; fully supported.
			Windows,
		};

		#ifdef CLIP_PLATFORM_WINDOWS
			using native_handle = HANDLE;
			using window_handle = HWND;

			const native_handle null_handle = NULL; // native_handle();
			const window_handle null_window = NULL;
		#endif
	}

	// ATTENTION: Native type; not RAII compliant.
	using handle = platform::native_handle;

	// ATTENTION: Native type; not RAII compliant.
	using window = platform::window_handle;

	constexpr auto& null_handle = platform::null_handle;
	constexpr auto& anonymous_window = platform::null_window;
}