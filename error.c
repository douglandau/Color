
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>

#define UNANSWERED -1
#define ERROR_OK 1

extern XtAppContext theAppContext;
extern Widget theToplevelShell;

Widget dialog;
Widget responseShell = NULL, responseForm, responseText;
int answer = UNANSWERED;
char *theResponse = NULL;

void choiceCB (Widget w, XtPointer clientData, XmAnyCallbackStruct *cbs) {
    answer = (int) clientData;
}

/*  keep a dialog visible  */
static void keepOnTop 
(Widget w, XtPointer unused, XEvent *eventP, Boolean *keep_goingP) {
    XMapRaised(XtDisplay(w), XtWindow(w));
    *keep_goingP = True;
}

/* center a popup */
void centerMe (Widget w, XtPointer unused, XmAnyCallbackStruct *cbs) {
    Dimension width, height;
    Position x, y;

    XtVaGetValues (w, XmNx, &x, XmNy, &y, NULL);

    x = (Position) WidthOfScreen(XtScreen(w)) / 2;
    y = (Position) HeightOfScreen(XtScreen(w)) / 2;
    XtVaGetValues (w, XmNwidth, &width, XmNheight, &height, NULL);
    x -= width/2;
    y -= height/2;
    XtVaSetValues (w, XmNx, x, XmNy, y, NULL);
}


void createMessage (Widget parent) {
    XmString okString;

    if (!dialog) {
      dialog = XmCreateWarningDialog (parent, "message", NULL, 0);
      okString = XmStringCreateLtoR ( "Ok", XmSTRING_DEFAULT_CHARSET);

      XtVaSetValues (dialog, XmNokLabelString, okString,
                             XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL,
                             XmNdefaultPosition, False,
                             NULL);
      XtVaSetValues (XtParent(dialog), XmNdeleteResponse, XmDO_NOTHING, NULL);
      XtVaSetValues (XtParent(dialog), XmNtitle, "Message", NULL);
      XtAddEventHandler( XtParent(dialog), VisibilityChangeMask, False,
						(XtEventHandler) keepOnTop,NULL);
      XtAddCallback (dialog, XmNmapCallback, (XtCallbackProc) centerMe, NULL);
      XtAddCallback (dialog, XmNokCallback, 
                     (XtCallbackProc) choiceCB, (XtPointer) ERROR_OK);
      XmStringFree (okString);
      XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_CANCEL_BUTTON));
      XtUnmanageChild (XmMessageBoxGetChild (dialog, XmDIALOG_HELP_BUTTON));
  }
}

void Message(char *s) {
    XmString msgString, okString;
    char buf[512];

    answer = UNANSWERED;
    sprintf (buf, "%s", s);
    msgString = XmStringCreateLtoR (buf, XmSTRING_DEFAULT_CHARSET);
    XtVaSetValues (dialog, XmNmessageString, msgString, NULL);

    XtManageChild (dialog);
    XtPopup (XtParent(dialog), XtGrabNone);

    while (answer==UNANSWERED)
      XtAppProcessEvent(theAppContext, XtIMAll);
}

void responseCB (Widget w, XtPointer clientData, XmAnyCallbackStruct *cbs) {
    char *theText = NULL;

    XtUnmanageChild (responseForm);
    XtPopdown (responseShell);

    answer = (int) clientData;
    if (theResponse) {
        free (theResponse);
        theResponse = NULL;
    }

    if (answer) {
        if (!(theText = XmTextGetString(responseText)) || !*theText) {
            if (theText)
                XtFree (theText);
            return ;
        } 
    } else 
 	return;

    if (theText) {
        theResponse = strdup(theText);
        XtFree (theText);
    }
}

void timestampCB (Widget w, XtPointer clientData, XmAnyCallbackStruct *cbs) {
    char buf[128];
    XmTextPosition curpos;
    Widget textw = (Widget) clientData;
    curpos = XmTextGetInsertionPosition (textw);
    sprintf (buf, "%s", theTime());
    XmTextInsert (textw, curpos, buf);
}


char *GetResponse (char *message) {
  char buf[512];
  Widget w, aLabel, rowcol;
  Arg args[16];
  int n;

  if (!responseShell) {
      responseShell = 
	  XtVaCreatePopupShell( "responseShell", xmDialogShellWidgetClass, 
                             theToplevelShell, 
/* width and/or height necessary to force the realizeWidget?? */
XmNwidth, 100, XmNheight, 100,
				XmNdialogType, XmDIALOG_SYSTEM_MODAL, 
				XmNtitle, "Add comment:", NULL);
      /* the XtRealizeWidget is not necessary if we set a width and height? */
      /*XtRealizeWidget (responseShell); */

      XtVaSetValues (responseShell, XmNtitle, "Add comment:", NULL);
      XtAddCallback 
        (responseShell, XmNpopupCallback, (XtCallbackProc) centerMe, NULL);
      XtAddEventHandler (responseShell, VisibilityChangeMask, False, 
						  (XtEventHandler) keepOnTop, NULL);

      responseForm = XtVaCreateWidget
                       ("responseForm",xmFormWidgetClass,responseShell,args,n);

      n = 0;
      XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);                n++;
      XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);               n++;
      XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);              n++;
      XtSetArg (args[n], XmNorientation, XmHORIZONTAL);                   n++;
      aLabel = 
	  XtCreateManagedWidget(message, xmLabelWidgetClass, responseForm,args,n);

      n = 0;
      XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);               n++;
      XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);              n++;
      XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);             n++;
      XtSetArg (args[n], XmNorientation, XmHORIZONTAL);                   n++;
      rowcol = XtCreateManagedWidget
			  ("rowcol",xmRowColumnWidgetClass,responseForm,args,n);
  
      w = XtCreateManagedWidget("Ok", xmPushButtonWidgetClass, rowcol, NULL, 0);
      XtAddCallback
          (w, XmNactivateCallback, (XtCallbackProc)responseCB, (XtPointer)TRUE);
      w = XtCreateManagedWidget("Cancel",xmPushButtonWidgetClass,rowcol,NULL,0);
      XtAddCallback (w, XmNactivateCallback, (XtCallbackProc)responseCB, FALSE);

      n = 0;
      XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);              n++;
      XtSetArg (args[n], XmNtopWidget, aLabel);                           n++;
      XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);               n++;
      XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);              n++;
      XtSetArg (args[n], XmNeditMode,  XmMULTI_LINE_EDIT);                n++;
      XtSetArg (args[n], XmNbottomAttachment, XmATTACH_WIDGET);           n++;
      XtSetArg (args[n], XmNbottomWidget, rowcol);                        n++;
      responseText = XtCreateManagedWidget 
	    ("responseText", xmTextWidgetClass, responseForm, args, n);

      w = XtCreateManagedWidget 
	    ("Add Timestamp", xmPushButtonWidgetClass, rowcol, NULL,0);    
	  XtAddCallback 
		(w, XmNactivateCallback, (XtCallbackProc) timestampCB, responseText);

  }
  XtManageChild (responseForm);
  XtPopup (XtParent(responseShell), XtGrabNone);

  answer = UNANSWERED;
  while (answer==UNANSWERED)
    XtAppProcessEvent(theAppContext, XtIMAll);

  return theResponse;
}
 
