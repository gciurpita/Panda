#ifndef VARS_H
# define VARS_H

#include <Arduino.h>    // needed for byte

#include "eng.h"

#define MAX_CHAR  33

#define MAX_THR   100
#define MAX_REV   100
#define MAX_MID   (MAX_REV / 2)

enum {
    DIR_NEUTRAL = 0,
    DIR_FOR     = 1,
    DIR_REV     = -1
};


struct Vars_s {
    int        *p;
    const char *desc;
};

extern Vars_s  *pVars;

// -------------------------------------
#define N_LOCO      20

struct Loco_s {
    int     adr;
    float   mphToDcc;
    int     engIdx;
};

extern Loco_s locos [N_LOCO];

// -------------------------------------
// dynamic variables

extern int      dccAdr;

 // extern int      brakeInd;
extern float    brakeIndPct;
extern int      brakeCfm;

 // extern int      brakeAir;
extern float    brakeAirPct;
extern int      button;

extern int      cars;
extern int      carLen;
extern int      cutoff;

extern int      dccSpd;
extern int      dccSpdLst;

extern int      dir;

extern int      encApos;
extern int      encBpos;

extern int      grX10;

extern int      engine;
extern int      locoIdx;

extern unsigned long    msec;
extern int      mass;

extern float    mph;
extern float    mphLst;
extern float    mphToDcc;

extern int      reverser;

extern int      state;

extern int      throttle;
extern int      timeSec;
extern int      tonnage;

extern int      tractEff;
extern int      tractEffMax;

extern int      whistle;
extern int      wtCar;

// -------------------------------------
// stored variables

extern char     name [];
extern const char *version;

extern char     host [];
extern int      port;

extern char     ssid [];
extern char     pass [];

#endif
