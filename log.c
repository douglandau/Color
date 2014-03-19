

#include <Xm/Scale.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/FileSB.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/ScrolledW.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/MwmUtil.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>

#include "color.h"
#include "log.h"
#include "cmap.h"
#include "error.h"

#include <time.h>


extern XtAppContext app;
extern Display *theDisplay;
extern Screen *theScreen;
extern GC theGC;
extern Colormap theColormap;

static Widget logForm, logSWin, logText, fileBrowser;
static char dirName[512], fileName[512];
static char *theLog = NULL;

static void logSaveLog (char *filename) {
    int i;
    FILE *saveFile, *fopen();
    char buf[128];
    char *theText = NULL;

    if (!(saveFile = fopen(filename, "w"))) {
        if (saveFile = fopen(filename, "r"))
            sprintf (buf, "Can't write to %s (is read-only).\n", filename);
        else
            sprintf (buf, "Unknown error opening %s.", filename);
        Message (buf);
        return;
    }
    if (!(theText = XmTextGetString(logText)) || !*theText) {
        if (theText)
 	    XtFree (theText);
        return;
    }
    fprintf (saveFile, "%s\n", theText);
    fclose(saveFile);
}

/*  A choice has been made from the save file browser */
static void logSaveCB 
(Widget w, XtPointer client_data, XmFileSelectionBoxCallbackStruct *cbs) {
  XtUnmanageChild (w);

  if (cbs->reason == XmCR_OK) {
    char *fname, *filename, dirname[512];
    char *p;

    if (XmStringGetLtoR (cbs->value, XmSTRING_DEFAULT_CHARSET, &fname)) {
      if (!strrchr(fname, '/'))
          return;                       /* no '/' */
      strcpy(dirname, fname);
      filename = strrchr(fname, '/') + 1;
      p = strrchr(dirname, '/');
      *p = '\0';

      strcpy(dirName, dirname);
      strcpy(fileName, filename);

      if (strlen(fname) > 0) {
        if (fname) {
	      logSaveLog (fname); 
 	    }
      }
    }
  }
}

static void logSaveFileBrowserCreate() {
    int n = 0;
    Arg args[8];
    XmString xs ;
    XmString ok;

    if (fileBrowser)
        return;

    xs = XmStringCreateLtoR("./", XmSTRING_DEFAULT_CHARSET);
    ok = XmStringCreateLtoR("Ok", XmSTRING_DEFAULT_CHARSET);

    XtSetArg (args[n], XmNdirectory, xs);                         n++;
    XtSetArg (args[n], XmNtitle, "Save");                	  n++;
    XtSetArg (args[n], XmNwidth, 300);                            n++;
    fileBrowser = XmCreateFileSelectionDialog (logForm,
                                               "fileBrowser", args, n);
    XmStringFree (xs);
    XtAddCallback(fileBrowser, XmNokCallback,
                  (XtCallbackProc) logSaveCB, NULL);
    XtAddCallback(fileBrowser, XmNcancelCallback,
                  (XtCallbackProc) logSaveCB, NULL);

    XtUnmanageChild
        (XmFileSelectionBoxGetChild (fileBrowser, XmDIALOG_HELP_BUTTON));

    XtVaSetValues (fileBrowser, XmNokLabelString, ok, NULL);
    XtVaSetValues (fileBrowser, XmNresizePolicy, XmRESIZE_GROW, NULL);
    XmStringFree (ok);
}

static void logSaveFileBrowserPopup() {

    if (!fileBrowser)
        logSaveFileBrowserCreate();

    XmFileSelectionDoSearch (fileBrowser, NULL);
    XtManageChild (fileBrowser);
    XtPopup(XtParent(fileBrowser), XtGrabNone);
}

/*  logSave - save the log to a file */
void logSave () {
    logSaveFileBrowserPopup();
}

/*  logColor - currently unused */
static void logColor (XColor color) {
    char tempStr [256];
    sprintf(tempStr, "Pixel %3ld  $%04x, $%04x, $%04x\n",
           color.pixel, color.red, color.green, color.blue);
}

/*  logString - append the given string to the log */
void logString (char *str) {
    int size;
    char *text = NULL;

    if (!str)
        return;

    size = 0;
    if (theLog)
      size = strlen(theLog);
    size += strlen(str);

    if (!(text = XtMalloc(size+1))) {
      printf ("logString: malloc failed\n");
      return;
    }
    if (theLog)
      sprintf (text, "%s%s", theLog, str);
    else
      sprintf (text, "%s", str);

    if (theLog)
      XtFree (theLog);
    theLog = text;
    XmTextSetString(logText, theLog);
    XmTextShowPosition (logText, strlen(theLog));

}

/*  logClear - clears out the log */
void logClear () {
    char text[256];

    if (theLog)
      XtFree (theLog);
    theLog = NULL;

    sprintf(text, "log cleared at %s\n", theTime());
    logString (text);
}

/*  logAddComment - get a comment from the user and add it to the log */
void logAddComment () {
    char *r, text[256];
    sprintf (text, "enter comment:");
    r = GetResponse(text);
    if (r) 
      if (strlen(r) > 0) {
        logString (r);
        if (r[strlen(r)-1] != '\n') {
          char eoln[2];
          sprintf (eoln, "\n");
          logString (eoln);
        }
      }
}

/*  logAddSeparator - add a separator to the log */
void logAddSeparator () {
    char text[256];
    sprintf (text, "-----\n");
    logString (text);
}

/*  logInit - add the "log started at..." message to the log */
static void logInit () {
    char text[256];
    sprintf (text, "log started at %s\n",  theTime());
    logString (text);
    logAddSeparator();
}

/*  logCreate - create the GUI to show and manipulate the log */
void logCreate (Widget parent) {
    XmString xs;
    int n, yOffset;
    Arg args[16];
    Widget w, rowcol, logFrame;
    char buf[128];

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);     n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);       n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);      n++;
    XtSetArg (args[n], XmNfractionBase, 100);           	n++;
    XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE);         n++;
    logForm = 
	XtCreateManagedWidget("logForm",xmFormWidgetClass,parent,args,n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);       n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);      n++;
    XtSetArg (args[n], XmNshadowType, XmSHADOW_IN);         	n++;
    logFrame = 
        XtCreateManagedWidget("logFrame",xmFrameWidgetClass,logForm,args,n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);        	n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);       	n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);      	n++;
    XtSetArg (args[n], XmNorientation, XmHORIZONTAL);        		n++;
    rowcol = 
	XtCreateManagedWidget("rowcol",xmRowColumnWidgetClass,logFrame,args,n);

    w = XtCreateManagedWidget("Clear", 
				xmPushButtonWidgetClass, rowcol, NULL, 0);
    XtAddCallback
        (w, XmNactivateCallback, (XtCallbackProc) logClear, (XtPointer) w);

    w = XtCreateManagedWidget("Save", 
				xmPushButtonWidgetClass, rowcol, NULL, 0);
    XtAddCallback
        (w, XmNactivateCallback, (XtCallbackProc) logSave, (XtPointer) w);

    w = XtCreateManagedWidget("Add separator", 
				xmPushButtonWidgetClass, rowcol, NULL, 0);
    XtAddCallback
        (w, XmNactivateCallback, (XtCallbackProc) logAddSeparator,(XtPointer)w);

    w = XtCreateManagedWidget("Add comment", 
				xmPushButtonWidgetClass, rowcol, NULL, 0);
    XtAddCallback
        (w, XmNactivateCallback, (XtCallbackProc) logAddComment, (XtPointer) w);


    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);          n++;
    XtSetArg (args[n], XmNtopWidget, logFrame);          	n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);       n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);         n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNeditable,  False);			  n++;
    XtSetArg (args[n], XmNscrollingPolicy, XmAUTOMATIC);          n++;
    XtSetArg (args[n], XmNeditMode,  XmMULTI_LINE_EDIT);                n++;
    logText = XmCreateScrolledText (logForm, "logText", args, n);
    XtManageChild(logText);

    logInit();
}



