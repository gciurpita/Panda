// steam locomotive emulator

#if 0
# include "Arduino.h"

#else
# include <stdio.h>
# include <math.h>
# include <string.h>
#endif

#include "avgForce.h"
#include "eng.h"
#include "engine.h"
#include "koala.h"
#include "rollRes.h"

#include "vars.h"

// ----------------------------------------------------------
// state variables

State_t  st = {};

int      nCyc;

// ----------------------------------------------------------
// steam tables - relationship between temperature (F),
//      presure (psi), vapor and liquid density (lb/sq.ft ?)

float StDegF [] = {
    212.0,  239.4,  258.7,  274.0,  286.7,  297.7,  307.3,  316.0,
    323.9,  331.2,  337.9,  344.2,  350.1,  355.6,  360.9,  365.9,
    370.6,  375.6,  379.6,  383.8,  387.8,  391.7,  395.5,  399.1,
    402.6,  406.0,  409.4,  412.6,  415.7,  418.8,  421.8, };

float StPsi [] = {
        0,     10,     20,     30,     40,     50,     60,     70,
       80,     90,    100,    110,    120,    130,    140,    150,
      160,    170,    180,    190,    200,    210,    220,    230,
      240,    250,    260,    270,    280,    290,    300, };

float StDenS [] = {
   0.0373, 0.0606, 0.0834, 0.1057, 0.1277, 0.1496, 0.1713, 0.1928,
   0.2143, 0.2356, 0.2569, 0.2782, 0.2994, 0.3205, 0.3416, 0.3627,
   0.3838, 0.4048, 0.4259, 0.4469, 0.4680, 0.4890, 0.5101, 0.5312,
   0.5522, 0.5733, 0.5944, 0.6155, 0.6367, 0.6578, 0.6790, };

float StDenW [] = {
    59.83,  59.11,  58.57,  58.12,  57.73,  57.39,  57.07,  56.79,
    56.52,  56.27,  56.03,  55.81,  55.59,  55.39,  55.19,  55.00,
    54.82,  54.65,  54.48,  54.31,  54.15,  53.99,  53.84,  53.69,
    53.54,  53.40,  53.26,  53.12,  52.99,  52.86,  52.73, };

#define ST  (sizeof(StDenW) / sizeof(float))

// ----------------------------------------------------------
// interpolate -
static float
interp (
    float   x,
    float   vecX [],
    float   vecY [],
    int     N,
    int     dbg )
{
    // find vecX seqment x is on
    int  n;
    for (n = 1; n < N; n++)  {
        if (vecX [n] > x)
            break;
    }

    if (N == n)
        n--;

    // calc y on corresponding seqment of vecY
    float k   = (x - vecX [n-1]) / (vecX [n] - vecX [n-1]);
    float res = vecY [n-1] + k * (vecY [n] - vecY [n-1]);

    if (dbg) {
        printf ("%s:", __func__);
        printf (" x %.4f", x);
        printf (" n %d", n);
        printf (" N %d", N);
        printf (" vecX [n] %.4f", vecX [n]);
        printf (" vecY [n] %.4f", vecY [n]);
        printf (" vecX [n-1] %.4f", vecX [n-1]);
        printf (" vecY [n-1] %.4f", vecY [n-1]);
        printf (" res %.4f", res);
        printf ("\n");
    }

    return res;
}

// ----------------------------------------------------------
// locomotive (& tender) resistance
//     weight and aero components (from ALCO data)
static float
resistanceLoco (
    float ton,
    float mph)
{
    return 25 * (1 + ton) + 0.24 * mph * mph;
    // ??        ^  should be zero
}

// --------------------------------------------------------------------
// mean effective pressure - average cylinder pressure
//   accounting for cutoff
//   **** needs to be checked ****
static float
mep (
    float   cutoff)
{
    if (0 == cutoff)
        return 0;

 // float x = cutoff / 100;
    float x = cutoff;
    float y = 1-x;

    float p3 = x + y * (1+x) / 2;       // needs to be checked

#if 0
    if (0) {
        printf ("# mep:");
        printf (" cutoff %6.2f", cutoff);
        printf (", p3 %8.3f", p3);
        printf ("\n");
    }
#endif
    return  p3;
}

// --------------------------------------------------------------------
// determine throttle orifice
//     1st 25% opens smaller pilot valve
//     lst 75% opens main valve
//     both valve closes at 0% and 25%
float
throttleDia (
    int   x,
    float dia)
{
    // printf ("%s: throttle %d, dia %.2f\n", __func__, x, dia);

#if 0
    const float Kp  =  1/3;     // pilot valve size
    const float Km  =  3;       // main valve size
    const float P   = 25;       // pilot valve range

    float       k;

    if (x <= P)
        k = (Kp / Km) * x / P;
    else
        k = (x - P) / (100 - P);

#if 0
    printf ("# throttleDia: %6d %6.2f %6.2f %6.2f %6.2f\n",
            x, dia,  Kp, Km, P);
#endif

    return k * dia;

#else
    return dia * x / 100;
#endif
}

// ------------------------------------------------
// throttle flow (lb/hr) given orifice and pressure difference
// https://www.engineeringtoolbox.com/steam-flow-orifices-d_1158.html
static float
srFlow (
    float dia,
    float psi)
{
    const float K = 2000 / .25;
    const float P = 300;
#if 0
    float res = (K * dia * dia) * psi / P;

    printf ("%s: res %.1f, dia %.1f, psi %.1f, K %.1f, P = %.1f\n",
        __func__, res, dia, psi, K, P);

    return res;
#else
    return (K * dia * dia) * psi / P;
#endif
}

// --------------------------------------------------------------------
// *** ignored for now ... needed to manage steam consumption
//
#if 0
static void
boilerPressure (
    float dTsec)
{
    lbPmin  [T] = den [T-1] * vol [T-1] * 60 / dTsec
    lbSteam [T] = den [T-1] * vol [T-1]

    lbSteam [T] = flow [T-1]

    btu0 [T]    = (degF [T-1] + BtuVapor - FeedDegF) * (blrLbs - lbSteam [T])
    if (btu0 [T] < 0)
        btu0 [T] = 0

    btus [T]    = btu0 [T] + btuPmin * dTsec / 60
    if (btus [T] > Btu0)
        btus [T] = Btu0

    degF [T]    = (btus [T] / blrLbs) - BtuVapor + FeedDegF
    if (degF [T] < 212)
        degF [T] = 212

    psiBlr [T] = getSteamPsi(degF [T])
}
#endif

// --------------------------------------------------------------------
// update cylinder pressure
//   depending on throttle opening and pressure difference
//   resulting in st.mass flow (lb/hr) thru throttle,
//   accumulated st.mass (lb) in pipe, steam chest and cylinder,
//   given that volume resulting in a steam density and pressure
//   using steam tables for saturated (wet) steam
//
float subCycLst = 0;
float subCyc;
float dCyc;
float dCycLst;

static void
cylinderPressure (
    float dTsec)
{
    // -------------------------------------
    // consumption
    nCyc        = int(st.cyc);
    st.cyc     -= nCyc;

    // partial admission
    subCyc      = st.cyc < st.cut ? st.cyc : st.cut;

    // previous remaining addmission
    dCycLst     = st.cut - subCycLst;

    // total completion of prev + current cycle
    if (0 == nCyc)
        dCyc    = st.cut * (subCyc - subCycLst);
    else {
        dCyc    = st.cut * (nCyc - 1);
        dCyc   += st.cut - subCycLst;
        dCyc   += subCyc;
    }

    st.consume  = st.cylVol * dCyc;
    st.consume  = 0 > st.consume ? 0 : st.consume;

    subCycLst   = subCyc;

    // -------------------------------------
    // update flow (lb/hr) thru throttle
    st.thrDia   = throttleDia(st.thr, pEng->pipeDia/2);
    st.flw     = srFlow(st.thrDia, pEng->psi - st.psiChst) * dTsec / 3600;
#if 1
    st.flow    += (st.flw - st.flow) / 2;
#else
    st.flow    +=  st.flw;
#endif

    // -------------------------------------
    // update amount of steam in steam chest 
    st.fill    += st.flow;                      // flow from throttle
    st.fill    -= st.consume;
    st.fill     = 0 > st.fill ? 0 : st.fill;

    // -------------------------------------
    // determine steam chest pressure
#define STCHEST_VOL     (1.5 * st.cylVol)
    st.den     = st.fill / STCHEST_VOL;

    st.psiChst = interp(st.den, StDenS, StPsi, ST, 0) - PsiStd;
    st.psiChst = st.psiChst > 0           ? st.psiChst : 0; 
    st.psiChst = st.psiChst < pEng->psi ? st.psiChst : pEng->psi;

    // determine cylinder pressure
    st.mep      = mep(st.cut);      // determine cyliner pressure !
    st.psiCyl   = st.mep * st.psiChst;

    static int hdr;
    if (DBG_CYLPRESS & debug)  {
        if (! hdr++ % 10)  {
            printf ("%s:", __func__);
            printf (" %6s", "dTsec");
            printf (" %6s", "cyc");
            printf (" %6s", "nCyc");
            printf (" %6s", "flw");
            printf (" %6s", "PSI");
            printf (" %6s", "psiCh");
            printf (" %6s", "flow");
            printf (" %6s", "fill");
            printf (" %6s", "consum");
            printf (" %6s", "den");
            printf (" %6s", "mep");
            printf (" %6s", "psiCh");
            printf ("\n");
        }

        printf ("%s:", __func__);
        printf (" %6.1f", dTsec);
        printf (" %6.1f", st.cyc);
        printf (" %6.0f", nCyc);
        printf (" %6.1f", st.flw);
        printf (" %6.1f", pEng->psi);
        printf (" %6.1f", st.psiChst);
        printf (" %6.1f", st.flow);
        printf (" %6.1f", st.fill);
        printf (" %6.1f", st.consume);
        printf (" %6.1f", st.den);
        printf (" %6.1f", st.mep);
        printf (" %6.1f", st.psiChst);
        printf ("\n");
    }
}

// --------------------------------------------------------------------
// standard equation for wheel rim (drawbar) force
//     accounting for cylinder quatering and efficiency
//     http://www.catskillarchive.com/rrextra/blclas.Html
float
cylForce (
    float  psiCyl,
    float  cylDia,
    float  cylStr,
    float  drvrDia)
{
    float  f =  0.85 * psiCyl * cylDia * cylDia * cylStr / drvrDia;

    if (0)
        printf ("# cylForce: %6.1f psiCyl, %6.1f cylDia, %6.1f cylStr, %6.1f drvrDia, %6.1f lbF\n",
            psiCyl, cylDia, cylStr, drvrDia, f);

    return f;
}

// --------------------------------------------------------------------
//
void
enginePr (
    int   hdr )
{
    if (hdr)  {
        printf (" %5s", "cps");
        printf (" %5s", "cyc");
        printf (" %s",  "C");

        printf (" %5s", "cnsmp");

// #define STD
#define AVG
#ifdef STD
        printf (" %5s", "flw");
        printf (" %5s", "flow");
        printf (" %5s", "fill");
        printf (" %6s", "den");

        printf (" %6s", "psiCh");
        printf (" %4s", "MEP");
        printf (" %6s", "psiCyl");
#elif defined(AVG)
        printf (" %6s", "lbFloc");
#else
        printf (" %6s", "dCyl");
        printf (" %6s", "dCylLst");
        printf (" %6s", "subCyl");
#endif
    }

    else {
        printf (st.cps < 10 ? " %5.3f" : " %5.1f", st.cps);
        printf (" %5.3f", st.cyc);
        printf (" %d",    nCyc);
        printf (" %5.3f", st.consume);


#ifdef STD
        printf (" %5.3f", st.flw);
        printf (" %5.3f", st.flow);
        printf (" %5.3f", st.fill);
        printf (" %6.4f", st.den);

        printf (" %6.1f", st.psiChst);
        printf (" %4.2f", st.mep);
        printf (" %6.1f", st.psiCyl);

        if (st.slip)
            printf (" Slip");
#elif defined(AVG)
        printf (" %5.3f", st.lbFloco);
#else
        printf (" %5.3f", dCyc);
        printf (" %5.3f", dCycLst);
        printf (" %5.3f", subCyc);

#endif
    }
}

// --------------------------------------------------------------------
// update the state resulting in speed
float
engineTe (
    float dTsec,
    float fps,
    int   throttle,
    int   cutoff )
{
    if (NULL == pEng)  {
        printf ("%s: ERROR - uninitialized loco\n", __func__);
        exit (1);
    }

    if (1 == debug)
        printf (" %s: dTsec %.1f, throttle %d, cutoff %d\n",
                __func__, dTsec, throttle, cutoff);

    st.time    += dTsec;

    st.cut      = cutoff / 100.0;
    st.thr      = throttle;

    // update cyc
    st.cycLst   = st.cyc;
    st.cps      = fps * st.revPft * st.cycPrev;
    st.cyc     += st.cps * dTsec;

    // correct rpm for slip
    if (st.slip)  {
        st.cps = 10;
    }

    // update forces
#if 0
    cylinderPressure(dTsec);

    st.lbFloco  = cylForce(st.psiCyl,
        pEng->cylDia, pEng->cylStr, pEng->drvrDia) * mep(st.cut);
#else
    st.lbFloco  = avgForce(mph, cutoff) * pEng->psi * st.cylArea
                    * st.thr / 100.;
#endif

    // correct TE for slip
#if 1
    if (st.slip)  {
        st.te = st.slpTe;
        if (st.slpTe > st.lbFloco)  {
            st.te  = st.lbFloco;
            st.slip = 0;
        }
    }
    else if (st.maxTe < st.lbFloco)  {
        st.te = st.slpTe;
        st.slip= 1;
    }
    else
#endif
        st.te  = st.lbFloco;

    // --------------------------------------
    // handle resistive forces at standstill and in reverse

    st.locFres   = resistanceLoco(pEng->wtAdhT, st.mph);

    st.lbFnet    = 0;
    if (st.te > st.locFres)
        st.lbFnet    = st.te - st.locFres;

#if 0
    printf (" %s:",            __func__);
# if 0
    printf (" mph %4.2f",      mph);
    printf (" cutoff %4.2f",   cutoff);
    printf (" avgForce %4.2f", avgForce(mph, cutoff));

# else
 // printf (" PSI %3d",      pEng->psi);
 // printf (", cylArea %6.2f", st.cylArea);

    printf (", thr %6.0f",     st.thr);
    printf (", lbFloco %6.0f", st.lbFloco);
    printf (", locFres %6.0f", st.locFres);
    printf (", te %6.0f",      st.te);
    printf (", maxTe %6ld",    st.maxTe);
 // printf (", wtAdh %6d",   pEng->wtAdhT);
    printf (", lbFnet %6.0f",  st.lbFnet);
    printf ("  %s",            pEng->name);
# endif
    printf ("\n");
#endif

    return st.lbFnet;
}

// --------------------------------------------------------------------
// reset state

void
engineRst () {
#if 0
    st.fill     = st.cylVol * interp(PsiStd, StPsi, StDenS, ST, 0);
#else
    st.fill     = 0;        // ******** ^ delete
#endif
    st.psiChst  = 0;
    st.vol      = 0;
    st.fps      = 0;

    printf ("%s:",           __func__);
    printf ("  cylVol %.2f", st.cylVol);
    printf (", PSI %d",      pEng->psi);
    printf (", ftPrev %.2f", M_PI * pEng->drvrDia / 12);
    printf (", revPft %.3f", 1 / (M_PI * pEng->drvrDia / 12));
    printf (", cycPft %.3f", st.revPft * st.cycPrev);
    printf ("  %s",          pEng->name);
    printf ("\n");
}

// --------------------------------------------------------------------
void
engineInit (void)
{
    memset ((char*) &st, 0, sizeof(State_t));

    st.cylArea  = M_PI * (pEng->cylDia/2) * (pEng->cylDia/2);
    st.cylVol   = st.cylArea * pEng->cylStr * CuFtPcuIn;
    st.cycPrev  = 2 * pEng->numCyl;

    st.vol      = st.cylVol / 2;
    st.maxTe    = 0.25 * LbPton * pEng->wtAdhT;
    st.slpTe    = st.maxTe / 5;

    st.revPft   = 1 / (M_PI * pEng->drvrDia / 12);

#if 1
    printf ("%s: loco %s, wt %d, maxTe %ld\n",
        __func__, pEng->name, pEng->wtAdhT, st.maxTe);
#endif

    engineRst ();
}
