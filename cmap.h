#ifndef cmap_h
#define cmap_h


void logVerbose (int on);
void checkCells (); 
void blackOutCmap (); 
int cellsFree ();
Boolean allocCell (Pixel pix); 
Boolean setCell (int r, int g, int b);
void cmapCreate ();

#endif /* cmap_h */


