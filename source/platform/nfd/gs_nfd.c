#include "platform/gs_platform.h"
#include "third_party/include/nfd/nfd.h"

#if ( defined GS_PLATFORM_WIN )

	#include "third_party/source/nfd/nfd_common.c"
	#include "third_party/source/nfd/nfd_win.cpp"

#elif ( defined GS_PLATFORM_APPLE )



#elif ( defined GS_PLATFORM_LINUX )



#endif