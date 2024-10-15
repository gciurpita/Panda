
#include <Arduino.h>

#include "encoder.h"
#include "pins.h"

// -----------------------------------------------------------------------------
//  table containing action indexed by inputs BA and state

typedef enum { ___, For, Rev, Skp, ActSize } Act_t;

const char* actStr [] = { "___", "For", "Rev", "Skp", "ActSize" };

Act_t  encAct [ActSize][ActSize] = {
// state  00   01   10   11      inputs
       { ___, Rev, For, Skp },  // 00
       { For, ___, Skp, Rev },  // 01
       { Rev, Skp, ___, For },  // 10
       { Skp, For, Rev, ___ },  // 11
};

int encAst  = 0;
int encBst  = 0;

// ------------------------------------------------
// read brake
inline void readEncoder (
    byte  dt,
    byte  clk,
    int & brkPos,
    int & encSt )
{
    byte  val = digitalRead (dt) << 1 | digitalRead (clk);

    switch (encAct [val][encSt])  {
    case For:
        brkPos++;
        break;

    case Rev:
        brkPos--;
        break;

    default:
        break;
    }

    encSt = val;

 // digitalWrite (LED, ! digitalRead (LED));
}

// -------------------------------------
void IRAM_ATTR isrEncA (void)
{
    readEncoder (Enc_A_Dt, Enc_A_Clk, encApos, encAst);
}

// -------------------------------------
void IRAM_ATTR isrEncB (void)
{
    readEncoder (Enc_B_Dt, Enc_B_Clk, encBpos, encBst);
}

// -------------------------------------
void
encoderInit (void)
{
    // encoders
    pinMode (Enc_A_Clk, INPUT_PULLUP);
    pinMode (Enc_A_Dt,  INPUT_PULLUP);

    attachInterrupt (Enc_A_Clk, isrEncA, CHANGE);
    attachInterrupt (Enc_A_Dt,  isrEncA, CHANGE);

    pinMode (Enc_B_Clk, INPUT_PULLUP);
    pinMode (Enc_B_Dt,  INPUT_PULLUP);

    attachInterrupt (Enc_B_Clk, isrEncB, CHANGE);
    attachInterrupt (Enc_B_Dt,  isrEncB, CHANGE);
}
