// air & independent brakes implementation

#ifdef Sim
# include <stdio.h>
# include <math.h>

#else
# include "Arduino.h"
#endif

#include "brakes.h"
#include "vars.h"

const int  IndMax   = 100;
const int  IndMin   =   0;

double  IndPsiMax   = 45;

double  AtmPsi      = 14.7;
int     BrkLnPsiMax = 90;
int     BrkCylMax   = 25;
double  MainRsvrPsi = 160;
double  NBR         = 0.1;
int     SqFt        = 144;

double  BrkLnDia    = 1.5;
double  BrkLnRad    = (BrkLnDia/2);
double  BrkLnArea   = (M_PI * BrkLnRad * BrkLnRad);

double  BrkRsvrVol  = 1.5;

int     airPos;
int     indPos;

double  equPsi;

double  brkLnPsi;
double  brkLnPsi_0  = 0;
double  brkLnPsiLst = 0;
double  brkLnPsiMin = 0;

double  brkLnFil    = 6;          // gross estimate

double  brkFlRat    = 0;
double  brkFilMax;

double  brkLnVol;
double  brkRsvrVol;
double  brkTotVol;

double  airPct;
double  dBrkLnPsi;
double  dBrkLnPsiInd;           // holding position
double  indPsi =   0;
double  indK;

double  timeMsec    = 0;

bool    dbg         = false;

// ---------------------------------------------------------
// air model dependent
double   rateRel    =  100;      // cu.ft. / min
double   rateSvc    =  -10;
double   rateEmer   = -300;

int     dropSvc1;
int     dropSvc2;

const char *  desc;

#if 0
const int BRK_A_REL  = 0;
const int BRK_A_RUN  = 1;
const int BRK_A_HOLD = 2;
const int BRK_A_LAP  = 3;
const int BRK_A_SVC  = 4;
const int BRK_A_EMER = 5;

const int BRK_A_SVC1 = 7;
const int BRK_A_SVC2 = 8;
const int BRK_A_SVC3 = 9;
#endif

const char * airBrkStr [] = {
    "REL",
    "RUN",
    "HOLD",
    "LAP",
    "SVC",
    "EMER",
    "ovrFlw",
};

// independent model dependent
double   relQuick  = -20;       // cu.ft. / min
double   relSlow   =  -6;
double   appSlow   =   6;
double   appQuick  =  20;

#if 0
const int BRK_I_REL_QUICK  = 0;
const int BRK_I_RUN        = 1;
const int BRK_I_LAP        = 2;
const int BRK_I_APP_SLOW   = 3;
const int BRK_I_APP_QUICK  = 4;
#endif

// terms from 1946 Victoria Railways Air Brakes pg 72
const char * indBrkStr [] = {
    "REL",
    "RUN",
    "LAP",
    "AP-S",
    "AP-Q",
    "ovrFlw",
};

// ---------------------------------------------------------
// determine brake position

#ifdef Board1
int stepsPdetent = 1;
#else
int stepsPdetent = 2;
#endif

typedef struct {
    int     nSteps;
    int     max;
    int     min;
    int     pos;
    int     posLst;
} Enc;

static Enc _encAir = { BRK_A_LAST * stepsPdetent, BRK_A_LAST * stepsPdetent };
static Enc _encInd = { BRK_I_LAST * stepsPdetent, BRK_I_LAST * stepsPdetent };

static int _brakeUpdate (
    int   encPos,
    Enc  *p )
{
    if (p->min > encPos)  {
        p->min = encPos;
        p->max = encPos + p->nSteps;
    }
    else if (p->max < encPos)  {
        p->max = encPos;
        p->min = encPos - p->nSteps;
    }

    p->posLst = encPos;

    return (encPos - p->min) / stepsPdetent;
}

// ---------------------------------------------------------
void
brakeReset ()
{
    printf ("%s:\n", __func__);
    if (0 < cars)  {
        brkLnPsi = brkLnPsi_0 = BrkLnPsiMax;
        brkLnFil = brkLnPsi * brkLnVol / AtmPsi;
        printf ("Brakes reset: %d cars, %.1f brkLnPsi\n",
                        cars, brkLnPsi);
    }
    else {
        brkLnPsi = brkLnPsi_0 = 0;
        brkLnFil = 0;
        printf ("Brakes reset: %d cars, %.1f brkLnPsi\n",
                        cars, brkLnPsi);
    }

    indPsi   = 0;
};

// -------------------------------------
void  brakeLnCapacity ()
{
    brkLnVol   = (0.1 + cars) * carLen * BrkLnArea / SqFt;
    brkRsvrVol = cars * BrkRsvrVol;
    brkTotVol  = brkLnVol + brkRsvrVol;
    brkFilMax  = brkTotVol * BrkLnPsiMax / AtmPsi;

#if 0
    printf ("%110s%s: ", "", __func__);
    printf (" brkLnVol %.1f",   brkLnVol);
    printf (" brkRsvrVol %.1f", brkRsvrVol);
    printf (" brkTotVol %.1f",  brkTotVol);
    printf (" brkFilMax %.1f",  brkFilMax);
    printf ("\n");
#endif
}

// -------------------------------------
//  brkLnPsi  = AtmPsi * brkLnFil / brkTotVol;
//
void airBrkFill (
    int  psi )
{
    printf ("%s: %d psi\n", __func__, psi);

    brkLnPsi = equPsi = psi;

    brakeLnCapacity ();
    brkLnFil = brkLnPsi * brkTotVol / AtmPsi ;

    airPct   = 0;
#if 0
    brkLnFil = brkLnVol * brkLnPsi / AtmPsi;

    printf (" brakeAirFill: psi %d, lnPsi %.1f, eqPsi %.1f, lnFil %6.1f\n",
                psi, brkLnPsi, equPsi, brkLnFil);
#endif
}

// ---------------------------------------------------------
// what i think the code should do:
//      brake handle in/decrease eq-reservoir press and
//      affect fill rate in REL/RUN and EMER
//
//      brk line psi tracks eq-rsvr psi regardless of brk position
//      i.e may decrese when REL
//
//      brkLnPsi_0 tracks local max used to determine brk valve psi
//
//      brklnPsi_0 should be set to brkLnPsi whenever ++
//
//      brk application % based on brkLnPsi_0 - brkLnPsi
//
//      then there's brake propogation

double
brakeAir (
    double  dTsec )
{
    airPos  = _brakeUpdate (encBpos, & _encAir);

    if (0 && dbg)
        printf ( " air: dTsec %4.1f, airPos %d %4s"
                 ", cars %d, lnPsi %.1f, eqPsi %.1f\n",
                dTsec, airPos, airBrkStr [airPos], cars, brkLnPsi, equPsi);

    if (0 == cars)
        return 0;

    double perMin = dTsec / 60.0;

    brkLnPsiLst = brkLnPsi;
    brkFlRat    = 0;

    switch (airPos)  {
    case BRK_A_REL:
     // brkFlRat   = rateRel / 60;
        if (equPsi < BrkLnPsiMax)
            equPsi    += 2 * dTsec;                     // increase rsvr

        if (equPsi < brkLnPsi)
            brkFlRat   = -(brkLnPsi/6) / 60;            // cont. to decrease
        else if (equPsi > brkLnPsi)
            brkFlRat   = (MainRsvrPsi - brkLnPsi) / 60; // higher pressure
        break;

    case BRK_A_RUN:
     // brkFlRat   = rateRel / 60;
    /// brkFlRat   = (BrkLnPsiMax - brkLnPsi) / 60;     // less pressure
     ///brkLnPsi_0 = brkLnPsi;
    /// equPsi     = brkLnPsi;
    ///
        if (equPsi < BrkLnPsiMax)
            equPsi    += 2 * dTsec;                     // increase rsvr

        if (equPsi < brkLnPsi)
            brkFlRat   = -(brkLnPsi/6) / 60;            // cont. to decrease
        else if (equPsi > brkLnPsi)
            brkFlRat   = (BrkLnPsiMax - brkLnPsi) / 60; // higher pressure
        break;

    case BRK_A_HOLD:        // ???
        break;

    case BRK_A_SVC:
        if (equPsi > brkLnPsiMin)
            equPsi    -= 2 * dTsec;

        if (equPsi < brkLnPsi)
            brkFlRat   = -(brkLnPsi/6) / 60;
        else if (equPsi > brkLnPsi)
            brkFlRat   = +(brkLnPsi/6) / 60;
     // brkFlRat   = 3 * rateSvc / 60;
        break;

    case BRK_A_EMER:
    /// brkLnPsiMin = 0;
     // brkFlRat    = rateEmer / 60;
        brkFlRat   = -brkLnPsi *3 / 60;
        equPsi    -= 50 * dTsec;
        break;

    case BRK_A_LAP:
    default:
        break;
    }

    // -----------------------------------------------
    // update brake line and reservoir capacity based on # cars
    brakeLnCapacity ();

    if (dbg)
        printf ("%110s%s: fil %.2f, rat %.2f",
                            "", __func__, brkLnFil, brkFlRat);

    // update brake line and reservoir fill
    if (0 > brkFlRat && equPsi < brkLnPsi)  {       // dropping
        brkLnFil  = brkLnVol * brkLnPsi / AtmPsi;
        brkLnFil += brkFlRat * dTsec;               // filRate negative

        if (0 > brkLnFil)
            brkLnFil = 0;
        brkLnPsi  = AtmPsi * brkLnFil / brkLnVol;
    }
    // need to fill air in both line and reservoir
    else if (0 < brkFlRat && brkLnPsi < BrkLnPsiMax)  {   // filling
        brkLnFil   = brkTotVol * brkLnPsi / AtmPsi;
        brkLnFil  += brkFlRat * dTsec;

        if (brkLnFil > brkFilMax)
            brkLnFil = brkFilMax;
        brkLnPsi  = AtmPsi * brkLnFil / brkTotVol;
    }

    // -----------------------------------------------
    // update brake line pressure
    if (0 == brkLnVol)
        brkLnPsi  = 0;

    // min/max limits
    if (brkLnPsi < brkLnPsiMin)
        brkLnPsi = brkLnPsiMin;

    if (BrkLnPsiMax < brkLnPsi)
        brkLnPsi = BrkLnPsiMax;

    // capture peak
    if (brkLnPsiLst <= brkLnPsi)
        brkLnPsi_0 = brkLnPsi;

    // -----------------------------------------------
    // update braking %
    dBrkLnPsi = brkLnPsi_0 - brkLnPsi;
    if (BRK_A_HOLD != airPos)
        dBrkLnPsiInd = dBrkLnPsi;

    if (0 < dBrkLnPsi)  {
        airPct = (100. * dBrkLnPsi / BrkCylMax);
        airPct = 100 < airPct ? 100 : airPct;
    }
    else if (brkLnPsi > brkLnPsiLst)  {
        airPct -= 100. / (7 * cars);   // brake air propogation
        if (0 > airPct)
            airPct = 0;
    }

    if (dbg)  {
        printf (", lnVol %.1f",  brkLnVol);
        printf (", lnFil %.1f",  brkLnFil);
        printf (", lnPsi %4.1f", brkLnPsi);
        printf (", airPct %4.1f", airPct);
    }

    if (dbg)
        printf ("\n");

    return NBR * airPct;
}

// ---------------------------------------------------------
// independent brake routine

double
brakeInd (
    double  dTsec )
{
    indPos = _brakeUpdate (encApos, & _encInd);

    switch (indPos)  {
    case BRK_I_REL_QUICK:
     // indPct += relQuick * dTsec;
        indPsi -= indPsi * 2 * dTsec;
        break;

    case BRK_I_RUN:
        if (0 != cars)
            indPsi -= ((indPsi - dBrkLnPsiInd)
                        * IndPsiMax / BrkCylMax) * dTsec;
        else
         // indPct += relSlow * dTsec;
            indPsi -= indPsi * dTsec;
        break;

    case BRK_I_APP_SLOW:
     // indPct += appSlow * dTsec;
        indPsi += ((IndPsiMax - indPsi) / 3) * dTsec;
        break;

    case BRK_I_APP_QUICK:
     // indPct += appQuick * dTsec;
        indPsi += (IndPsiMax - indPsi) * dTsec;
        break;

    default:
        break;
    }

    // limit
    if (indPsi > IndPsiMax)
        indPsi = IndPsiMax;

    if (indPsi < 0)
        indPsi = 0;

    bool    dbgInd = false;
    if (dbgInd) {
        printf ( "Brake ind: dTsec %.1f, pos %d", dTsec, indPos);
        printf ( ", indPsi %3.1f\n", indPsi);
    }

    return indPsi;
}

// ---------------------------------------------------------
void
brakePrHdr ()
{
    printf (" %4s", "Pos");
    printf (" %4s", "vol");
    printf (" %6s", "flRat");
    printf (" %6s", "fil");

    printf (" %6s", "eqPSI");

    printf (" %6s", "PSI");
    printf (" %5s", "Air");

    printf (" %4s", "Pos");
    printf (" %4s", "Ind");
}

// ---------------------------------------------------------
void
brakePr ()
{
    printf (" %4s",   airBrkStr [airPos]);
    printf (" %4.1f", brkLnVol);
    printf (" %6.2f", brkFlRat);
    printf (" %6.1f", brkLnFil);

    printf (" %6.1f", equPsi);

    printf (" %6.1f", brkLnPsi);
    printf (" %4.1f%%", airPct);

    printf (" %4s",   indBrkStr [indPos]);
    printf (" %4.1f", indPsi);
}
