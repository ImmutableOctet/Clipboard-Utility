#pragma once

#include "build_info.hpp"

#ifdef CLIP_DEBUG
	#include <iostream>

	#define DEBUG_ASSERT(condition, message) \
		do { \
			if (!(condition)) \
			{ \
				std::cerr \
						<< "ASSERTION `" #condition "` FAILED IN FILE: \"" << __FILE__ \
						<< "\" @ LINE #" << __LINE__ << ": " << message << std::endl; \
				std::terminate(); \
			} \
		} while (false)

	#define ASSERT(condition) \
		do { \
			if (!(condition)) \
				std::terminate(); \
		} while (false)
#else
	#define DEBUG_ASSERT(condition, message) do { } while (false)
	#define ASSERT(condition) do { } while (false)
#endif