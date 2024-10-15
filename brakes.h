#ifndef BRAKES_H
# define BRAKIES_HNG_H

enum {
    BRK_A_REL,
    BRK_A_RUN,
    BRK_A_HOLD,
    BRK_A_LAP,
    BRK_A_SVC,
    BRK_A_EMER,
    BRK_A_LAST = BRK_A_EMER
};

enum {
    BRK_I_REL_QUICK,
    BRK_I_RUN,
    BRK_I_LAP,
    BRK_I_APP_SLOW,
    BRK_I_APP_QUICK,
    BRK_I_LAST = BRK_I_APP_QUICK
};

extern int     airPos;
extern int     indPos;

extern const char * airBrkStr [];
extern const char * indBrkStr [];

extern const char * brakeStr [];

extern void airBrkFill (int  psi);

extern double brakeAir (double dTsec);
extern double brakeInd (double dTsec);

extern void brakePr    (void);
extern void brakePrHdr (void);
extern void brakeMdlPr (void);
extern void brakeReset ();

#endif
