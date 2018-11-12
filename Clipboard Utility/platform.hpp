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
#include <functional>

#include "assert.hpp"
#include "lock_guard.hpp"
#include "types.hpp"

namespace clip
{
	namespace platform
	{
		#ifdef CLIP_PLATFORM_WINDOWS
			using native_clipboard_format = UINT;
		#else
			using native_clipboard_format = unsigned int;
		#endif

		// Platforms:
		enum
		{
			// Unknown / Unsupported platform.
			Unknown = 0,

			// Microsoft Windows; fully supported.
			Windows,
		};

		// Clipboard formats:
		enum clipboard_format : native_clipboard_format
		{
			// Primarily used for enumeration.
			ANY = 0,

			// TEXT format supports conversion to string,
			// integer, and floating-point types natively.
			// Additional types can be supported through text-parsing.
			#ifdef CLIP_PLATFORM_WINDOWS
				TEXT = CF_TEXT,
			#else
				TEXT = 1,
			#endif
			
			#ifdef CLIP_PLATFORM_WINDOWS
				EXT_BITMAP = CF_BITMAP,
			#else
				EXT_BITMAP = 2,
			#endif
			
			UNKNOWN = ANY,
		};

		using clipboard_enumerator = std::function<bool(clipboard_format type)>;

		native_clipboard_format to_native_clipboard_format(clipboard_format type);

		// NOTE: On platforms other than Windows, this will return 'clipboard_format::UNKNOWN' on undocumented formats.
		clipboard_format to_portable_clipboard_format(native_clipboard_format type);

		/*
			This will enumerate clipboard formats, including native/OS defined formats.
			
			From your call-back: Return 'true' to continue, return 'false' to exit enumeration.
			
			Possible TODO: Change this to a template to remove overhead?
		*/
		void enum_clipboard_formats(const clipboard_enumerator& call_back, bool force_convert_types=false, clipboard_format starting_type=clipboard_format::ANY);

		bool has_clipboard_format(clipboard_format type=clipboard_format::ANY, bool force_convert_type=false);

		// Aliases (Win32):
		#ifdef CLIP_PLATFORM_WINDOWS
			using native_handle = HANDLE; // HGLOBAL;
			using window_handle = HWND;

			const native_handle null_handle = NULL; // native_handle();
			const window_handle null_window = NULL;
		#endif

		// Memory maps are used to handle globally allocated clipboard/system data.
		// These maps are normally handled by 'clipboard' objects, and should only be
		// used with the understanding that they are system controlled resources with differing behavior.
		// For more details, view the 'memory_map::open_clipboard' function.
		struct memory_map
		{
			public:
				using raw_memory_ptr = char*;
				using guard = lock_guard<memory_map, raw_memory_ptr, nullptr>;

				//friend class guard;

				// Static member-functions:

				/*
					This command opens a memory-map to a segment of the clipboard.
					If no segment exists, a 'null' memory-map will be returned.
					
					NOTE: Although a standard platform-independent interface exists here,
					memory-maps are only guaranteed to have defined behavior when an instance
					of a 'clipboard' object has been opened and initialized.
					
					Under any other situation, behavior is OS/platform defined.
				*/
				static memory_map open_clipboard(clipboard_format type);

				/*
					This command submits a global memory-map to the operating system's clipboard.
					
					NOTE: This version of submit does not remove true-ownership from the memory-map.
					
					To submit and alter state appropriately
					(Allowing the system and other applications to own the memory),
					please use the method equivalent.
				*/
				static bool submit_clipboard(const memory_map& inst, clipboard_format type);

				memory_map() = default;
				memory_map(native_handle&& handle, memory_map::raw_memory_ptr&& memory, bool perfect_ownership=false);
				memory_map(memory_map&& mem);
				memory_map(std::size_t size, bool zero_init=false);

				~memory_map();

				inline bool is_null() const { return (resource_handle == null_handle); }
				inline bool exists() const { return !is_null(); }

				inline bool locked() const { return (resource_ptr != nullptr); }
				inline bool unlocked() const { return !locked(); }

				inline bool perfect_ownership() const { return true_ownership; }

				raw_memory_ptr lock();
				bool unlock(raw_memory_ptr raw_memory);

				// This calls the global-version in order to submit the contents of the global buffer,
				// then alters the ownership status of this object, allowing the
				// submitted memory to be owned by something else.
				bool submit_clipboard(clipboard_format type);

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
				std::size_t text_length(); // noexcept

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

				bool true_ownership = false;
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