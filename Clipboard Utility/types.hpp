#pragma once

#include "build_info.hpp"

#include <string>

// Unfortunately, you can't forward declare the STL:
/*
namespace std
{
	class string;
}
*/

namespace clip
{
	using path_t = std::string;
}