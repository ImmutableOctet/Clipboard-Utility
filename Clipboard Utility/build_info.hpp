#pragma once

// Debugging related:
#ifndef NDEBUG
	#define CLIP_DEBUG
#else
	#define CLIP_RELEASE
#endif

#ifdef _WIN32
	#define _CLIP_WIN32
#endif