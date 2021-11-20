
/*
 * sysinfo.c		- get kernel info
 *
 */

/* 
 * Written by Gabor Herr <herr@iti.informatik.th-darmstadt.de>.
 *
 * Copyright (c) 1992, 1993 by Gabor Herr, all rights reserved.
 * 
 * Permission to use, copy, modify, distribute, and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear in
 * supporting documentation, and that may name is not used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission. I make no representations
 * about the suitability of this software for any purpose. It is
 * provided "as is" without express or implied warranty.
 */

/* $Id: sysinfo.c,v 1.3 1993/02/12 11:40:16 gabor Exp $ */
 
#include "sysinfo.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

static int meminfo_fd;
static int load_fd;
static int loadavg_fd;
static int swaps_fd;

static char buffer[1024];

static void reread( int fd, char *message )
{
	if ( lseek( fd, 0L, 0 ) == 0 &&
	     read( fd, buffer, sizeof(buffer) - 1 ) > 0 )
		return;
       
	perror(message);
	exit(1);
}

static int getentry(const char *tag, const char *bufptr) {
	char *tail;
	int retval, len = strlen(tag);
  
	while (bufptr) {
		if (*bufptr == '\n') bufptr++;
		if (!strncmp(tag, bufptr, len)) {
			retval = strtol(bufptr + len, &tail, 10);
			if (tail == bufptr + len) {
				return -1;
			} else {
				return retval;
			}
		}
		bufptr = strchr( bufptr, '\n' );
	}
	return -1;
}

void get_meminfo( int *swapdevs, struct meminfo *result )
{
	int i;
	char *bufptr;
	int res;
	int sw_size, sw_used;

	reread( meminfo_fd, "get_meminfo");

	/* Try new /proc/meminfo format first */
	*swapdevs = 1;
	if ((result[0].total   = getentry("MemTotal:", buffer))  < 0 ||
	    (result[0].free    = getentry("MemFree:", buffer))   < 0 ||
	    (result[0].shared  = getentry("MemShared:", buffer)) < 0 ||
	    (result[0].buffers = getentry("Buffers:", buffer))   < 0 ||
	    (result[0].cache   = getentry("Cached:", buffer))    < 0 ||
	    (result[1].total   = getentry("SwapTotal:", buffer)) < 0 ||
	    (result[1].free    = getentry("SwapFree:", buffer))  < 0) {

		/* Fall back to old format */
		bufptr = strchr( buffer, '\n' ) + 1;
		sscanf( bufptr, "Mem: %d %*d %d %d %d",
			&result[0].total,
			&result[0].free,
			&result[0].shared,
			&result[0].cache );
		result[0].buffers = -1;
    
		for ( i = 1; i < MAX_SWAPFILES; i++ ) {

			bufptr = strchr( bufptr, '\n' ) + 1;

			if ( *bufptr == '\0' ||
			     (res = sscanf( bufptr, "Swap: %d %*d %d",
					    &result[i].total,
					    &result[i].free )) != 2 )
				break; 

		}

		*swapdevs = i - 1;  
	} else if (swaps_fd > 0) {

		reread( swaps_fd, "get_meminfo");
		bufptr = buffer;

		for ( i = 1; i < MAX_SWAPFILES; i++ ) {

			bufptr = strchr( bufptr, '\n' ) + 1;
			if ( *bufptr == '\0' ||
			     (res = sscanf( bufptr, "%*s %*s %d %d",
					    &sw_size, &sw_used)) != 2 )
				break;

			result[i].total = sw_size;
			result[i].free = sw_size - sw_used;
		}

		*swapdevs = i - 1;  
	}
}

double get_loadavg( void )
{
	double load;

	reread( loadavg_fd, "get_loadavg");

	sscanf( buffer, "%lf", &load );

	return load;
}

void get_load(struct load * result)
{
	static int initialized = 0;
	static struct load *last_load;
	struct load *loadptr;
	struct load curr_load;
	int procs, i, index;
	char *bufptr;

	if(!initialized) {
		procs = get_total_procs();
		last_load = (struct load *)malloc(procs * sizeof(struct load));
		loadptr = last_load;
		for( i = 0; i < procs; i++) {
			loadptr->total = 0;
			loadptr->user = 0;
			loadptr->system = 0;
			loadptr->nice = 0;
			loadptr->idle = 0;
			loadptr->cpu = NULL;
			loadptr++;
		}
		initialized = 1;
	}

	reread( load_fd, "get_load" );
	bufptr = buffer;

	if(sscanf(result->cpu, "cpu%d", &index) != 1)
		loadptr = last_load;
	else
		loadptr = last_load + index;

	bufptr = strstr(bufptr, result->cpu);
	sscanf( bufptr, "%*s %lu %lu %lu %lu\n", &curr_load.user,
		&curr_load.nice, &curr_load.system, &curr_load.idle );
	curr_load.total = curr_load.user + curr_load.nice + curr_load.system +
		curr_load.idle;

	result->total  = curr_load.total  - loadptr->total;
	result->user   = curr_load.user   - loadptr->user;
	result->nice   = curr_load.nice   - loadptr->nice;
	result->system = curr_load.system - loadptr->system;
	result->idle   = curr_load.idle   - loadptr->idle;
	loadptr->total  = curr_load.total;
	loadptr->user   = curr_load.user;
	loadptr->nice   = curr_load.nice;
	loadptr->system = curr_load.system;
	loadptr->idle   = curr_load.idle;
}

int sysinfo_init( void )
{
	if ((meminfo_fd = open("/proc/meminfo",O_RDONLY)) < 0) {
		perror("/proc/meminfo");
		return 1;
	}
	if ((loadavg_fd = open("/proc/loadavg",O_RDONLY)) < 0) {
		perror("/proc/loadavg");
		return 1;
	}
	if ((load_fd = open("/proc/stat",O_RDONLY)) < 0) {
		perror("/proc/stat");
		return 1;
	}
	/* Failing here is not critical. We can get the info from meminfo. */
	swaps_fd = open("/proc/swaps",O_RDONLY);

	return 0;
}

int get_total_procs( void )
{
	int count = 1;
	char cpu_num[7];
	char *bufptr;

	reread( load_fd, "get_total_procs" );

	while(1)
	{
		bufptr = buffer;
		sprintf(cpu_num,"cpu%d ", count);
		if(strstr(bufptr, cpu_num) != NULL)
			count++;
		else
			return count;
	}

	return -1;
}
