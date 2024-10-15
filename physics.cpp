// // physics model for locomotive including engine and brake models

#ifdef Sim
# include <stdio.h>
# include <math.h>

#else
# include "Arduino.h"
#endif

#include "avgForce.h"
#include "brakes.h"
#include "engine.h"
#include "eng.h"
#include "vars.h"

#define abs(x)  (0 > (x) ? (-x) : (x))
#define sgn(x)  (0 > (x) ? -1 : 1)

// -----------------------------------------------------------------------------
    long    msec0;
    long    msecLst;
    double  sec;

    long    hdr;

    double  acc;
    double  fps;
    double  fpsLst;
#if 0
    double  cps;
#endif
    double  ft;
    double  ftLst;

    bool    slip;

    // train parameters
    double  brkAirLbF;
    double  brkAirPct;
    double  brkIndLbF;
    double  cyc0;
    double  cyc1;
    int     wtTrT;

    double  grLbF;
    double  resLbF;     // total resistance
    double  trResLbF;   // train resistance
    double  teLbF;
    double  TeMax;

    double  blrPsi;

#if 0
    Cylinder  cyl0  = new Cylinder (eng, 0);
    Cylinder  cyl1  = new Cylinder (eng, 1);
#endif

    void physicsPr (long msec, int thr, int cutoff, double blrPsi, int prFlag );

    // ------------------------------------------------------------------------
    void phyReset (
        const char * locoName)
    {
        msec0  = msecLst = 0;
        fps    = ft = 0;
        TeMax  = pEng->wtAdhT * LbPton * 0.25;
        blrPsi = pEng->psi;

#if 0
        cyl0.reset (eng);
        cyl1.reset (eng);
#endif

        brakeReset ();

        printf (
            "# Physics reset: TeMax %.0f, blr Psi %.0f  %s\n",
                TeMax, blrPsi, pEng->name);
    }

    // ------------------------------------------------------------------------
    void
    phySetMph (
        int mph )
    {
        fps  = mph * 5280 / 3600.0;
    }

    // ------------------------------------------------------------------------
    void physics (
        long    msec,
#if 0
        int     thr,
        int     reverser,
        int     brkAirPos,
        int     brkIndPos,
        int     cars,
        double  gradePct,
#endif
        int     prFlag )
    {
        if (0 == msec0)  {
            msec0 = msecLst = msec;
            return;
        }

               sec    = (msec - msec0) / 1000.0;
        double dTsec  = (msec - msecLst) / 1000.0;

        // --------------------------------------
        tonnage = cars * 100;
        wtTrT   = pEng->wtLocoT + pEng->wtTndrT + tonnage;
        mass    = wtTrT * LbPton / 32.2;

        // --------------------------------------
#if 0
        if (0 == fps) {
            cyc0   = 0.1;
            ft     = 0;
        }
        else
            cyc0   = abs(ft) / (eng.drvCircFt / 2);
        cyc0   = cyc0 - (int)cyc0;

        cyc1   = 0.5 + cyc0;
        if (1.0 <= cyc1)
            cyc1 -= 1.0;

        if (1.0 < cyc0 || 1.0 < cyc1)  {
            printf (
                "update: cyc0 %.5f, cyc1 %.5f, ft %.2f %.2f\n",
                    cyc0, cyc1, ftLst, ft);
            System.exit (2);
        }
#endif

        int  cutoff = (int) abs(reverser);

        brkAirPct = brakeAir (dTsec) / 100;
        brkAirLbF = brkAirPct * tonnage * LbPton;
        brkIndLbF = (pEng->wtLocoT + pEng->wtTndrT) * brakeInd (dTsec);
        trResLbF  = (3 + exp(0.045* abs(mph))) * wtTrT;

        resLbF    = brkAirLbF + brkIndLbF + trResLbF;

        grLbF     =            wtTrT * LbPton * grX10 / 1000.;
#if 0
        teLbF     =   cyl0.force(dTsec, cyc0, cps, thr, cutoff, blrPsi)
                    + cyl1.force(dTsec, cyc1, cps, thr, cutoff, blrPsi);
#else
        teLbF     =   avgForce (mph, cutoff)
                        * pEng->psi * st.cylArea * throttle / 100.0;
#endif
        if (0 > reverser)
            teLbF = -teLbF;

        // combine forces and set direction of static forces
        double proLbF = teLbF - grLbF;
        double resLbF = trResLbF + brkAirLbF + brkIndLbF;

        // set direction of resistive force to oppose propulsive force
        if (0 == fps)  {
            if (abs(proLbF) > abs(resLbF))
                resLbF *= sgn(proLbF);   // in opposition to force
            else
                resLbF  = proLbF;        // limit holding force
        }
        else
            resLbF *= sgn(fps);          // in opposite to direction

        double netLbF = proLbF - resLbF;

        // apply forces
        ftLst  = ft;        // ????

        acc    = netLbF / mass;
        fpsLst = fps;
        fps   += acc * dTsec;

        // static forces should not cause movement
        double dDir = fpsLst * fps;         // negative if direction changed

   //   if (teLbF < resLbF && sgn(fps) != sgn(fpsLst))
        if ( (0 == fpsLst || 0 > dDir) && abs(proLbF) < abs(resLbF))
            acc = fps = 0;

#if 0
        cps    = abs(fps / (pEng->drvCircFt / 2));
#endif
        ft    += fps * dTsec;
        mph    = fps * 3600 / 5280.0;

        slip = (TeMax < teLbF);

        // debug and print
        if (16 == prFlag) {
            if (0 == (hdr++ % 10))  {
                printf ("# %6s",          "sec");
                printf (" %6s %6s %6s",   "fps", "acc", "net");
                printf (" %6s %6s",       "pro", "res");
                printf (" %6s %6s",       "te", "gr");
                printf (" %6s %6s %6s",   "tr", "ind", "airLbF");
                printf (" %6s %6s",       "air %", "ton");
                printf ("\n");
            }

            printf ("  %6.1f",              sec);
            printf (" %6.0f %6.0f %6.0f",   fps,    acc, netLbF);
            printf (" %6.0f %6.0f",         proLbF, resLbF);
            printf (" %6.0f %6.0f",         teLbF,  grLbF);
            printf (" %6.0f %6.0f %6.0f %6.2f %6d\n",
                trResLbF, brkIndLbF, brkAirLbF, brkAirPct, tonnage);
        }

        else if (8 == prFlag) {
            if (0 == (hdr++ % 10))
                printf ("# %6s %6s %6s %6s %6s %6s %6s %6s\n",
                            "sec", "te", "tr", "ind",
                            "mass", "acc", "fpsLst", "fps");
            printf (
                    "  %6.1f %6.0f %6.0f %6.0f %6.0f %6.2f %6.1f %6.2f\n",
                        sec, teLbF, trResLbF, brkIndLbF, mass, acc, fpsLst, fps);
        }

        else if (0 < prFlag)
            physicsPr (msec, throttle, cutoff, blrPsi, prFlag);

        msecLst = msec;
    }

    // ------------------------------------------------------------------------
    void physicsPr (
        long    msec,
        int     thr,
        int     cutoff,
        double  blrPsi,
        int     prFlag )
    {
#if 0
        if  ((msec/1000) != (msecLst/1000))  {
            printf ("#  sec  tons thr cut blr");
            printf ("  TE  TR   acc   fps    ft   mph   cps");
#else
        static int hdr;

        if (! (hdr++ % 10))  {
            printf ("#  sec  tons thr cut blr");
            printf ("  TE  TR   acc   fps    ft   mph");
#endif

#if 0
            if (0 != (1 & prFlag))
                cyl0.prHdr ();
            if (0 != (2 & prFlag))
                cyl1.prHdr ();
#endif
            if (0 != (4 & prFlag))
                brakePrHdr ();
            printf ("\n");
        }

#if 1
        printf ("%6.1f",  (msec - msec0) / 1000.);
#else

        int mins   =  msec / 60000;
        int secs   = (msec % 60000) / 1000;
        int tenths = (msec %  1000) / 10;
        printf ("%2d:%02d.%d",  mins, secs, tenths);
#endif
        printf (" %5d",   wtTrT);
        printf (" %3d",   thr);
        printf (" %3d",   cutoff);
        printf (" %3.0f", blrPsi);

        printf (" %3.0f", teLbF  / 1000);
        printf (" %3.0f", resLbF / 1000);

        printf (" %5.1f", acc);
        printf (" %5.1f", fps);
        printf (" %5.1f", ft);
        printf (" %5.1f", mph);

#if 0
        printf (" %5d",   pEng->psi);
        printf (" %5.1f", st.cylArea);
        printf (" %5.1f", avgForce (mph, cutoff / 100.0));
#endif
#if 0
        if (100 <= cps)
            printf (" %5.0f", cps);
        else
            printf (" %5.1f", cps);
#endif

#if 0
        if (0 != (1 & prFlag))
            cyl0.pr ();
        if (0 != (2 & prFlag))
            cyl1.pr ();
#endif
        if (0 != (4 & prFlag))
            brakePr ();

        printf ("\n");
    }

    // ------------------------------------------------------------------------
#if 0
    double getAcc     () { return acc; }
    const char * getAirLbl  (int i) { return brk.airBrkStr [i]; }
    const char * getIndLbl  (int i) { return brk.indBrkStr [i]; }

    double getBrkEqPsi() { return brk.equPsi; }
    double getBrkLnPsi() { return brk.brkLnPsi; }
    double getBrkInd()   { return brk.indPsi; }
    double getBrkAir()   { return brk.airPct; }

    double getCps     () { return cps; }
    double getCylPsi  (int id) {
        if (0 == id)
            return cyl0.getCylPsi ();
        return cyl1.getCylPsi ();
    }
 // const char * getEngName () { return eng.name; }
    double getFps     () { return fps; }
    double getMph     () { return mph; }
    double getResLb   () { return trResLbF; }
    double getTeLb    () { return teLbF; }
    int    getTonnage () { return tonnage;  }
#endif
