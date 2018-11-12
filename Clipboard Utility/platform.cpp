#include "platform.hpp"

#include <algorithm>

namespace clip
{
	namespace platform
	{
		native_clipboard_format to_native_clipboard_format(clipboard_format type)
		{
			#ifdef CLIP_PLATFORM_WINDOWS
				// On Windows, formats are guaranteed to be the same as native.
				return static_cast<native_clipboard_format>(type);
			#else
				static_assert(false, "Unknown conversion path from portable to native clipboard formats.");
			#endif
		}

		clipboard_format to_portable_clipboard_format(native_clipboard_format type)
		{
			using format = clipboard_format;
			using native = native_clipboard_format;

			#ifdef CLIP_PLATFORM_WINDOWS
				/*
				switch (type)
				{
					case CF_TEXT:
						return format::TEXT;
					default: // case 0:
						return format::UNKNOWN;
				}

				return format::UNKNOWN;
				*/
				
				// On Windows, formats are guaranteed to be the same as native.
				return static_cast<format>(type);
			#else
				
				static_assert(false, "Unknown conversion path from native to portable clipboard formats.");
			#endif
		}

		void enum_clipboard_formats(const clipboard_enumerator& call_back, bool force_convert_types, clipboard_format starting_type)
		{
			#ifdef CLIP_PLATFORM_WINDOWS
				static const auto NATIVE_ANY = static_cast<native_clipboard_format>(clipboard_format::ANY);
				
				auto native_type = static_cast<native_clipboard_format>(starting_type);
				
				while (true)
				{
					native_type = EnumClipboardFormats(native_type);

					if (native_type != NATIVE_ANY)
					{
						clipboard_format type_out;

						if (force_convert_types)
						{
							type_out = to_portable_clipboard_format(native_type);
						}
						else
						{
							type_out = static_cast<clipboard_format>(native_type);
						}

						// Tell the caller about this clipboard segment,
						// and exit enumeration if requested to do so:
						if (!call_back(type_out))
							break;
					}
					else
					{
						/*
							Standard behavior for 'EnumClipboardFormats' is to
							return zero ('NATIVE_ANY') if enumeration is complete,
							or if an error has occurred.
							
							In either case, we need to stop our enumeration here.
						*/
						break;
					}
				}

				// Check if we short-circuited before checking for errors:
				if (native_type == NATIVE_ANY)
				{
					// Check for native error-codes:
					auto native_error_code = GetLastError();

					DEBUG_ASSERT((native_error_code == ERROR_SUCCESS), "Undetermined error detected during enumeration of clipboard segments.");
				}
			#else
				// Enumeration is not supported on this platform.
				return;
			#endif
		}

		memory_map memory_map::open(clipboard_format type)
		{
			#ifdef CLIP_PLATFORM_WINDOWS
				handle native = GetClipboardData(type);
				memory::raw_memory_ptr native_ptr = nullptr;

				return memory(std::move(native), std::move(native_ptr)); // CF_TEXT
			#else
				return {};
			#endif
		}

		memory_map::memory_map(native_handle&& handle, memory_map::raw_memory_ptr&& memory)
		{
			std::swap(this->resource_handle, handle);
			std::swap(this->resource_ptr, memory);
		}

		memory_map::memory_map(memory_map&& mem)
			: memory_map(std::move(mem.resource_handle), std::move(nullptr))
		{
			// Just to be safe, initialize default-state for the moved object.
			mem = memory_map();
		}

		// NOTE: The native handle is not closed in this implementation due
		// to Windows's requirement to leave the handle open, but unlocked.
		// With that in mind, however, if a different operating system requires
		// that the handle to clipboard should be closed, do so here.
		memory_map::~memory_map()
		{
			// We must be locked in order for
			// an unlock-operation to take place.
			if (exists() && locked())
			{
				unlock();
			}
		}

		memory_map::raw_memory_ptr memory_map::lock()
		{
			ASSERT(exists());

			if (locked())
			{
				ASSERT(resource_ptr != nullptr);

				return resource_ptr;
			}

			void* native = nullptr;
			
			// Fail on non-Windows platforms:
			#ifdef CLIP_PLATFORM_WINDOWS
				native = GlobalLock(resource_handle);
			#endif
			
			if (native)
			{
				resource_ptr = reinterpret_cast<raw_memory_ptr>(native);

				ASSERT(resource_ptr);

				if (resource_ptr)
				{
					return resource_ptr;
				}
			}

			return {};
		}

		bool memory_map::unlock(raw_memory_ptr raw_memory)
		{
			ASSERT(exists());

			if (unlocked())
			{
				ASSERT(resource_ptr == nullptr);

				return true;
			}

			DEBUG_ASSERT(raw_memory != nullptr, "Invalid raw-memory pointer.");
			DEBUG_ASSERT(this->resource_ptr == raw_memory, "Unknown memory resource given to open memory context.");
			
			// Fail on non-Windows platforms:
			#ifdef CLIP_PLATFORM_WINDOWS
				if (GlobalUnlock(this->resource_handle))
				{
					// Unlock the resource by discarding the pointer when safe to do so.
					this->resource_ptr = nullptr;
				}
			#endif

			return unlocked();
		}
		
		std::size_t memory_map::size() const
		{
			// Make sure our handle is valid before
			// asking the operating system about it.
			if (!exists())
				return 0;

			#ifdef CLIP_PLATFORM_WINDOWS
				// Ask Windows how big the clipboard is.
				return GlobalSize(resource_handle);
			#endif

			return 0;
		}

		std::size_t memory_map::text_length()
		{
			if (!exists())
				return 0;

			// Create a guarded lock in order to view clipboard memory.
			guard view(*this);

			// Retrieve a raw-pointer to the requested clipboard data-segment.
			const auto raw_data = view.ptr();

			// Return the length of our zero-terminated character-data.
			return strlen(reinterpret_cast<const char*>(raw_data));
		}
	}
}