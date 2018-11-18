#include <iostream>
#include <cstdlib>

#include "cliputil.hpp"
#include "test.hpp"

static_assert(CLIP_PLATFORM == CLIP_PLATFORM_WINDOWS, "Please build using Windows as your target platform.");

int main()
{
	using namespace clip;

	bool execute_tests = true;

	if (execute_tests)
	{
		unit_test::feature_test();
	}
	else
	{
		// ...
	}

	// Tell the user we're done.
	std::cout << "Operations complete; exiting..." << std::endl;

	// Pause the application.
	std::system("PAUSE");

	return 0;
}