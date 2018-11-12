#pragma once

// A bit of a C-ism, but I'm too lazy to declare
// a separate source fiile for this:
#ifdef _WIN32
	#define CLIP_PLATFORM_WINDOWS clip::platform::Windows
	
	#define CLIP_PLATFORM CLIP_PLATFORM_WINDOWS
#else
	#define CLIP_PLATFORM platform::Unknown
#endif

#ifdef CLIP_PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	
	#include <windows.h>
	
	#ifndef WIN32
		static_assert(false, "Unable to determine valid Win32 header.")
	#endif
#endif

//#include <tuple>
//#include <optional>
#include <cstddef>

#include "assert.hpp"
#include "lock_guard.hpp"
#include "types.hpp"

namespace clip
{
	namespace platform
	{
		// Platforms:
		enum
		{
			// Unknown / Unsupported platform.
			Unknown = 0,

			// Microsoft Windows; fully supported.
			Windows,
		};

		// Clipboard formats:
		enum clipboard_format
		{
			// TEXT format supports conversion to string,
			// integer, and floating-point types natively.
			// Additional types can be supported through text-parsing.
			#ifdef CLIP_PLATFORM_WINDOWS
				TEXT = CF_TEXT,
			#else
				TEXT = 1,
			#endif
		};

		// Aliases (Win32):
		#ifdef CLIP_PLATFORM_WINDOWS
			using native_handle = HANDLE;
			using window_handle = HWND;

			const native_handle null_handle = NULL; // native_handle();
			const window_handle null_window = NULL;
		#endif

		// Memory maps are used to handle globally allocated clipboard/system data.
		// These maps are normally handled by 'clipboard' objects, and should only be
		// used with the understanding that they are system controlled resources with differing behavior.
		// For more details, view the 'memory_map::open' function.
		struct memory_map
		{
			public:
				using raw_memory_ptr = char*;
				using guard = lock_guard<memory_map, raw_memory_ptr, nullptr>;

				//friend class guard;

				// Static member-functions:

				// NOTE: Although a standard platform-independent interface exists here,
				// memory-maps are only guaranteed to have defined behavior when an instance
				// of a 'clipboard' object has been opened and initialized.
				// Under any other situation, behavior is OS/platform defined.
				static memory_map open(clipboard_format type);

				memory_map() = default;
				memory_map(native_handle&& handle, memory_map::raw_memory_ptr&& memory);

				memory_map(memory_map&& mem);

				~memory_map();

				inline bool is_null() const { return (resource_handle == null_handle); }
				inline bool exists() const { return !is_null(); }

				inline bool locked() const { return (resource_ptr != nullptr); }
				inline bool unlocked() const { return !locked(); }

				raw_memory_ptr lock();
				bool unlock(raw_memory_ptr raw_memory);

				/*
					Returns the raw OS-defined size of the mapped memory block.
					Implementations reserve the right to not implement this feature.
					
					If an implementation is not provided, this method shall return zero.
					
					In addition, if the internal handle is not valid,
					this method shall gracefully return zero.
				*/
				std::size_t size() const; // noexcept

				/*
					Unlike 'size', this command peeks into the TEXT segment
					of the clipboard (if available), and determines its size.
					
					NOTE: This command is NOT const-qualified, as it performs a
					lock-operation, which mutates the state of this memory-map.

					If a TEXT-segment does not exist, this method shall return zero.
				*/
				std::size_t text_size(); // noexcept

				inline operator raw_memory_ptr() const
				{
					ASSERT(locked());

					return resource_ptr;
				}

				inline operator bool() const { return exists(); }
			private:
				memory_map& operator=(const memory_map&) = default;

				// Used internally; a user should specify what they
				// think the resource is for contractual/safety purposes.
				inline bool unlock()
				{
					return unlock(resource_ptr);
				}

				native_handle resource_handle = null_handle;
				raw_memory_ptr resource_ptr = nullptr;
		};
	}

	// ATTENTION: Native type; not RAII compliant.
	using handle = platform::native_handle;

	// ATTENTION: Native type; not RAII compliant.
	using window = platform::window_handle;

	using memory = platform::memory_map;
	using memory_lock = memory::guard;

	constexpr auto& null_handle = platform::null_handle;
	constexpr auto& anonymous_window = platform::null_window;
}