#include <iostream>
#include <cassert>

#include <cstdlib>

#include "cliputil.hpp"

int main()
{
	using namespace clip;

	assert(CLIP_PLATFORM == platform::Windows);

	clip::clipboard c(clip::anonymous_window);

	DEBUG_ASSERT(c.is_open(), "Clipboard not available.");

	c.log("output/clipboard.txt");
	c.clear();

	std::cout << "Operations complete; exiting..." << std::endl;

	std::system("PAUSE");

	return 0;
}