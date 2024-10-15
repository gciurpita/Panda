
#include <Arduino.h>

#include "pins.h"
#include "buttons.h"
#include "keypad.h"
#include "koala.h"
#include "menu.h"
#include "vars.h"

enum {
    B_MODE = 10,
    B_F0   = 50,
    B_F1   = 51,
    B_F2   = 52,

    B_F3   = 53,
    B_F4   = 54,
    B_F5   = 55,
    B_F6   = 56,

    B_F7   = 57,
    B_F8   = 58,
    B_F9   = 59,
    B_F10  = 60,

    B_MENU = 13,
    B_SEL  = 23,
    B_UP   = 33,
    B_DN   = 43,
};

// -----------------------------------------------------------------------------
// keypad
// 
//                      + + + +   <------ pin 14 / IO-13 / KB_R0
//                      + + + +   <------ pin 13 / IO-12 / KB_R1
//                      + + + +   <------ pin 12 / IO-14 / KB_R2
//                      + + + +   <------ pin 11 / IO-27 / KB_R3
//                                
//                      | | | |----> pin 21 / IO-15 / KB_C3
//                      | | |------> pin 22 / IO-02 / KB_C2
//                      | |--------> pin 24 / IO-00 / KB_C1
//                      |----------> pin 25 / IO-04 / KB_C0
//

// define the pins in the row and column
byte kpCols[] = {  KB_C0, KB_C1, KB_C2, KB_C3 };
#define KP_COL_SIZE sizeof(kpCols)

byte kpRows[] = { KB_R0, KB_R1, KB_R2, KB_R3 };
#define KP_ROW_SIZE sizeof(kpRows)

// button translation into cab bus codes
char keyCodes [KP_ROW_SIZE] [KP_COL_SIZE] = {

    { B_MODE, B_F3,   B_F7,   B_MENU },
    { B_F0,   B_F4,   B_F8,   B_SEL  },
    { B_F1,   B_F5,   B_F9,   B_UP   },
    { B_F2,   B_F6,   B_F10,  B_DN   },
};

#define N_BUTTONS 16

bool butState   [N_BUTTONS] = {};
bool butPress   [N_BUTTONS] = {};

// -----------------------------------------------------------------------------
// scans for a keypress and translates using spcified tbl
static char getKey (char tbl [KP_ROW_SIZE] [KP_COL_SIZE] )
{
    byte    row  = 0;
    byte    col  = 0;
    int     code = 0;

    if (NO_KEY != keyscan (& row, & col))
    {
        code = tbl [row][col];

        if (DBG_BUT & debug)
            printf ("%s: code %d, row %x, col %x\n",
                __func__, code, row, col); 
    }

    return code;
}

// ---------------------------------------------------------
// process button presses
unsigned long msecMenu = 0;

void
buttonFuncs (
    int button)
{
    static int           buttonLst = 0;

    if (buttonLst == button)
        return;
    buttonLst = button;

    switch (button)  {
    case 0:             // no button press
        break;

    case 10:
        state ^= ST_DVT;
        break;

    case B_MENU:
        menu (M_MENU);
        state |= ST_MENU;
        msecMenu = msec;
        break;

    case B_SEL:
        menu (M_SEL);
        msecMenu = msec;
        break;

    case B_UP:
        menu (M_UP);
        msecMenu = msec;
        break;

    case B_DN:
        menu (M_DN);
        msecMenu = msec;
        break;

    default:
        jmriFuncKey (button - B_F0, FUNC_TGL);
        break;
    }
}

// -------------------------------------
void buttonsChk (void)
{
    button = getKey (keyCodes);
    buttonFuncs (button);

#define MENU_TIMEOUT    4000
    if (msec - msecMenu > MENU_TIMEOUT)
        state &= ~ST_MENU;
}

// -------------------------------------
void buttonsInit (void)
{
    keypadSetup (kpCols, KP_COL_SIZE, kpRows, KP_ROW_SIZE);
}

