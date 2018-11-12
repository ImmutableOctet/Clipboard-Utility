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

	// May be changed into a template at a later date.
	std::string clipboard::read_text() const
	{
		using T = std::string;

		static_assert(std::is_default_constructible_v<T>, "Specified type must be default-constructible.");
		static_assert(std::is_constructible_v<T, const char*>, "Specified type must be constructible using 'const char*' as a parameter.");

		ASSERT(is_open());

		memory m = memory::open(format::TEXT);

		// Check if we could open the handle.
		if (m)
		{
			memory_lock guard(m);

			auto raw_data = guard.ptr();

			if (raw_data)
			{
				// ATTENTION: Not memory-safe; if an exception is thrown, we're leaving a handle locked.
				auto out = T(raw_data);

				return out;
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