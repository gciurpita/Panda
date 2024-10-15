
#include "vars.h"

const char *version   = "240918a";


int      dccAdr;

int      brakeAir;
float    brakeAirPct;
int      brakeCfm;

int      brakeInd;
float    brakeIndPct;
int      button;

int      cars    = 1;
int      carLen  = 40;

int      cutoff;

int      dccSpd     = 0;
int      dccSpdLst  = 0;

int      dir;

int      encApos;
int      encBpos;

int      grX10;

int      engine;

unsigned long    msec;
int      mass;

float    mph;
float    mphLst;
float    mphToDcc;

int      reverser;

int      state;

int      throttle;
int      timeSec;
int      tonnage;

int      tractEff;
int      tractEffMax;

int      whistle;
int      wtCar   = 50;

// -----------------------------------------------------------------------------
// list of vars to display

Vars_s vars [] = {
    { & brakeAir,    "brakeAir" },
 // { & brakeAirPct, "brakeAirPct" },
    { & brakeInd,    "brakeInd" },
 // { & brakeIndPct, "brakeIndPct" },
    { & brakeCfm,    "brakeCfm" },

    { & cars,     "cars" },
    { & carLen,   "carLen" },
    { & dccSpd,   "dccSpd" },
    { & dir,      "dir" },
 // { & mph,      "mph" },
    { & mass,     "mass" },
    { & reverser, "reverser" },
    { & state,    "state" },
    { & throttle, "throttle" },
    { & tractEff, "tractEff" },
    { & tractEffMax, "tractEffMax" },
    { & timeSec,  "timeSec" },
    { & tonnage,  "tonnage" },
    { & wtCar,    "wtCar" },
    { 0,        NULL },
};

Vars_s *pVars = & vars [0];

// -----------------------------------------------------------------------------
//  stored variables

char     name [MAX_CHAR] = "Koala Throttle";
int      locoIdx         = 0;

// WiFi and JMRI Server Definitions
char     ssid [MAX_CHAR] = "WiFi-ssid";
char     pass [MAX_CHAR] = "WiFi-password";

char     host [MAX_CHAR] = "192.168.0.41";
int      port            = 12080;


Loco_s locos [N_LOCO] = {
    { 100, 1.1, 0 },        // default, I-10
    { 200, 2.1, 1 },
    { 300, 3.1, 2 },
    { 400, 4.1, 3 },
    { 500, 3.2, 4 },
    { 600, 2.2, 5 },
    { 700, 1.2, 6 },
    { 800, 2.3, 7 },
    { 900, 3.3, 8 },
};

