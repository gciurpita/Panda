#ifndef KOALA_H
# define KOALA_H

#define ST_NULL     (0)
#define ST_WIFI     (1 << 0)
#define ST_JMRI     (1 << 1)
#define ST_NO_LOCO  (1 << 2)
#define ST_HORN     (1 << 3)
#define ST_MENU     (1 << 4)
#define ST_CFG      (1 << 5)
#define ST_DVT      (1 << 6)
#define ST_ECHO     (1 << 7)
#define ST_E_STOP   (1 << 8)

# define NUL        0

# define DISP_Y0    0
# define DISP_Y1    14
# define DISP_Y2    28
# define DISP_Y3    42

# define CLR     1

# define SH1106
# ifdef SH1106
#  include "SH1106Wire.h"
extern SH1106Wire  display;
# else
#  include "SSD1306Wire.h"
extern SSD1306Wire  display;
# endif

#define ABS(x)   (0 > (x) ? -(x) : (x))
#define SGN(x)   (0 > (x) ? -1   : 1)

#define DBG_ENGINE   2
#define DBG_BRAKE    4
#define DBG_CYLPRESS 8

#define DBG_FORCE    16
#define DBG_MENU     32
#define DBG_BUT      64
#define DBG_KEYPAD  128

extern unsigned int debug;
extern void eStop (void);
extern void dispOled (const char*, const char*, const char*, const char*, bool); 
// extern char s0[30];
// extern char s1[30];
//
#define S_SIZE  80
extern char s [S_SIZE];

enum { FUNC_CLR, FUNC_SET, FUNC_TGL };
int jmriFuncKey (unsigned func, int cmd);

void wifiSend (const char*);

#endif
