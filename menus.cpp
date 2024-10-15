// tables describing menu options

#include <stdlib.h>
#include <stdint.h>

#include "cfg.h"
#include "menu.h"
#include "menus.h"
#include "vars.h"

const char *sOnOff [] = { "Off", "On" };
const char *sWt []    = { "Light", "Low", "Med", "Heavy" };

// -------------------------------------------------------------------
// parameters
P_t pTon = { "Tonnage", &tonnage, 0, 3, sWt,    { __,   __,  up, dn,  dspP }};
P_t pLoc = { "Loco",    &locoIdx, 0, 0, NULL,   { __,  sft, inc, dec, dspV }};
P_t pCar = { "# Cars",  &cars,    0, 0, NULL,   { __,  sft, inc, dec, dspV }};

P_t pAd0 = { "Adr[0]", &locos[0].adr, 0, 0, & locos[0].adr, {sel, sft, inc, dec, dspV}};
P_t pAd1 = { "Adr[1]", &locos[1].adr, 0, 0, & locos[1].adr, {sel, sft, inc, dec, dspV}};
P_t pAd2 = { "Adr[2]", &locos[2].adr, 0, 0, & locos[2].adr, {sel, sft, inc, dec, dspV}};
P_t pAd3 = { "Adr[3]", &locos[3].adr, 0, 0, & locos[3].adr, {sel, sft, inc, dec, dspV}};

P_t pAd4 = { "Adr[4]", &locos[4].adr, 0, 0, & locos[4].adr, {sel, sft, inc, dec, dspV}};
P_t pAd5 = { "Adr[5]", &locos[5].adr, 0, 0, & locos[5].adr, {sel, sft, inc, dec, dspV}};
P_t pAd6 = { "Adr[6]", &locos[6].adr, 0, 0, & locos[6].adr, {sel, sft, inc, dec, dspV}};
P_t pAd7 = { "Adr[7]", &locos[7].adr, 0, 0, & locos[7].adr, {sel, sft, inc, dec, dspV}};

P_t pAd8 = { "Adr[8]", &locos[8].adr, 0, 0, & locos[8].adr, {sel, sft, inc, dec, dspV}};
P_t pAd9 = { "Adr[9]", &locos[9].adr, 0, 0, & locos[9].adr, {sel, sft, inc, dec, dspV}};

#if 0
P_t pHos = { "Host",  (int*)host, 0, 0,  NULL, { __,   sfA, inA, deA, dspA }};
P_t pPrt = { "Port",  &port,      0, 0,  NULL, { __,   sft, inc, dec, dspV }};

P_t pSsd = { "SSID",  (int*)ssid, 0, 0,  NULL, { __,   sfA, inA, deA, dspA }};
P_t pPsw = { "Pass",  (int*)pass, 0, 0,  NULL, { __,   sfA, inA, deA, dspA }};
#endif

// -------------------------------------------------------------------
// menus
#if 0
Menu_t menuComm [] = {
    { "Host",   "",      T_STR,    (void*) & pHos },
    { "Port",   "",      T_PARAM,  (void*) & pPrt },

    { "SSid",   "",      T_STR,    (void*) & pSsd },
    { "Pass",   "",      T_STR,    (void*) & pPsw },
    { NULL,     NULL,    T_NULL,   NULL },
};
#endif

// -------------------------------------
// DCC loco addresses
Menu_t menuAdr [] = {
    { "ADDR 0", "",      T_LIST,   (void*) & pAd0 },
    { "ADDR 1", "",      T_LIST,   (void*) & pAd1 },
    { "ADDR 2", "",      T_LIST,   (void*) & pAd2 },
    { "ADDR 3", "",      T_LIST,   (void*) & pAd3 },
    { NULL,     NULL,    T_NULL,   NULL },
};

// -------------------------------------
// main menu
Menu_t menuMain [] = {
 // { NULL,      NULL,   T_NONE,   NULL },
 // { "Loco",    "Addr", T_MENU,   (void*) menuAdr, &loco },
#if 0
    { "Comm",    "Cfg",  T_MENU,   (void*) menuComm },
#endif
    { "Cars",    "",     T_PARAM,  (void*) & pCar },
    { "Loco",    "Addr", T_MENU,   (void*) menuAdr },
    { "Loco",    "Idx",  T_PARAM,  (void*) & pLoc },
    { "Tonnage", "not used",     T_PARAM,  (void*) & pTon },
    { "Options", "",     T_NONE,   NULL },
    { "Version", version, T_NONE,   NULL },
    { "",        "",     T_NULL,   NULL },
};

// menuTop makes menuMain accessible externally
Menu_t *menuTop = & menuMain [0];
