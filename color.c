


#include "stdlib.h"
#include "stdio.h"
#include "time.h"

#include <Xm/ArrowB.h>
#include <Xm/BulletinB.h>
#include <Xm/CascadeB.h>

#include <Xm/DrawingA.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/TextF.h>

#include "color.h"
#include "rgb.h"
#include "log.h"
#include "cmap.h"


Display *theDisplay;
Visual *theVisual;
Colormap theColormap;
Screen *theScreen;
GC theGC;
Pixel thePixel;
XtAppContext theAppContext;
Widget theToplevelShell;

Widget mainForm, menuForm, leftForm, rightForm, cmapForm, rgbForm;

#define CELL_CHECK_TIMEOUT 5

char *theTime() {
    static char text[256];
    //static long time(), t ;
    time_t t ;
    struct tm *today = localtime(&t);
 
    t = time(0);
    today = localtime(&t);
    sprintf(text, "%2d:%2d:%2d", today->tm_hour, today->tm_min, today->tm_sec);
    return text;
}

static void timer_notify (XtPointer clientData, XtIntervalId *unused) {
    checkCells();
    rgbCellsFree(); 
    XtAppAddTimeOut(theAppContext, CELL_CHECK_TIMEOUT*1000, timer_notify,NULL);
}

void fileCB (Widget w, XtPointer cdata, XtPointer cbs) {
   if ( strcmp(XtName((Widget) cdata), "Exit...") == 0)
     exit (0);
}

void logCB (Widget w, XtPointer cdata, XtPointer cbs) {
   if ( strcmp(XtName((Widget) cdata), "Verbose") == 0)
       logVerbose (TRUE);
   if ( strcmp(XtName((Widget) cdata), "Quiet") == 0)
       logVerbose (FALSE);
   if ( strcmp(XtName((Widget) cdata), "Save") == 0)
       logSave ();
   if ( strcmp(XtName((Widget) cdata), "Clear") == 0)
       logClear ();
   if ( strcmp(XtName((Widget) cdata), "Add Comment") == 0)
       logAddComment ();
   if ( strcmp(XtName((Widget) cdata), "Add Separator") == 0)
       logAddSeparator ();
}


void blackOutCB (Widget w, XtPointer unused, XtPointer cbs) {
    blackOutCmap();
}

menuCreate (Widget parent) {
  Widget menubar, fileMenu, colormapMenu, cellMenu, logMenu, w;
  XmString xs;
  Arg args[8];
  int n = 0;

  XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);     n++;
  XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);    n++;
  menubar = XmCreateMenuBar (parent, "mmMenubar", args, n);
  fileMenu = XmCreatePulldownMenu (parent, "fileMenu", NULL, 0);
  colormapMenu = XmCreatePulldownMenu (parent, "colormapMenu", NULL, 0);
  cellMenu = XmCreatePulldownMenu (parent, "cellMenu", NULL, 0);
  logMenu = XmCreatePulldownMenu (parent, "logMenu", NULL, 0);

  xs = XmStringCreateLtoR("File", XmSTRING_DEFAULT_CHARSET);
  XtVaCreateManagedWidget ("File", xmCascadeButtonWidgetClass, menubar,
                           XmNlabelString, xs,
                           XmNmnemonic, 'F',
                           XmNsubMenuId, fileMenu, NULL);
  XmStringFree (xs);

  w = XtVaCreateManagedWidget ( "Load...", xmPushButtonGadgetClass, fileMenu,
                                 XmNmnemonic, 'L', NULL);
  XtSetSensitive (w, FALSE);
  w = XtVaCreateManagedWidget ("Save...", xmPushButtonGadgetClass, fileMenu,
                                XmNmnemonic, 'S', NULL);
  XtSetSensitive (w, FALSE);
  XtVaCreateManagedWidget ("separator", xmSeparatorGadgetClass, fileMenu, NULL);
  w = XtVaCreateManagedWidget ("Exit...", xmPushButtonGadgetClass, fileMenu,
                                XmNmnemonic, 'x', NULL);
  XtAddCallback
        (w, XmNactivateCallback, (XtCallbackProc) fileCB, (XtPointer) w);

  xs = XmStringCreateLtoR("Colormap", XmSTRING_DEFAULT_CHARSET);
  XtVaCreateManagedWidget ("Colormap", xmCascadeButtonWidgetClass, menubar,
                           XmNlabelString, xs,
                           XmNmnemonic, 'M',
                           XmNsubMenuId, colormapMenu, NULL);
  XmStringFree (xs);

  w = XtVaCreateManagedWidget ( "Black out free cells", xmPushButtonGadgetClass,
                                 colormapMenu, XmNmnemonic, 'B', NULL);
  XtAddCallback 
	(w, XmNactivateCallback, (XtCallbackProc) blackOutCB, (XtPointer) 0);


  xs = XmStringCreateLtoR("Cell", XmSTRING_DEFAULT_CHARSET);
  XtVaCreateManagedWidget ("Cell", xmCascadeButtonWidgetClass, menubar,
                           XmNlabelString, xs,
                           XmNmnemonic, 'C',
                           XmNsubMenuId, cellMenu, NULL);
  XmStringFree (xs);

  w = XtVaCreateManagedWidget ( "Allocate", xmPushButtonGadgetClass,
                                 cellMenu, XmNmnemonic, 'A', NULL);
  XtSetSensitive (w, FALSE);
  w = XtVaCreateManagedWidget ( "Free", xmPushButtonGadgetClass,
                                 cellMenu, XmNmnemonic, 'F', NULL);
  XtSetSensitive (w, FALSE);

  xs = XmStringCreateLtoR("Log", XmSTRING_DEFAULT_CHARSET);
  XtVaCreateManagedWidget ("Log", xmCascadeButtonWidgetClass, menubar,
                           XmNlabelString, xs,
                           XmNmnemonic, 'L',
                           XmNsubMenuId, logMenu, NULL);
  XmStringFree (xs);

  w = XtVaCreateManagedWidget ( "Verbose", xmPushButtonGadgetClass,
                                 logMenu, XmNmnemonic, 'V', NULL);
  XtAddCallback
        (w, XmNactivateCallback, (XtCallbackProc) logCB, (XtPointer) w);

  w = XtVaCreateManagedWidget ( "Quiet", xmPushButtonGadgetClass,
                                 logMenu, XmNmnemonic, 'Q', NULL);
  XtAddCallback
        (w, XmNactivateCallback, (XtCallbackProc) logCB, (XtPointer) w);
  w = XtVaCreateManagedWidget ( "Save", xmPushButtonGadgetClass,
                                 logMenu, XmNmnemonic, 'S', NULL);
  XtAddCallback
        (w, XmNactivateCallback, (XtCallbackProc) logCB, (XtPointer) w);

  w = XtVaCreateManagedWidget ( "Clear", xmPushButtonGadgetClass,
                                 logMenu, XmNmnemonic, 'C', NULL);
  XtAddCallback
        (w, XmNactivateCallback, (XtCallbackProc) logCB, (XtPointer) w);

  w = XtVaCreateManagedWidget ( "Add Comment", xmPushButtonGadgetClass,
                                 logMenu, XmNmnemonic, 'A', NULL);
  XtAddCallback
        (w, XmNactivateCallback, (XtCallbackProc) logCB, (XtPointer) w);

  w = XtVaCreateManagedWidget ( "Add Separator", xmPushButtonGadgetClass,
                                 logMenu, XmNmnemonic, 'S', NULL);
  XtAddCallback
        (w, XmNactivateCallback, (XtCallbackProc) logCB, (XtPointer) w);

  XtManageChild (menubar);

}

main(int argc, char *argv[]) {
    int j, i, n;
    Widget w, leftMenuForm, rightMenuForm, rightMenuForm2, 
           leftNameForm, rightNameForm, logForm;
    Arg args[16];
    
    String fallbacks[] = {
        "*XmText.fontList:   	7x14",
        "*fontList:          	-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-*-*",
        "*XmCascadeButton.fontList:    \
				-adobe-helvetica-bold-o-*-*-14-*-*-*-*-*-*-*",
        "*mainMenuForm*XmPushButtonGadget.fontList:  \
				-adobe-helvetica-bold-o-*-*-14-*-*-*-*-*-*-*",
        "*responseForm.width:   	400",
        "*responseForm.height:   	200",
	NULL };

    theToplevelShell = XtVaAppInitialize (&theAppContext, "XCWatch", NULL, 0,
        &argc, argv, fallbacks, XmNtitle, "Colormap Watcher", NULL);

    theDisplay = XtDisplay(theToplevelShell);
    theScreen = XtScreen(theToplevelShell);
    theVisual = DefaultVisualOfScreen (theScreen);
    theColormap = DefaultColormapOfScreen(theScreen);
    theGC = DefaultGCOfScreen(theScreen);

    mainForm = XtVaCreateManagedWidget ("mainForm", xmFormWidgetClass, 
					theToplevelShell,
                            		XmNfractionBase, 100,
                            		XmNwidth, 512,
                            		XmNheight, 512 ,
                            		NULL);
    XtRealizeWidget (theToplevelShell);

    menuForm = XtVaCreateManagedWidget ("menuForm", xmFormWidgetClass, mainForm,
                                       XmNfractionBase, 100,
                                       XmNtopAttachment, XmATTACH_FORM,
                                       XmNleftAttachment, XmATTACH_FORM,
                                       XmNrightAttachment, XmATTACH_FORM,
                                       NULL);

    leftForm = XtVaCreateManagedWidget ("leftForm", xmFormWidgetClass, mainForm,
                                       XmNfractionBase, 100,
                                       XmNtopAttachment, XmATTACH_WIDGET,
                                       XmNtopWidget, menuForm,
                                       XmNleftAttachment, XmATTACH_FORM,
                                       XmNrightAttachment, XmATTACH_POSITION,
                                       XmNrightPosition, 50,
          				XmNheight, 256,
          				XmNresizePolicy, XmRESIZE_NONE,
                                       NULL);

    rightForm = 
        XtVaCreateManagedWidget("rightForm", xmFormWidgetClass, mainForm,
                                       XmNfractionBase, 100,
                                       XmNtopAttachment, XmATTACH_WIDGET,
                                       XmNtopWidget, menuForm,
                                       XmNrightAttachment, XmATTACH_FORM,
                                       XmNleftAttachment, XmATTACH_POSITION,
                                       XmNleftPosition, 50,
          				XmNheight, 256,
          				XmNresizePolicy, XmRESIZE_NONE,
                                       NULL);

    cmapForm = XtVaCreateManagedWidget("cmapForm",xmFormWidgetClass,leftForm,
                            XmNtopAttachment, XmATTACH_FORM,
                            XmNleftAttachment, XmATTACH_FORM,
                            XmNrightAttachment, XmATTACH_FORM,
                            XmNbottomAttachment, XmATTACH_FORM,
                            NULL);

    rgbForm =XtVaCreateManagedWidget("rgbForm",xmFormWidgetClass,rightForm,
                            XmNtopAttachment, XmATTACH_FORM,
                            XmNleftAttachment, XmATTACH_FORM,
                            XmNrightAttachment, XmATTACH_FORM,
                            XmNbottomAttachment, XmATTACH_FORM,
                            NULL);

    logForm = XtVaCreateManagedWidget("logForm",xmFormWidgetClass, mainForm,
                            XmNtopAttachment, XmATTACH_WIDGET,
                            XmNtopWidget, leftForm,
                            XmNleftAttachment, XmATTACH_FORM,
                            XmNrightAttachment, XmATTACH_FORM,
                            XmNbottomAttachment, XmATTACH_FORM,
                            NULL);


    menuCreate(menuForm);
    cmapCreate(cmapForm);
    rgbCreate(rgbForm);
    logCreate(logForm);

    checkCells();

    XtAppAddTimeOut(theAppContext, CELL_CHECK_TIMEOUT*1000, timer_notify, NULL);
    XtAppMainLoop (theAppContext);
}


