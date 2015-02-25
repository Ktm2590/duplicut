#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include "definitions.h"
#include "config.h"
#include "memstate.h"
#include "chunk.h"
#include "line.h"
#include "filehandle.h"
#include "error.h"
#include "debug.h"


/** Configure how many threads to use.
 *      --> g_conf.threads
 */
static void     config_threads(void)
{
    long        max_threads;

    max_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (max_threads < 0)
        error("sysconf(_SC_NPROCESSORS_ONLN): %s", ERRNO);
    else if (max_threads == 0)
        die("sysconf(_SC_NPROCESSORS_ONLN): Unexpected return: 0");

    if (g_conf.threads == 0)
        g_conf.threads = max_threads;
    else if (g_conf.threads > max_threads)
        error("Cannot use more than %ld threads", max_threads);
}


/** Return the nearest prime number <= `n`.
 * Used in order to settle hmap size with a prime value.
 * It ensures an optimal hash repartition.
 */
static long     get_prev_prime(long n)
{
    int         i;

    n = (n - 1) | 1;
    while (n > 0)
    {
        i = 3;
        while (i && i <= sqrt(n))
        {
            if (n % i == 0)
                i = 0;
            else
                i += 2;
        }
        if (i)
            return (n);
        n -= 2;
    }
    return (n);
}


/** Configure hmap_size.
 * This values needs to know file size and current memstate
 * in order to determine an optimal hash map size.
 * It also uses a prime number as final value, thanks to get_prev_prime().
 */
static void     config_hmap_size(t_file *file, struct memstate *memstate)
{
    long        max_size;
    long        hmap_size;

    hmap_size = file->info.st_size / MEDIUM_LINE_BYTES;

    max_size = (memstate->mem_available * HMAP_MAX_SIZE) / sizeof(t_line);
    if (hmap_size > max_size)
        hmap_size = max_size;

    g_conf.hmap_size = get_prev_prime(hmap_size);
}


/** Configure `g_conf` vars after argument parsing
 */
void            configure(void)
{
    struct memstate memstate;

    init_memstate(&memstate);

    config_threads();
    config_hmap_size(g_file, &memstate);

    DLOG("--------configure()-----------");
    DLOG("------------------------------");
    DLOG("conf->infile_name:   %s", g_conf.infile_name);
    DLOG("conf->outfile_name:  %s", g_conf.outfile_name);
    DLOG("conf->threads:       %u", g_conf.threads);
    DLOG("conf->line_max_size: %u", g_conf.line_max_size);
    DLOG("conf->page_size:     %d", g_conf.page_size);
    DLOG("conf->hmap_size:     %ld", g_conf.hmap_size);
    DLOG("conf->chunk_size:    %ld", g_conf.chunk_size);
    DLOG("------------------------------");
}
