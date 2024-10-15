#ifndef SIMD_H
# define SIMD_H

# include "eng.h"
# include "phyConst.h"

// ----------------------------------------------------------
typedef struct  {
    // static based on loco & train
    float   cylArea;
    float   cylVol;
    int     cycPrev;

    long    maxTe;
    long    slpTe;

    // dynamic
    float   time;

    float   mep;
    float   cut;        // int ?
    float   thr;        // int ?

    float   thrDia;
    int     nCyl;
    float   den;
    float   fill;
    float   flw;        // instantaneous
    float   flow;       // averaged
    float   consume;    // averaged
    float   vol;

    float   psiChst;
    float   psiCyl;

    float   lbFloco;
    float   lbFnet;
    float   te;

    float   acc;
    float   fps;
    float   mph;
 // float   rps;        // rev/sec
    float   cps;        // cyc/sec
 // float   rev;
 //
    float   cyc;
    float   cycLst;

    float   revPft;     // loco attribute

 // float   brFres;
 // float   grFres;
    float   locFres;
 // float   trFres;

 // float   lbFres;

    float   negF;
    float   posF;

    long    slip;
} State_t;

// ----------------------------------------------------------
extern State_t  st;

extern void  engineInit (void);
extern void  enginePr   (int hdr);
extern float engineTe   (float dTsec, float fps, int throttle, int cutoff);
extern void  engineRst  (void);

#endif
