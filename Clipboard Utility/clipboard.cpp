#include "clipboard.hpp"

#include <string>
#include <cassert>
#include <exception>

#include <iostream>
#include <fstream>

static_assert(CLIP_PLATFORM == clip::platform::Windows, "Only windows is supported at this time.");

namespace clip
{
	clipboard::clipboard(const window& wnd)
		: owner(wnd)
	{
		open(owner);
	}

	clipboard::clipboard(clipboard&& c)
		: clipboard(static_cast<const clipboard&>(c))
	{
		c.access = false;
		//c.owner = anonymous_window;
	}

	clipboard::~clipboard()
	{
		close(owner);
	}

	// This may be changed into a template at a later date:
	std::string clipboard::read_text() const
	{
		using T = std::string;

		static_assert(std::is_default_constructible_v<T>, "Specified type must be default-constructible.");
		static_assert(std::is_constructible_v<T, const char*>, "Specified type must be constructible using 'const char*' as a parameter.");

		ASSERT(is_open());

		auto m = context(format::TEXT);

		// Check if we were able to open the memory segment:
		if (m)
		{
			// Lock the memory segment, so that we can read from it.
			memory_lock guard(m);

			// Retrieve a raw pointer to the memory segment.
			const auto raw_data = guard.ptr();
			
			// Acquire a C-style string (Pointer) from our "raw-pointer".
			auto c_str = static_cast<const char*>(raw_data);

			// Verify that the C-string exists before we try to use it.
			if (c_str)
			{
				// Copy our C-string into another object, then return it.
				// Memory will be cleaned up automatically from here. (RAII)
				return T(c_str);
			}
		}

		return T();
	}

	bool clipboard::read_text_raw(void* data_out, std::size_t size, std::size_t offset) const
	{
		ASSERT(is_open());

		auto m = context(format::TEXT);

		// Check if we were able to open the memory segment:
		if (m)
		{
			const auto memory_size = m.size();

			// Determine if the area requested is within the memory-context's range:
			if ((size + offset) > memory_size)
			{
				return false;
			}

			// Lock the memory segment, so that we can read from it.
			memory_lock guard(m);

			// Retrieve a raw pointer to the memory segment.
			const auto raw_data = guard.ptr();

			// Acquire a C-style string (Pointer) from our "raw-pointer".
			auto raw_bytes = reinterpret_cast<const uint8_t*>(raw_data);

			// Verify that the C-string exists before we try to use it.
			if (raw_bytes)
			{
				return (std::memcpy(data_out, (raw_bytes + offset), size) != nullptr);
			}
		}

		return false;
	}

	bool clipboard::write_text(const std::string& data) const
	{
		return write_text_raw(data.c_str(), (data.size() + 1));
	}

	bool clipboard::write_text_raw(const void* data, std::size_t size, std::size_t offset) const
	{
		ASSERT(is_open());

		const auto write_size = (size + offset);

		auto m = memory(write_size);
		
		const auto memory_size = m.size();

		ASSERT(m && (memory_size >= write_size));

		bool success = false;

		{
			memory_lock guard(m);

			auto memory_location = guard.ptr();

			if (memory_location)
			{
				success = (std::memcpy((reinterpret_cast<std::uint8_t*>(memory_location) + offset), data, size) != nullptr);
			}
		}

		if (success)
		{
			return m.clipboard_submit(format::TEXT);
		}

		return false;
	}

	bool clipboard::open(const window& owner)
	{
		// Check if we're already open, before anything else:
		if (is_open())
			return true;

		#ifdef CLIP_PLATFORM_WINDOWS
			if (OpenClipboard(owner))
			{
				this->access = true;
			}
		#endif
		
		return is_open();
	}
	
	bool clipboard::close(const window& owner)
	{
		// Check if we're already closed, before anything else:
		if (is_closed())
			return true;

		#ifdef CLIP_PLATFORM_WINDOWS
			if (CloseClipboard()) // wnd;
			{
				this->access = false;
			}
		#endif
		
		return is_closed();
	}

	memory clipboard::context(format type) const
	{
		ASSERT(is_open());

		return memory::open_clipboard(type);
	}

	bool clipboard::log(const path_t& file_path, bool append) const
	{
		// Check if we have a handle to the clipboard, if not, immediately fail:
		if (is_closed())
			return false;
		
		std::fstream fs;

		auto flags = (std::ios_base::out | std::ios_base::binary);

		if (append)
			flags |= std::fstream::app;

		fs.open(file_path, flags);

		if (!fs.is_open())
			return false;

		fs << *this;

		fs.close();

		return true;
	}

	bool clipboard::clear()
	{
		// Check if we have a handle to the clipboard, and if not, immediately fail:
		if (is_closed())
			return false;

		#ifdef CLIP_PLATFORM_WINDOWS
			return EmptyClipboard();
		#endif

		return false;
	}

	std::size_t clipboard::size() const
	{
		std::size_t total_size = 0;

		enumerate
		(
			[&](format type)
			{
				total_size += size(type);

				return true;
			}
		);

		return total_size;
	}

	std::size_t clipboard::size(format type) const
	{
		// Check if we have a handle to the clipboard, and if not, immediately return:
		if (is_closed())
			return 0;
		
		return context(type).size();
	}

	std::size_t clipboard::text_length() const
	{
		// Check if we have a handle to the clipboard, and if not, immediately return:
		if (is_closed())
			return 0;

		return context(format::TEXT).text_length();
	}
}