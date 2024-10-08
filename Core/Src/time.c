#include <sys/time.h>

/* Dummy implementation */
int _gettimeofday( struct timeval * tv,
                   void * tz )
{
    if( tv )
    {
        tv->tv_sec = 0;  /* Set to current time in seconds */
        tv->tv_usec = 0; /* Set to current time in microseconds */
    }

    return 0;
}
