#include <iostream>
#include <cstdlib>

#include "cliputil.hpp"

int main()
{
	using namespace clip;

	static_assert(CLIP_PLATFORM == CLIP_PLATFORM_WINDOWS, "Please build using Windows as your target platform.");

	{
		clip::clipboard c(clip::anonymous_window);

		DEBUG_ASSERT(c.is_open(), "Clipboard not available.");

		// Log the clipboard to a text file.
		c.log("output/clipboard.txt");

		auto clipboard_segments = c.count();

		std::cout << "Entries found: " << clipboard_segments << "\n";

		if (true) // c.has_text()
		{
			std::cout << "Text segment:" << "\n" << c << "\n\n";
		}

		// Clear the clipboard's contents.
		c.clear();
	}

	// Tell the user we're done.
	std::cout << "Operations complete; exiting..." << std::endl;

	// Pause the application.
	std::system("PAUSE");

	return 0;
}