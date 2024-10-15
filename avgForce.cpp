// ratio of boiler pressure to siderod force
//    depending on speed, cutoff, siderod ang and backe-pressure

#include <stdio.h>

#include "avgForce.h"

#define ABS(x)   (0 > (x) ? -(x) : (x))

const float Cut0 = 20;
const float CutN = 80;
const float CutD =  5;

const int Ncut = 13;
struct Cyl {
    float mph;
    float psiRat [Ncut];
};

static int dbg = 0;

// ---------------------------------------------------------
Cyl cyl [] = {
//    mph   20%   25%   30%   35%   40%   45%   50%   55%   60%   65%   70%   75%   80%
 {  0.0, { 0.50, 0.60, 0.67, 0.73, 0.79, 0.84, 0.90, 0.92, 0.94, 0.97, 0.99, 1.00, 1.00 }},
 {  0.25, { 0.50, 0.60, 0.67, 0.73, 0.79, 0.84, 0.90, 0.92, 0.94, 0.97, 0.99, 1.00, 1.00 }},
 {  0.50, { 0.50, 0.60, 0.67, 0.73, 0.79, 0.84, 0.90, 0.92, 0.94, 0.97, 0.99, 1.00, 1.00 }},
 {  1.00, { 0.50, 0.59, 0.67, 0.73, 0.79, 0.84, 0.90, 0.92, 0.94, 0.96, 0.98, 0.99, 0.99 }},
 {  2.00, { 0.49, 0.58, 0.66, 0.72, 0.78, 0.83, 0.88, 0.89, 0.91, 0.93, 0.95, 0.96, 0.96 }},
 {  4.00, { 0.47, 0.55, 0.62, 0.68, 0.73, 0.77, 0.82, 0.82, 0.83, 0.85, 0.86, 0.86, 0.86 }},
 {  6.00, { 0.44, 0.52, 0.59, 0.64, 0.68, 0.72, 0.76, 0.76, 0.76, 0.78, 0.78, 0.77, 0.76 }},
 {  7.00, { 0.43, 0.51, 0.57, 0.62, 0.66, 0.69, 0.73, 0.73, 0.73, 0.74, 0.74, 0.74, 0.72 }},
 {  8.00, { 0.42, 0.50, 0.56, 0.61, 0.64, 0.67, 0.71, 0.70, 0.71, 0.72, 0.71, 0.70, 0.68 }},
 {  9.00, { 0.42, 0.49, 0.55, 0.59, 0.63, 0.66, 0.69, 0.68, 0.69, 0.69, 0.69, 0.67, 0.65 }},
 { 10.00, { 0.41, 0.48, 0.54, 0.58, 0.61, 0.64, 0.67, 0.66, 0.67, 0.67, 0.66, 0.65, 0.62 }},
 { 15.00, { 0.38, 0.45, 0.50, 0.54, 0.56, 0.58, 0.61, 0.59, 0.60, 0.59, 0.57, 0.55, 0.52 }},
 { 20.00, { 0.37, 0.43, 0.47, 0.51, 0.53, 0.55, 0.57, 0.56, 0.55, 0.54, 0.52, 0.49, 0.46 }},
 { 25.00, { 0.36, 0.41, 0.46, 0.49, 0.51, 0.52, 0.54, 0.53, 0.52, 0.51, 0.48, 0.45, 0.41 }},
 { 30.00, { 0.35, 0.40, 0.45, 0.48, 0.50, 0.51, 0.52, 0.51, 0.50, 0.48, 0.46, 0.42, 0.38 }},
 { 40.00, { 0.34, 0.39, 0.43, 0.46, 0.48, 0.49, 0.49, 0.49, 0.47, 0.45, 0.42, 0.38, 0.34 }},
 { 50.00, { 0.33, 0.38, 0.42, 0.45, 0.46, 0.47, 0.47, 0.47, 0.45, 0.43, 0.40, 0.36, 0.32 }},
 { 60.00, { 0.33, 0.38, 0.41, 0.44, 0.45, 0.46, 0.46, 0.46, 0.44, 0.42, 0.38, 0.34, 0.30 }},
};
const int Ncyl = sizeof(cyl)/sizeof(Cyl);

Cyl *pCyl = cyl;

// -------------------------------------
// return index of cutoff rounded down
inline cut2idx (
    float cut)
{
    int  col = (cut - Cut0) / CutD;
    if (Ncyl <= col)
        col--;
    return col;
}

// ---------------------------------------------------------
float interp (
    float x,
    float x0, float x1,
    float y0, float y1 )
{
    float res = y0 + (y1 - y0) * (x - x0) / (x1 - x0);

    if (dbg)  {
        printf ("   %s:", __func__);
        printf (" %5.2f %5.2f %5.2f", x, x0, x1);
        printf (", %5.2f",  (x - x0) / (x1 - x0));
        printf (" -- %5.2f %5.2f", y0, y1);
        printf (", %6.3f", res);
        printf ("\n");
    }
    return res;
}

// -------------------------------------
// get interpolated val based on cutoff
float cutMph (
    float cut,
    int   idx )
{
    Cyl   *p    = & cyl [idx];
    int    col  = cut2idx (cut);
    float  cut0 = Cut0 + col * CutD;
    float res = interp (cut,    cut0,              cut0 + CutD,
                                p->psiRat [col], p->psiRat [col+1]);

    if (dbg) {
        printf ("  %s:    ", __func__);
        printf (" %6.2f", cyl [idx].mph);
        printf (" %4.1f%%", cut);
        printf (" %2di",   idx);
        printf (" %2dc",   col);

        printf ("  ..");
        printf (" %5.2f", p->psiRat [col]);
        printf (" %5.2f", p->psiRat [col+1]);

        printf (", %6.3f", res);
        printf ("\n");
    }

    return res;
}

// -------------------------------------
float avgForce (
    float  mph,
    int    cut )
{
    mph    = ABS(mph);   
    cut    = ABS(cut);   
    Cyl *p = cyl;

    // find col index cut is within
    int  col = 10 * (cut - Cut0) / (CutN - Cut0);
         col = col == Ncyl-1 ? col-- : col;

    int  n;
    for (n = 0; n < Ncyl-1; n++, p++)  {
     // printf ("  %s: %2d %6.2f %6.2f\n", __func__, n, p->mph, mph);
        if (p->mph > mph)               // idx of val > mph
            break;
    }

    if (dbg) {
        printf (" %s: %6.2f %2.0f%%", __func__, mph, cut);
        printf (" %2dc", col);
        printf (" %2dr", n);
        printf ("\n");
    }

    float valLo = cutMph (cut, n-1);
    float valHi = cutMph (cut, n);
    float res  = interp (mph, cyl [n-1].mph, cyl [n].mph, valLo, valHi);

    if (dbg)
        printf ("  %s: %6.2f\n", __func__, res);

    return res;
}
