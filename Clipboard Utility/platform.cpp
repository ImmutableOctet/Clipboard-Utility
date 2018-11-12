#include "platform.hpp"

#include <algorithm>

namespace clip
{
	namespace platform
	{
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
			// asking the operating system about it:
			if (exists())
			{
				#ifdef CLIP_PLATFORM_WINDOWS
					// Ask Windows how big the clipboard is.
					return GlobalSize(resource_handle);
				#endif
			}

			return 0;
		}

		std::size_t memory_map::text_size()
		{
			// Create a guarded lock in order to view clipboard memory.
			guard view(*this);

			// Retrieve a raw-pointer to the requested clipboard data-segment.
			auto raw_data = view.ptr();

			// Return the length of our zero-terminated character-data.
			return strlen(reinterpret_cast<const char*>(raw_data));
		}
	}
}