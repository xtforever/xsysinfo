
#ifndef _XgBarGauge_h
#define _XgBarGauge_h

/***********************************************************************
 *
 * BarGauge: Widget to periodically display a value in form of a horizontal bar.
 * based on Athena StripChart Widget.
 *
 * Author: Gabor Herr		herr@iti.informatik.th-darmstadt.de
 *
 * $Id: BarGauge.h,v 1.3 1993/02/07 23:54:15 gabor Exp $
 *
 ***********************************************************************/

/***********************************************************
Copyright 1992, 1993 by Gabor Herr
Copyright 1987, 1988 by Digital Equipment Corporation, Maynard, Massachusetts,
and the Massachusetts Institute of Technology, Cambridge, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the names of Gabor Herr or Digital or MIT not
be used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

GABOR HERR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
GABOR HERR BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/

/* 

 Name		     Class		RepType		Default Value
 ----		     -----		-------		-------------

 BarGauge resources:

 autoAdjust	     AutoAdjust		Boolean		False
   If False, displays each value returned by getValues procedure in a
   separate segment. If True, only the first value returned by getValues
   is displayed in n segments, where n is value/maxValue; the first n-1
   segments are of length maxValue; the n'th segment is of
   length value%maxValue; the bar's total length is n*maxValue.
 
 segments	     Segments		int		1
   Total number of segments the bar is subdivided.
 
 segmentGap	     SegmentGap		int		2
   Gap between adjacent segments in pixel.
 
 foreground	     Foreground		Pixel		XtDefaultForeground
 foreground1	     Foreground		Pixel		XtDefaultForeground
 foreground2	     Foreground		Pixel		XtDefaultForeground
 foreground3	     Foreground		Pixel		XtDefaultForeground
 foreground4	     Foreground		Pixel		XtDefaultForeground
 foreground5	     Foreground		Pixel		XtDefaultForeground
 foreground6	     Foreground		Pixel		XtDefaultForeground
 foreground7	     Foreground		Pixel		XtDefaultForeground
   Foreground color of each of the max. 8 segments.
 
 getValue	     Callback		XtCallbackList	NULL
   Callback procedure to be called to obtain the actual segment values. The
   call_data parameter is a pointer to XgBarGaugeValues.
 
 maxValue	     MaxValue		float		1.0
   The bar's full length. I.e. a segment with this value will completely fill
   the gauge.
 
 update		     Interval		int		10 (seconds)
   Time between two calls of the getValue procedure.

 
 Inherited resources:
 
 accelerators	     Accelerators	AcceleratorTable NULL
 ancestorSensitive   AncestorSensitive	Boolean		True 
 background	     Background		Pixel		XtDefaultBackground
 backgroundPixmap    Pixmap		Pixmap		XtUnspecifiedPixmap
 borderColor	     BorderColor	Pixel		XtDefaultForeground
 borderPixmap	     Pixmap		Pixmap		XtUnspecifiedPixmap
 borderWidth	     BorderWidth	Dimension	1
 colormap	     Colormap		Colormap	parent's colormap
 cursor		     Cursor		Cursor		None
 cursorName	     Cursor		String		NULL
 depth		     Depth		int		parent's depth
 destroyCallback     Callback		XtCallbackList	NULL
 height		     Height		Dimension	120
 insensitiveBorder   Insensitive	Pixmap		GreyPixmap
 mappedWhenManaged   MappedWhenManaged	Boolean		True
 pointerColor	     Foreground		Pixel		XtDefaultForeground
 pointerColorBackground Background	Pixel		XtDefaultBackground
 screen		     Screen		Screen		parent's screen
 sensitive	     Sensitive		Boolean		True
 translations	     Translations	TranslationTable NULL
 width		     Width		Dimension	120
 x		     Position		Position	0
 y		     Position		Position	0

*/

#ifndef _XtStringDefs_h_
#define XtNupdate "update"
#endif

#define XtNautoAdjust "autoAdjust"
#define XtCAutoAdjust "AutoAdjust"

#define XtNgetValue "getValue"

#define XtNsegments "segments"
#define XtCSegments "Segments"

#define XtNsegmentGap "segmentGap"
#define XtCSegmentGap "SegmentGap"

#define XtNmaxValue "maxValue"
#define XtCMaxValue "MaxValue"

#define XtNforeground1 "foreground1"
#define XtNforeground2 "foreground2"
#define XtNforeground3 "foreground3"
#define XtNforeground4 "foreground4"
#define XtNforeground5 "foreground5"
#define XtNforeground6 "foreground6"
#define XtNforeground7 "foreground7"

/* call data param for getValue */
typedef struct {
	Cardinal num_values;		/* # of values */
	double *values;		/* values[i] = value for i-th bar segment */
} XgBarGaugeValues;

typedef struct _BarGaugeRec *BarGaugeWidget;
typedef struct _BarGaugeClassRec *BarGaugeWidgetClass;

extern WidgetClass barGaugeWidgetClass;

#endif /* _XgBarGauge_h */
