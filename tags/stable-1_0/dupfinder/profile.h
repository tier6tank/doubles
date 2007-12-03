
#ifdef PROFILE
LARGE_INTEGER __frequency;
#endif

#if defined(_WIN32) && defined(PROFILE)

#define INITPROFILE QueryPerformanceFrequency(&__frequency);

#define STARTTIME(a) { LARGE_INTEGER __b1, __b2; QueryPerformanceCounter(&__b1); 

#define STOPTIME(a) QueryPerformanceCounter(&__b2);  (a).QuadPart += __b2.QuadPart - __b1.QuadPart; }

#define SECONDS(a)  (todouble((a).QuadPart)/__frequency.QuadPart)


#else /* defined(_WIN32) && defined(PROFILE) */

#define INITPROFILE 

#define STARTTIME(a) {

#define STOPTIME(a) }

#define SECONDS(a) 1

#endif /* defined(_WIN32) && defined(PROFILE) */
