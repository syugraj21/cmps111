/* Justin Kwok, Ian Hamilton, Yugraj Singh, Ben Lieu, Greg Arnheiter */
/* Created 5/20/14 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

void *slug_malloc(size_t size, char *WHERE);

void slug_free(void *addr, char *WHERE);

void slug_memstats(void);