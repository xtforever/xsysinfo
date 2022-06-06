/*
 * xsysinfo: X application for displaying some kernel parameters
 *
 * $Id: xsysinfo.c,v 1.9 1993/02/12 11:40:16 gabor Exp $
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

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include "BarGauge.h"
#include <stdio.h>
#include <stdlib.h>
#include "sysinfo.h"

#define norm(x,max)	((max) == 0 ? 0 : (double)(x) / (max))

/* Application Ressources: */
typedef struct {
	Boolean show_title;
	Boolean show_labels;
	Boolean show_loadavg;
	Boolean show_load;
	Boolean show_mem;
	Boolean show_swap;
	Boolean smp_kernel;
} AppData, *AppDataPtr;

static AppData appres;

static XtResource application_resources[] = {
	{ "showTitle", "ShowTitle", XtRBoolean, sizeof(Boolean),
	  XtOffset(AppDataPtr,show_title), XtRImmediate, (XtPointer) True },
	{ "showLabels", "ShowLabels", XtRBoolean, sizeof(Boolean),
	  XtOffset(AppDataPtr,show_labels), XtRImmediate, (XtPointer) True },
	{ "showLoadAvg", "ShowLoadAvg", XtRBoolean, sizeof(Boolean),
	  XtOffset(AppDataPtr,show_loadavg), XtRImmediate, (XtPointer) True },
	{ "showLoad", "ShowLoad", XtRBoolean, sizeof(Boolean),
	  XtOffset(AppDataPtr,show_load), XtRImmediate, (XtPointer) True },
	{ "showMem", "ShowMem", XtRBoolean, sizeof(Boolean),
	  XtOffset(AppDataPtr,show_mem), XtRImmediate, (XtPointer) True },
	{ "showSwap", "ShowSwap", XtRBoolean, sizeof(Boolean),
	  XtOffset(AppDataPtr,show_swap), XtRImmediate, (XtPointer) True },
	{ "smpKernel", "SmpKernel", XtRBoolean, sizeof(Boolean),
	  XtOffset(AppDataPtr,smp_kernel), XtRImmediate, (XtPointer) True }
};

/* Command line options */
static XrmOptionDescRec options[] = {

	{ "-update", "*.bar.update", XrmoptionSepArg, NULL },

	{ "-title",   "*showTitle", XrmoptionNoArg, (XtPointer) "True" },
	{ "-notitle", "*showTitle", XrmoptionNoArg, (XtPointer) "False" },

	{ "-labels",   "*showLabels", XrmoptionNoArg, (XtPointer) "True" },
	{ "-nolabels", "*showLabels", XrmoptionNoArg, (XtPointer) "False" },

	{ "-loadavg",   "*showLoadAvg", XrmoptionNoArg, (XtPointer) "True" },
	{ "-noloadavg", "*showLoadAvg", XrmoptionNoArg, (XtPointer) "False" },

	{ "-load",   "*showLoad", XrmoptionNoArg, "True" },
	{ "-noload", "*showLoad", XrmoptionNoArg, "False" },

	{ "-mem",   "*showMem", XrmoptionNoArg, (XtPointer) "True" },
	{ "-nomem", "*showMem", XrmoptionNoArg, (XtPointer) "False" },

	{ "-swap",   "*showSwap", XrmoptionNoArg, (XtPointer) "True" },
	{ "-noswap", "*showSwap", XrmoptionNoArg, (XtPointer) "False" },

	{ "-smp",   "*smpKernel", XrmoptionNoArg, (XtPointer) "True" },
	{ "-nosmp", "*smpKernel", XrmoptionNoArg, (XtPointer) "False" }
};

static String fallback_resources[] = {
	"*.title.label:		Linux System Info",
	"*.loadavg*label:	CPU Load Avg:",
	"*.load*label:		CPU Load:",
	"*.mem*label:		Memory:  ",
	"*.swap*label:		Swap:    ",
	NULL
};

static XtAppContext context;
static struct meminfo swap[MAX_SWAPFILES];
static int swapfiles;
static int memupdate_freq = 0;

static void getloadavgCB(Widget widget,XtPointer client_data,XtPointer call_data)
{
	XgBarGaugeValues *gauge = (XgBarGaugeValues *) call_data;
	gauge->values[0] = get_loadavg();  
}

static void getloadCB( Widget widget,XtPointer client_data,XtPointer call_data)
{
	static char cpuLoadBuffer[15];
	struct load load;
	XgBarGaugeValues *gauge = (XgBarGaugeValues *) call_data;

	// 20070307 <tmancill@debian.org>
	// this char buffer is never freed
	//load.cpu = (char *)malloc(7*sizeof(char));
	load.cpu = cpuLoadBuffer;
	strncpy(load.cpu, (char *)client_data, 7);
	get_load(&load);

	gauge->values[0] = norm(load.user, load.total);
	gauge->values[1] = norm(load.system, load.total);
	gauge->values[2] = norm(load.nice, load.total);
}

static void update_meminfo(void)
{
	static int count = 0;

	if ( count-- > 0 )
		return;

	count = memupdate_freq;
  
	get_meminfo( &swapfiles, swap );
}

static void getmemCB( Widget widget, XtPointer client_data,XtPointer call_data)
{
	XgBarGaugeValues *gauge = (XgBarGaugeValues *) call_data;
	struct meminfo *mem = swap;

	update_meminfo();
  
	if (mem->buffers >= 0) {
		/* New style /proc/meminfo format */
    
		gauge->values[0] = norm(mem->total - mem->free - mem->cache -
					mem->buffers,mem->total);
		gauge->values[1] = norm(mem->cache + mem->buffers,mem->total);
	} else {
		/* Old style /proc/meminfo format (no buffers-entry) */
		gauge->values[0] = norm(mem->total - mem->free - mem->cache,
					mem->total);
		gauge->values[1] = norm(mem->cache,mem->total);
	}
}

static void getswapCB( Widget widget,XtPointer client_data,XtPointer call_data)
{
	XgBarGaugeValues *gauge = (XgBarGaugeValues *) call_data;
	int i;
	int total;

	update_meminfo();
  
	if ( gauge->num_values != swapfiles )
		XtVaSetValues( widget, XtNsegments, swapfiles, NULL);

	for ( total = i = 0; i < gauge->num_values; i++ ) 
		total += swap[i + 1].total;

	for ( i = 0; i < gauge->num_values; i++ ) 
		gauge->values[i] = norm(swap[i + 1].total - swap[i + 1].free,
					total);
}

static Widget create_gauge( 
			   String name,
			   Widget parent,
			   int segs,
			   XtCallbackProc valueproc,
			   XtPointer client_data)
{
	Widget gauge;
	Widget bar;
	Widget label;
  
	gauge = XtCreateManagedWidget( name, formWidgetClass, parent, NULL, 0 );
 
	if (appres.show_labels) {
		label = XtVaCreateManagedWidget( "name",labelWidgetClass,gauge,
						 XtNright, XawChainLeft,
						 NULL);

		bar = XtVaCreateManagedWidget( "bar",barGaugeWidgetClass,gauge,
					       XtNsegments, segs,
					       XtNfromHoriz, label,
					       XtNleft, XawChainLeft,
					       NULL);
	} else {
		bar = XtVaCreateManagedWidget( "bar",barGaugeWidgetClass,gauge,
					       XtNsegments, segs,
					       XtNleft, XawChainLeft,
					       NULL);
	}

	XtAddCallback( bar, XtNgetValue, valueproc, client_data );

	return bar;
}

static void create_app( Widget top )
{
	Widget form;
	Widget above_widget = NULL;
	Widget this;
	Widget bar;
	char *bufptr;
	int total_procs, i;
  
	form = XtCreateManagedWidget( "form", formWidgetClass, top, NULL, 0 );

	if (appres.show_title)
		above_widget = XtVaCreateManagedWidget( "title",
							labelWidgetClass, form,
							XtNright, XawChainRight,
							NULL);

	if (appres.show_loadavg)	{
		bar  = create_gauge( "loadavg", form, 1, getloadavgCB, NULL );
		XtVaSetValues( bar,
			       XtNautoAdjust, True,
			       NULL );
		this = XtParent(bar);
		XtVaSetValues( this,
			       XtNfromVert, above_widget,
			       XtNright,    XawChainRight,
			       NULL );
		above_widget = this;
	}
    
	if (appres.show_load) {
		total_procs = get_total_procs();
		if(!appres.smp_kernel || total_procs == 1) {
			bufptr = (char *)malloc(7*sizeof(char));
			sprintf(bufptr, "cpu ");
			this = XtParent(create_gauge( "load", form, 3, getloadCB, bufptr ));
			XtVaSetValues( this,
				       XtNfromVert, above_widget,
				       XtNright,    XawChainRight,
				       NULL );
			above_widget = this;
		}
		else {
			for( i = 0; i < total_procs; i ++) {
			        bufptr = (char *)malloc(15*sizeof(char));
				sprintf(bufptr, "cpu%d ", i);
				this = XtParent(create_gauge( "load", form, 3, getloadCB, bufptr ));
				XtVaSetValues( this,
					       XtNfromVert, above_widget,
					       XtNright,    XawChainRight,
					       NULL );
				above_widget = this;
			}
		}
	}
    
	if (appres.show_mem)	{		
		this = XtParent(create_gauge( "mem", form, 2, getmemCB, NULL ));
		XtVaSetValues( this,
			       XtNfromVert, above_widget,
			       XtNright,    XawChainRight,
			       NULL );
		above_widget = this;
		memupdate_freq++;
	}
    
	if (appres.show_swap && swapfiles > 0)	{		
		this = XtParent(create_gauge( "swap", form, swapfiles,
					      getswapCB, NULL ));
		XtVaSetValues( this,
			       XtNfromVert, above_widget,
			       XtNright,    XawChainRight,
			       NULL );
		above_widget = this;
		memupdate_freq++;
	}

	if (above_widget == NULL)
		XtAppError( context,"All gauges disabled, nothing to show.\n" );
}

static void show_usage(void)
{
	fprintf(stderr,"xsysinfo version 1.7\n\n");
	fprintf(stderr,"usage: xsysinfo [options...]\n\n");
	fprintf(stderr,"where options include:\n");
#ifdef USE_MILLI_SECONDS       /* (ThMO) */
	fprintf(stderr,"-update <n>                     set update rate to <n> milli-seconds\n");
#else
	fprintf(stderr,"-update <n>                     set update rate to <n> seconds\n");
#endif
	fprintf(stderr,"-title   (-notitle)		show (don't show) title string\n");
	fprintf(stderr,"-labels  (-nolabels)		show (don't show) gauge labels\n");
	fprintf(stderr,"-loadavg (-noloadavg)		show (don't show) CPU load average value\n");
	fprintf(stderr,"-load    (-noload)		show (don't show) CPU load value\n");
	fprintf(stderr,"-mem     (-nomem)		show (don't show) Memory info\n");
	fprintf(stderr,"-swap    (-noswap)		show (don't show) Swap info\n");
	fprintf(stderr,"-smp     (-nosmp)               smp (not smp) kernel\n");
	fprintf(stderr,"-help				show this message\n");
	fprintf(stderr,"\nor one of the standard toolkit options.\n\n");
}

int main( int argc, char **argv )
{
  //	Arg args[1];
	Widget toplevel;

	toplevel = XtAppInitialize( &context, "XSysinfo",
				    options, XtNumber(options),
				    &argc, argv,
				    fallback_resources,
				    NULL, 0 );

	if (argc != 1) {
		show_usage();
		exit(1);
	}
			      
	XtGetApplicationResources( toplevel, &appres,
				   application_resources,
				   XtNumber( application_resources ),
				   NULL, 0 );
			      
	if (sysinfo_init() != 0) {
		fprintf(stderr,	"Proc filesystem isn't mounted on `/proc'.\n");
		exit(1);
	}
	update_meminfo();

	create_app( toplevel );

	XtRealizeWidget(toplevel);
	XtAppMainLoop(context);
	return 0;
}
