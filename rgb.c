
#include <Xm/Scale.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/LabelG.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/MwmUtil.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>

#include "color.h"
#include "log.h"
#include "rgb.h"
#include "cmap.h"

#define RGB_WIDTH 300
#define RGB_HEIGHT 250

/*   private stuff  */

static Widget rgbForm, sliderForm, rgbSwatch, rgbSwatchFrame;
static Widget redText, redScale, greenText, greenScale, blueText, blueScale;
static Widget pixLabel, freeLabel;
      
extern Display *theDisplay;
extern Screen *theScreen;
extern GC theGC;
extern Colormap theColormap;
extern Pixel thePixel;


static void setLabel (Widget label, char *to) {
  XmString xs;

  xs = XmStringCreateLtoR(to, XmSTRING_DEFAULT_CHARSET);
  XtVaSetValues (label, XmNlabelString, xs, NULL);
  XmStringFree (xs);
}

static int getScale (Widget scale) {
    int i = 0;
    XtVaGetValues (scale, XmNvalue, &i, NULL);
    return i;
}

static int getTextField (Widget textField) {
    int i = 0;
    char *s = XmTextFieldGetString (textField);
    if (sscanf (s, "%d", &i) == 0)   /* no integer found, return zero */
      i = 0;
    XtFree (s);
    return (i);
}

static int setTextField (Widget textField, int value) {
    int newValue;
    char buf[128];

    newValue = max (value, 0);
    newValue = min (value, 255);
    sprintf (buf, "%d", newValue);
    XmTextFieldSetString (textField, buf);
}

static void setRgbScale (Widget scale, int value) {
    int scaleMin, scaleMax, newValue;

    newValue = max (value, 0);
    newValue = min (newValue, 255);
    XmScaleSetValue (scale, newValue);
}

static void 
textFieldChanged (Widget w, XtPointer scale, XEvent *e, Boolean *cont) {
    *cont = False;
    setRgbScale ((Widget) scale, getTextField(w));
    setCell (getScale(redScale), getScale(greenScale), getScale(blueScale));
    rgbReset();
}

static void 
scaleChanged (Widget w, XtPointer textField, XmScaleCallbackStruct *cbs) {
    setTextField ((Widget) textField, cbs->value);
    setCell (getScale(redScale), getScale(greenScale), getScale(blueScale));
}

static void drawSwatch (Pixel pix) {
    XGCValues savedValues;
    XWindowAttributes winAttr;
    Window w = XtWindow (rgbSwatch);

    XGetGCValues (theDisplay, theGC, GCForeground, &savedValues);
    XGetWindowAttributes (theDisplay, w, &winAttr);

    XSetForeground(theDisplay, theGC, pix);
    XFillRectangle (theDisplay, XtWindow(rgbSwatch), 
			theGC, 0, 0, winAttr.width, winAttr.height);

    XSetForeground (theDisplay, theGC, savedValues.foreground);
}


static void enableInput () {
    XtVaSetValues (redText, XmNeditable, True, NULL);
    XtVaSetValues (greenText, XmNeditable, True, NULL);
    XtVaSetValues (blueText, XmNeditable, True, NULL);
    XtVaSetValues (redScale, XmNsensitive, True, NULL);
    XtVaSetValues (greenScale, XmNsensitive, True, NULL);
    XtVaSetValues (blueScale, XmNsensitive, True, NULL);
}

static void disableInput () {

    XtVaSetValues (redText, XmNeditable, False, NULL);
    XtVaSetValues (greenText, XmNeditable, False, NULL);
    XtVaSetValues (blueText, XmNeditable, False, NULL);
    XtVaSetValues (redScale, XmNsensitive, False, NULL);
    XtVaSetValues (greenScale, XmNsensitive, False, NULL);
    XtVaSetValues (blueScale, XmNsensitive, False, NULL);
}


void rgbCellsFree () {
        int freeCells;
        char tempStr[256];

        freeCells = cellsFree();
        if (isFree (thePixel)) {
            sprintf (tempStr, "%d cells free including %ld",freeCells,thePixel);
            enableInput();
        } else {
            sprintf (tempStr, "%d cells free but not %ld", freeCells, thePixel);
            disableInput();
        }
        setLabel (freeLabel, tempStr);
}

void rgbReset () {
    if (thePixel <= 255) {
        XColor def;
        char tempStr[256];

        def.pixel = thePixel;
        XQueryColor(theDisplay, theColormap, &def);
        sprintf(tempStr, "Pixel %3ld  =  $%04x, $%04x, $%04x",
                             thePixel, def.red, def.green, def.blue);
        setRgbScale (redScale, def.red / 256);
        setRgbScale (greenScale, def.green / 256);
        setRgbScale (blueScale, def.blue / 256);
        setTextField (redText, def.red / 256);
        setTextField (greenText, def.green / 256);
        setTextField (blueText, def.blue / 256);
        setLabel (pixLabel, tempStr);

		rgbCellsFree ();

        drawSwatch (thePixel);
    } else {
        printf("Pix %3ld is out of legal range. \n", thePixel);
    }
}

static void 
swatchExpose (Widget w, XtPointer unused, XEvent *e, Boolean *cont) {
    rgbReset();
}



void rgbCreate (Widget parent) {
    XmString xs;
    int n, yOffset;
    Arg args[16];
    Widget w;
    char buf[128];

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNfractionBase, 100);           n++;
    XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE);         n++;
    rgbForm = XtCreateManagedWidget("rgbForm",xmFormWidgetClass,parent,args,n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);       n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);      n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_POSITION);      n++;
    XtSetArg (args[n], XmNbottomPosition, 50);      n++;
    XtSetArg (args[n], XmNfractionBase, 100);                   n++;
    XtSetArg (args[n], XmNresizePolicy, XmRESIZE_NONE);         n++;
    sliderForm = 
        XtCreateManagedWidget("sliderForm",xmFormWidgetClass,rgbForm,args,n);


    yOffset = 0; 
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+10);        		n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);           n++;
    XtSetArg (args[n], XmNrightPosition, 20);           n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END);    n++;
    w = XtCreateManagedWidget("Red:", xmLabelGadgetClass, sliderForm, args, n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+5);        		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, w);           n++;
    XtSetArg (args[n], XmNleftOffset, 5);           n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);           n++;
    XtSetArg (args[n], XmNrightPosition, 40);                       n++;
    XtSetArg (args[n], XmNmaxLength, 3);                       n++;
    XtSetArg (args[n], XmNnavigationType, XmNONE);           n++;
    XtSetArg (args[n], XmNtraversalOn, True);           n++;
    redText = XtCreateManagedWidget("redText",
                          xmTextFieldWidgetClass, sliderForm, args, n);
  
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+10);        		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, redText);           n++;
    XtSetArg (args[n], XmNleftOffset, 5);           n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END);    n++;
    w = XtCreateManagedWidget("0", xmLabelGadgetClass, sliderForm, args, n);
  
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset + 10);        		n++; 
    XtSetArg (args[n], XmNheight, 20);        	        	n++; 
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, w);           n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);           n++;
    XtSetArg (args[n], XmNrightPosition, 85);           n++;
    XtSetArg (args[n], XmNorientation, XmHORIZONTAL);           n++;
    XtSetArg (args[n], XmNminimum, 0);           n++;
    XtSetArg (args[n], XmNmaximum, 255);           n++;
    redScale = XtCreateManagedWidget("redScale",
                          xmScaleWidgetClass, sliderForm, args, n);
  
    XtAddCallback(redScale, XmNvalueChangedCallback,
                  (XtCallbackProc) scaleChanged,  (XtPointer) redText);
    XtAddCallback(redScale, XmNdragCallback,
                  (XtCallbackProc) scaleChanged,  (XtPointer) redText);
    XtAddEventHandler (redText, KeyReleaseMask, False, 
            (XtEventHandler) textFieldChanged, (XtPointer) (redScale));
  
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_POSITION);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+10);        		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, redScale);           n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING);    n++;
    w = XtCreateManagedWidget("255", xmLabelGadgetClass, sliderForm, args, n);
  
    yOffset += 30;

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+15);        		n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);           n++;
    XtSetArg (args[n], XmNrightPosition, 20);           n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END);    n++;
    w = XtCreateManagedWidget("Green:",xmLabelGadgetClass,sliderForm,args,n);
  
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+10);        		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, w);           n++;
    XtSetArg (args[n], XmNleftOffset, 5);           n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);           n++;
    XtSetArg (args[n], XmNrightPosition, 40);                       n++;
    XtSetArg (args[n], XmNmaxLength, 3);                       n++;
    XtSetArg (args[n], XmNnavigationType, XmTAB_GROUP);           n++; 
    XtSetArg (args[n], XmNtraversalOn, True);           n++;
    greenText = XtCreateManagedWidget("greenText",
                          xmTextFieldWidgetClass, sliderForm, args, n);
  
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+15);        		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, greenText);           n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END);    n++;
    XtSetArg (args[n], XmNleftOffset, 5);           n++;
    w = XtCreateManagedWidget("0", xmLabelGadgetClass, sliderForm, args, n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+15);        		n++;
    XtSetArg (args[n], XmNheight, 20);        	        	n++; 
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, w);           n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);           n++;
    XtSetArg (args[n], XmNrightPosition, 85);           n++;
    XtSetArg (args[n], XmNminimum, 0);           n++;
    XtSetArg (args[n], XmNmaximum, 255);           n++;
    XtSetArg (args[n], XmNorientation, XmHORIZONTAL);           n++;
    XtSetArg (args[n], XmNtraversalOn, False);           n++;
    greenScale = XtCreateManagedWidget("greenScale",
                          xmScaleWidgetClass, sliderForm, args, n);

    XtAddCallback(greenScale, XmNvalueChangedCallback,
                  (XtCallbackProc) scaleChanged,  (XtPointer) greenText);
    XtAddCallback(greenScale, XmNdragCallback,
                  (XtCallbackProc) scaleChanged,  (XtPointer) greenText);
    XtAddEventHandler (greenText, KeyReleaseMask, False, 
            (XtEventHandler) textFieldChanged, (XtPointer) greenScale);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+15);        		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, greenScale);           n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING);    n++;
    w = XtCreateManagedWidget("255", xmLabelGadgetClass, sliderForm, args, n);

    yOffset += 35;

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+15);        		n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);           n++;
    XtSetArg (args[n], XmNrightPosition, 20);           n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END);    n++;
    XtSetArg (args[n], XmNleftOffset, 5);           n++;
    w = XtCreateManagedWidget("Blue:", xmLabelGadgetClass,sliderForm, args, n);
  
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+10);        		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, w);           n++;
    XtSetArg (args[n], XmNleftOffset, 5);           n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);           n++;
    XtSetArg (args[n], XmNrightPosition, 40);                       n++;
    XtSetArg (args[n], XmNmaxLength, 3);                       n++;
    XtSetArg (args[n], XmNnavigationType, XmTAB_GROUP);           n++;
    XtSetArg (args[n], XmNtraversalOn, True);           n++;
    blueText = XtCreateManagedWidget("blueText",
                          xmTextFieldWidgetClass, sliderForm, args, n);
  
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+15);        		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, blueText);           n++;
    XtSetArg (args[n], XmNleftOffset, 5);           n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_END);    n++;
    w = XtCreateManagedWidget("0", xmLabelGadgetClass, sliderForm, args, n);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+15);        		n++;
    XtSetArg (args[n], XmNheight, 20);        	        	n++; 
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, w);           n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_POSITION);           n++;
    XtSetArg (args[n], XmNrightPosition, 85);           n++;
    XtSetArg (args[n], XmNminimum, 0);           n++;
    XtSetArg (args[n], XmNmaximum, 255);           n++;
    XtSetArg (args[n], XmNorientation, XmHORIZONTAL);           n++;
    blueScale = XtCreateManagedWidget("blueScale",
                          xmScaleWidgetClass, sliderForm, args, n);
  
    XtAddCallback(blueScale, XmNvalueChangedCallback,
                  (XtCallbackProc) scaleChanged,  (XtPointer) blueText);
    XtAddCallback(blueScale, XmNdragCallback,
                  (XtCallbackProc) scaleChanged,  (XtPointer) blueText);
    XtAddEventHandler (blueText, KeyReleaseMask, False, 
            (XtEventHandler) textFieldChanged, (XtPointer) blueScale);

    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);           n++;
    XtSetArg (args[n], XmNtopOffset, yOffset+15);        		n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);           n++;
    XtSetArg (args[n], XmNleftWidget, redScale);           n++;
    XtSetArg (args[n], XmNalignment, XmALIGNMENT_BEGINNING);    n++;
    w = XtCreateManagedWidget("255", xmLabelGadgetClass, sliderForm, args, n);


    yOffset += 50;
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNtopOffset, yOffset);        n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNleftOffset, 20);        n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);        n++;
    pixLabel = XtCreateManagedWidget("Pixel ",xmLabelWidgetClass,
					rgbForm,args,n);

    yOffset += 20;
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNtopOffset, yOffset);        n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNleftOffset, 20);        n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);        n++;
    freeLabel = XtCreateManagedWidget("Cells free",xmLabelWidgetClass,
                                        rgbForm,args,n);


    yOffset += 30;
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNtopOffset, yOffset);        n++;

    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNleftOffset, 20);        n++;
    XtSetArg (args[n], XmNrightAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNrightOffset, 20);        n++;
    XtSetArg (args[n], XmNbottomAttachment, XmATTACH_FORM);        n++;
    XtSetArg (args[n], XmNbottomOffset, 20);        n++;
    XtSetArg (args[n], XmNshadowType, XmSHADOW_IN);        n++;
    XtSetArg (args[n], XmNshadowThickness, 4);        n++;
    rgbSwatchFrame =
	XtCreateManagedWidget("",xmFrameWidgetClass,rgbForm,args,n);

    n = 0;
    rgbSwatch = 
        XtCreateManagedWidget("",xmLabelWidgetClass,rgbSwatchFrame,args,n);

    XtAddEventHandler (rgbSwatch, ExposureMask,
                     False, (XtEventHandler) swatchExpose, NULL);

    thePixel = 0;
    rgbReset();
}

