#include <pebble.h>

//#define LOG_ENABLED

#ifdef LOG_ENABLED
#define APP_I_LOG APP_LOG
#else
#define APP_I_LOG(level, fmt, args...)
#endif
