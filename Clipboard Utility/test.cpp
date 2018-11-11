#include "types.hpp"

#include <iostream>

namespace clip
{
	bool test(bool condition, const std::string& when_true, const std::string& when_false)
	{
		const auto endl = '\n';

		if (condition)
			std::cout << when_true << endl;
		else
			std::cout << when_false << endl;

		return condition;
	}
}