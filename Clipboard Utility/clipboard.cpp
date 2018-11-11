#include "clipboard.hpp"

#include <string>
#include <cassert>
#include <exception>

#include <iostream>
#include <fstream>

namespace clip
{
	clipboard::clipboard(const window& wnd)
		: owner(wnd)
	{
		#ifndef CLIP_PLATFORM_WINDOWS
			DEBUG_ASSERT(false, "Unsupported platform.")
		#endif
		
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