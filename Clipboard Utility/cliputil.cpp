#include <iostream>
#include <cstdlib>

#include "cliputil.hpp"

int main()
{
	using namespace clip;

	static_assert(CLIP_PLATFORM == CLIP_PLATFORM_WINDOWS, "Please build using Windows as your target platform.");
	
	const auto iterations = 2;
	const auto test_seperator_length = 16;

	for (auto i = 1; i <= iterations; i++)
	{
		std::cout << "Running test suite... (#" << i << ")\n\n";

		
		std::cout << "Opening handle to clipboard.\n";
		
		clip::clipboard c(clip::anonymous_window);

		DEBUG_ASSERT(c.is_open(), "Clipboard not available.");

		
		std::cout << "\nCounting clipboard segments...\n\n";
		
		auto clipboard_segments = c.count();

		std::cout << "Entries found: " << clipboard_segments << "\n";
		std::cout << "Total size: " << c.size() << " bytes\n";

		std::cout << "\nLooking for TEXT segment...\n";

		if (c)
		{
			if (c.has_text())
			{
				std::cout << "\nText currently in clipboard:" << "\n" << c << "\n";
				std::cout << "\nLength of TEXT segment: " << c.text_length() << "\n";
				std::cout << "\nLogging text to output file...\n";
				
				// Log the clipboard to a text file.
				c.log("output/clipboard.txt");
			}
			else
			{
				std::cout << "No TEXT segment detected.\n";
			}

			std::cout << "\nClearing clipboard contents...\n\n";

			// Clear the clipboard's contents.
			c.clear();

			//std::cout << "Writing new message to the clipboard...\n";
		}
		else
		{
			std::cout << "No segments found. No reason to clear clipboard.\n";
		}
		
		if (i < iterations)
		{
			std::cout << "Running tests again... (Tests remaining: " << (iterations - i) << ")\n" << std::endl;

			for (auto i = 1; i <= test_seperator_length; i++)
			{
				std::cout << '-';
			}

			std::cout << "\n" << std::endl;
		}
	}

	for (auto i = 1; i <= 4; i++)
		std::cout << '\n';

	// Tell the user we're done.
	std::cout << "Operations complete; exiting..." << std::endl;

	// Pause the application.
	std::system("PAUSE");

	return 0;
}