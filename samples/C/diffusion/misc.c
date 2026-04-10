/**
 * @file misc.c
 * @brief (File brief)
 *
 * (File explanation)
 *
 * @author Takashi Shimokawabe
 * @date 2010/12/09 Created
 * @version 0.1.0
 *
 * $Id: misc.c,v 7a382b862cde 2011/02/22 13:30:31 shimokawabe $
 */

#include "misc.h"
#include <stdio.h>
#include <sys/time.h>

static time_t sec_org = 0;
static time_t usec_org = 0;


void swap(float **f, float **fn)
{
    float *tmp;
    tmp = *f;
    *f = *fn;
    *fn = tmp;
}

void start_timer()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    sec_org = tv.tv_sec;
    usec_org = tv.tv_usec;
}

double get_elapsed_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (double)(tv.tv_sec - sec_org)
        + (double)(tv.tv_usec - usec_org)*1.0e-6;
}


