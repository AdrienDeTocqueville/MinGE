#pragma once

#ifdef PROFILE
# define IF_PROFILE(x) x

# define MICROPROFILE_ENABLED	 1
# define MICROPROFILE_WEBSERVER	 0
# define MICROPROFILE_UI	 1
# define MICROPROFILE_CONTEXT_SWITCH_TRACE	0
//# define MICROPROFILE_WEBSERVER_MAXFRAMES	10
//# define MICROPROFILE_MAX_FRAME_HISTORY	(2<<10)
#else
# define IF_PROFILE(x)
# define MICROPROFILE_ENABLED 0
#endif

#include "Profiler/microprofile.h"
#include "Profiler/microprofileui.h"
#include "Profiler/microprofiledraw.h"
