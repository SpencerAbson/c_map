#ifndef MAP_SEED_H_
#define MAP_SEED_H_

#define HASHMAP_STATIC_SEED 26664310u

#include <stdint.h>
#include <time.h>

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#if defined(_WIN32)
#include <windows.h>
#endif

volatile uint32_t hashmap_seed = 0;  // volatile for thread-safe expansion


/* seed generation from processID and time - jannsson */
static int pid_seed_generate(uint32_t *seed) {
#ifdef HAVE_GETTIMEOFDAY
    /* XOR of seconds and microseconds */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    *seed = (uint32_t)tv.tv_sec ^ (uint32_t)tv.tv_usec;
#else
    /* Seconds only */
    *seed = (uint32_t)time(NULL);
#endif

    /* XOR with PID for more randomness */
#if defined(_WIN32)
    *seed ^= (uint32_t)GetCurrentProcessId();
#elif defined(HAVE_GETPID)
    *seed ^= (uint32_t)getpid();
#endif

    return 0;
}


static uint32_t _map_generate_new_seed()
{
    uint32_t seed = 0;
    pid_seed_generate(&seed);

    if(seed == 0)
        seed = 1;

    return seed;
}


void hashmap_ensure_seed()
{
    if(hashmap_seed == 0)
    {
#ifdef HASHMAP_USE_STATIC_SEED
        hashmap_seed = HASHMAP_STATIC_SEED;
#else
        hashmap_seed = _map_generate_new_seed();
#endif
    }
}


#endif // MAP_SEED_H_
