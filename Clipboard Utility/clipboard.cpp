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

		auto m = memory::open(format::TEXT);

		// Check if we could open the memory segment:
		if (m)
		{
			// Lock the memory segment, so that we're able to read from it.
			memory_lock guard(m);

			// Retrieve a raw pointer to the memory segment:
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

	bool clipboard::open(const window& owner)
	{
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
		#ifdef CLIP_PLATFORM_WINDOWS
			if (CloseClipboard()) // wnd;
			{
				this->access = false;
			}
		#endif
		
		return is_closed();
	}

	bool clipboard::log(const path_t& file_path, bool append) const
	{
		// Check if we have a handle to the clipboard, and if not, immediately fail:
		if (!is_open())
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
		if (!is_open())
			return false;

		#ifdef CLIP_PLATFORM_WINDOWS
			return EmptyClipboard();
		#endif

		return false;
	}
}