/*
 * sysinfo.h		- get kernel info
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

/* $Id: sysinfo.h,v 1.4 1993/02/12 11:40:16 gabor Exp $ */
 
#ifndef SYSINFO_INCLUDED
#define SYSINFO_INCLUDED

#define MAX_SWAPFILES 8

struct load {
	unsigned long total;
	unsigned long user;
	unsigned long system;
	unsigned long nice;
	unsigned long idle;
	char *cpu;
};

struct meminfo {
	int total;
	int cache;
	int buffers;
	int free;
	int shared;
};

extern void get_meminfo( int *, struct meminfo * );
extern double get_loadavg(void);
extern void get_load(struct load *);
extern int sysinfo_init(void);
extern int get_total_procs( void );

#endif
