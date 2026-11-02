/**
 * @file misc.c
 * @brief helpers of the diffusion sample: argument parsing, pointer swap, and timer
 *
 * @author Takashi Shimokawabe
 * @date 2010/12/09 Created
 * @version 0.1.0
 *
 * $Id: misc.c,v 7a382b862cde 2011/02/22 13:30:31 shimokawabe $
 */

#include "misc.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

static time_t sec_org = 0;
static time_t usec_org = 0;


int read_int(const char *arg, long min, long max, int *value)
{
    char *end;
    errno = 0;
    const long val = strtol(arg, &end, 10);
    if ((end == arg) || (*end != '\0') || (errno == ERANGE) || (val < min) || (val > max)) {
        return 1;
    }
    *value = (int)val;
    return 0;
}

void swap(float **f, float **fn)
{
    float *tmp;
    tmp = *f;
    *f = *fn;
    *fn = tmp;
}

void start_timer(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    sec_org = tv.tv_sec;
    usec_org = tv.tv_usec;
}

double get_elapsed_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (double)(tv.tv_sec - sec_org)
        + (double)(tv.tv_usec - usec_org)*1.0e-6;
}


