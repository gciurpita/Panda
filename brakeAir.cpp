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

    if (0 || dbg)
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
        brkLnPsi_0 = brkLnPsi;
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
        brkLnPsi_0 = brkLnPsi;
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
    // update brake line and reservoir fill
    brkLnVol   = (0.1 + cars) * carLen * BrkLnArea / SqFt;
    brkRsvrVol = cars * BrkRsvrVol;
    brkTotVol  = brkLnVol + brkRsvrVol;

    brkFilMax  = brkTotVol * BrkLnPsiMax / AtmPsi;

    // update brake line and reservoir fill
    if (0 > brkFlRat && equPsi < brkLnPsi)  {       // dropping
        printf ("... %s: droping, %.2f %.2f\n", __func__, brkFlRat, brkLnPsi_0);
        brkLnFil  = brkLnVol * brkLnPsi / AtmPsi;
        brkLnFil += brkFlRat * dTsec;               // filRate negative

        if (0 > brkLnFil)
            brkLnFil = 0;
        brkLnPsi  = AtmPsi * brkLnFil / brkLnVol;
    }
    // need to fill air in both line and reservoir
    else if (0 < brkFlRat && brkLnPsi < BrkLnPsiMax)  {   // filling
        printf ("... %s: filling, %.2f %.2f\n", __func__, brkFlRat, brkLnPsi_0);
        brkLnFil   = brkTotVol * brkLnPsi / AtmPsi;
        brkLnFil  += brkFlRat * dTsec;

        if (brkLnFil > brkFilMax)
            brkLnFil = brkFilMax;
        brkLnPsi  = AtmPsi * brkLnFil / brkTotVol;
    }

    // update brake line pressure
    if (0 == brkLnVol)
        brkLnPsi  = 0;

    // min/max limits
    if (brkLnPsi < brkLnPsiMin)
        brkLnPsi = brkLnPsiMin;

    if (BrkLnPsiMax < brkLnPsi)
        brkLnPsi = BrkLnPsiMax;

    // update braking %
    dBrkLnPsi = brkLnPsi_0 - brkLnPsi;
    if (BRK_A_HOLD != airPos)
        dBrkLnPsiInd = dBrkLnPsi;

    if (0 < dBrkLnPsi)  {
        printf ("... %s: brake apply\n", __func__);
        airPct = (int) (100 * dBrkLnPsi / BrkCylMax);
        airPct = 100 < airPct ? 100 : airPct;
    }
    else if (brkLnPsi > brkLnPsiLst)  {
        printf ("... %s: brake restore\n", __func__);
        airPct -= 100. / (7 * cars);   // brake air propogation
        if (0 > airPct)
            airPct = 0;
    }

    if (dbg)  {
        printf (", lnVol %.1f",  brkLnVol);
        printf (", lnFil %.1f",  brkLnFil);
        printf (", lnPsi %4.1f", brkLnPsi);
        printf (", airPct %4.1f", airPct);
        printf ("\n");
    }

    return NBR * airPct;
}
