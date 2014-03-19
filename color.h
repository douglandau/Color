#ifndef color_h
#define color_h

/* Color editor   color.h    Doug Landau   Copyright (c) 1995 CEMAX */

#ifndef min
#define min(x,y) (((x) < (y)) ? (x) : (y))
#endif
#ifndef max
#define max(x,y) (((x) > (y)) ? (x) : (y))
#endif

#define STRLEN 128
#define BUTTON_WIDTH 50
#define BUTTON_HEIGHT 30
#define START_SIZE 300


typedef struct {
    int red, green, blue;
    char name[STRLEN];
    void *next, *restore;
} colorT;

char *theTime();
void colorExit();

#endif  /* color_h */
