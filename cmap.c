
/*      cmap.c       */

#include "stdlib.h"
#include "stdio.h"

#include <Xm/DrawingA.h>
#include <Xm/Form.h>

#include "color.h"
#include "rgb.h"
#include "log.h"
#include "cmap.h"

#include "time.h"

extern Display *theDisplay;
extern Visual *theVisual;
extern Colormap theColormap;
extern Screen *theScreen;
extern GC theGC;
extern Pixel thePixel;

static Pixel theFreeCells [256];
static int numFreeCells = 0;
static int verbose = 1;


void logVerbose (int on) {
    verbose = on;
}


static void freeCell (Pixel pix) {
    XFreeColors (theDisplay, theColormap, &pix, 1, 0);
}


/* currently unused */
Boolean allocCell (Pixel pix) {
    unsigned long plane_mask[1];
    unsigned long pixels_return[256];
    unsigned cells, numCells, found;
    Pixel freePix;

    found = FALSE;
    cells = 1;
    numCells = 0;
    while ((cells != 0) && (!found)) {
        cells =
            XAllocColorCells(theDisplay, theColormap, TRUE, plane_mask,
                             0, &pixels_return[numCells], 1);
        if (cells) {
          if (pix == pixels_return[numCells])
              found = TRUE;
          numCells++;
        }
    }
    for (freePix=0; freePix<numCells; freePix++) {
      if (freePix != pix)
        freeCell (pixels_return[freePix]);
    }

    return found;
}

Boolean setCell (int r, int g, int b) {
    XColor xcolor;
    int cells, numCells;
    unsigned long plane_mask[1];
    unsigned long pixels_return[256];

    xcolor.pixel = thePixel;
    xcolor.flags = DoRed | DoGreen | DoBlue ;
    xcolor.red = r * 256;
    xcolor.green = g * 256;
    xcolor.blue = b * 256;

    numCells = 0;
    cells = 1;
    while (cells != 0) {
      cells =
        XAllocColorCells(theDisplay, theColormap, TRUE, plane_mask,
                    0, &pixels_return[numCells], 1);
      if (cells) {
        if (thePixel == pixels_return[numCells])
            XStoreColor (theDisplay, theColormap, &xcolor);
        numCells++;
      }
    }
    XFreeColors (theDisplay, theColormap, pixels_return, numCells, 0);
    return TRUE;
}

static void countFreeCells () {
    unsigned long plane_mask[1];
    unsigned long pixels_return[256];
    unsigned cells;

    cells = 1;
    numFreeCells = 0;
    while ((cells != 0)) {
        cells =
            XAllocColorCells(theDisplay, theColormap, TRUE, plane_mask,
                             0, &pixels_return[numFreeCells], 1);
        if (cells) {
          theFreeCells[numFreeCells] = pixels_return[numFreeCells];
          numFreeCells++;
        }
    }
    XFreeColors (theDisplay, theColormap, pixels_return, numFreeCells, 0);
}

/* allocates storage for s1 + s2, cats them together, and frees the original */
static void appendString (char **s1, char *s2) {
  char *tmp;

  tmp = (char *) malloc(strlen(*s1)+strlen(s2)+1);
  strcpy (tmp, *s1);
  strcat (tmp, s2);
  free (*s1);
  *s1 = tmp;
}

void checkCells () {
    XColor theColors[256];
    static XColor oldColors[256];
    static int firstTime = TRUE;
    int i, changed = 0;
    char text [256];
    char *totalLog = NULL;
    time_t t ;
    struct tm *today = localtime(&t);
  
    t = time(0);
    today = localtime(&t);

    for (i = 0; i < 256; i++) {
        theColors[i].pixel = (Pixel) i;
        theColors[i].flags = DoRed | DoGreen | DoBlue;
    }
    XQueryColors (theDisplay, theColormap, theColors, 256);
    for (i = 0; i < 256; i++) {
        if (!firstTime)
            if ( (theColors[i].red != oldColors[i].red) || 
                 (theColors[i].green != oldColors[i].green) ||
                 (theColors[i].blue != oldColors[i].blue) ) {
                changed++;
                if (verbose) {
sprintf(text, "pixel  %3d  changed to  $%04X, $%04X, $%04X  at  %s\n",
	i, theColors[i].red, theColors[i].green, theColors[i].blue, theTime());
                    if (!totalLog)
                        totalLog = strdup ("");
                    appendString (&totalLog, text);
                }
            }
        oldColors[i].red = theColors[i].red;
        oldColors[i].green = theColors[i].green;
        oldColors[i].blue = theColors[i].blue;
    }
    firstTime = FALSE;
    if (!verbose)
        if (changed) {
            if (!totalLog)
	            totalLog = strdup ("");
            sprintf (text, "%d pixels changed at %s\n", changed, theTime());
            appendString (&totalLog, text);
        }
    sprintf (text, "---\n");
    if (totalLog)
        appendString (&totalLog, text);
    logString (totalLog); 
}


void blackOutCmap () {
    XColor xcolor;
    int cells, cellNum;
    unsigned long plane_mask[1];
    unsigned long pixels_return[256];

    xcolor.flags = DoRed | DoGreen | DoBlue ;
    xcolor.red = 0;
    xcolor.green = 0;
    xcolor.blue = 0;

    cells = 1;
    cellNum = 0;
    while ((cells != 0)) {
        cells =
            XAllocColorCells(theDisplay, theColormap, TRUE, plane_mask,
                             0, &pixels_return[cellNum], 1);
        if (cells) {
          xcolor.pixel = pixels_return[cellNum];
          XStoreColor (theDisplay, theColormap, &xcolor);
    	  cellNum++;
        }
    }
    XFreeColors (theDisplay, theColormap, pixels_return, cellNum, 0);
}

Boolean isFree (Pixel pix) {
    int i = 0;
    Boolean found = FALSE;

    while ((i < numFreeCells) && (!found))
      if (theFreeCells[i++] == pix) 
        found = TRUE;
    return found;
}

int cellsFree() {
    countFreeCells();
    return numFreeCells;
}


void cmapDraw (Widget theWidget, int windowWidth, int windowHeight) {
  int width_diff, height_diff, rect_width, rect_height;
  int i, x, y;
  unsigned int width, height;
  XGCValues theValues;
  static GC gc = NULL;

  if (!gc) {
      gc = XCreateGC (theDisplay, RootWindowOfScreen(theScreen), 0, 0);
      XSetForeground(theDisplay, gc, XBlackPixelOfScreen(theScreen));
      XSetBackground(theDisplay, gc, XWhitePixelOfScreen(theScreen));
  }

  /* save the foreground */
  XGetGCValues (theDisplay, gc, GCForeground, &theValues);

  width_diff = windowWidth % 16;
  height_diff = windowHeight % 16;
  rect_width = windowWidth / 16;
  rect_height = windowHeight / 16;

  for (i=0; i<256; i++) {
      XSetForeground(theDisplay, gc, i);
      if ((i%16) < width_diff) {
          x = (i%16)*rect_width+(i%16);
          width = rect_width+1;
      } else {
          x = (i%16)*rect_width+width_diff;
          width = rect_width;
      }

      if ((i/16) < height_diff) {
          y = (i/16)*rect_height+(i/16);
          height = rect_height+1;
      } else {
          y = (i/16)*rect_height+height_diff;
          height = rect_height;
      }
      XFillRectangle
          (theDisplay, XtWindow(theWidget), gc, x, y, width, height);
  }

  /* restore the foreground */
  XSetForeground (theDisplay, gc, theValues.foreground);
}

void
cmapExposeResize(Widget w, XtPointer unused, XmDrawingAreaCallbackStruct *cbs) {
    Status s;
    XWindowAttributes winAttr;

    if (cbs->window) {
        s = XGetWindowAttributes (theDisplay, cbs->window, &winAttr);
        cmapDraw(w, winAttr.width, winAttr.height);
    }
}

void cmapReport (Window w, int x, int y) {
    Status s;
    XWindowAttributes winAttr;
    static Pixel lastPix = 0;

    s = XGetWindowAttributes (theDisplay, w, &winAttr);
    
    thePixel = ( (y / 16) * 16 ) + (x / 16);

    if (thePixel != lastPix) 
        rgbReset();
    lastPix = thePixel;
}


static void 
cmapInput (Widget w, XtPointer xtp, XmDrawingAreaCallbackStruct *cbs) {
    if (((XAnyEvent *)cbs->event)->type == 4) {
        XButtonEvent *e = (XButtonEvent *) cbs->event;
        cmapReport (e->window, e->x, e->y);
    } 
}
        
static void cmapMotion (Widget w, XtPointer idp, XEvent *e, Boolean *cont) {
    XMotionEvent *event = (XMotionEvent *) e;
    *cont = True;

    cmapReport (event->window, event->x, event->y);
}

static void cmapCmapChanged (Widget w, XtPointer idp, XEvent *e, Boolean *cont)
{
    char buf[512];
    XColormapEvent *event = (XColormapEvent *) e;
    *cont = True;

    if (event->colormap == None) {
        sprintf (buf, "Colormap freed\n", event->colormap);
    } else {
        if (event->state == ColormapInstalled)
            sprintf (buf, "Colormap %x installed at %s\n", 
                     event->colormap, theTime());
        else 
            sprintf (buf, "Colormap %x uninstalled at %s\n", 
					 event->colormap, theTime());
    }
    logString (buf);
}

void cmapCreate (Widget parent) {
    Arg args[16];
    Widget  cmapCanvas;
    int n;
    
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNtopOffset, 0);        n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNbottomOffset, 0);        n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNleftOffset, 0);        n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNrightOffset, 0);        n++;
    cmapCanvas = XtCreateManagedWidget ("cmapCanvas", xmDrawingAreaWidgetClass,
                                        parent, args, n);

    XtAddCallback (cmapCanvas, XmNexposeCallback,
                   (XtCallbackProc) cmapExposeResize, NULL);
    XtAddCallback (cmapCanvas, XmNresizeCallback,
                 (XtCallbackProc) cmapExposeResize, NULL);

    XtAddCallback (cmapCanvas, XmNinputCallback,
                 (XtCallbackProc) cmapInput, NULL);
    XtAddEventHandler (cmapCanvas, Button1MotionMask,
                     False, (XtEventHandler) cmapMotion, NULL);
    XtAddEventHandler (cmapCanvas, ColormapChangeMask,
                     False, (XtEventHandler) cmapCmapChanged, NULL);

    countFreeCells();
}


