/**
 * @file misc.h
 * @brief declarations of the helpers of the diffusion sample: argument parsing, pointer swap, and timer
 *
 * @author Takashi Shimokawabe
 * @date 2010/12/09 Created
 * @version 0.1.0
 *
 * $Id: misc.h,v 7a382b862cde 2011/02/22 13:30:31 shimokawabe $
 */

#ifndef MISC_H
#define MISC_H


/* reads a decimal integer in [min, max] from arg into value; returns 0, or 1 (value unchanged) for another string */
int read_int(const char *arg, long min, long max, int *value);
void swap(float **f, float **fn);
void start_timer(void);
double get_elapsed_time(void);


#endif /* MISC_H */


