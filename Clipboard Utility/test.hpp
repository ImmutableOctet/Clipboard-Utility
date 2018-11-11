#pragma once

#include "types.hpp"

namespace clip
{
	template <typename T, typename F>
	inline bool test(bool condition, const T& when_true, const F& when_false)
	{
		if (condition)
			when_true();
		else
			when_false();

		return condition;
	}

	bool test(bool condition, const std::string& when_true, const std::string& when_false);
}