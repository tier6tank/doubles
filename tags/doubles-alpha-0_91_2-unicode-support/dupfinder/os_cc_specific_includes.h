
#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>

#endif /* _Win32 */

/* perhaps better if !defined(_WIN32) && defined(__GNUC__) ???? */
#ifndef _WIN32

#include <sys/stat.h>
#include <fts.h>
#include <cerrno>
#include <stdarg.h>

#endif /* !defined(_WIN32) */
