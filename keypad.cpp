// keypad

#include "keypad.h"
#include "koala.h"  // debug

#undef DEBUG

static byte* _kpCols = NULL;
static byte* _kpRows = NULL;

static unsigned int _kpColSize = 0;
static unsigned int _kpRowSize = 0;

// ---------------------------------------------------------
void
keypadSetup (
    byte*           kpCols,
    unsigned int    kpColSize,
    byte*           kpRows,
    unsigned int    kpRowSize )
{
    _kpCols     = kpCols;
    _kpColSize  = kpColSize;
    _kpRows     = kpRows;
    _kpRowSize  = kpRowSize;

    for (unsigned int c = 0; c < _kpColSize; c++)
    {
        pinMode (_kpCols [c], INPUT_PULLUP);
        if (DBG_KEYPAD & debug)
            printf (" %s: col pin %2d\n", __func__, _kpCols [c]);
    }

    for (unsigned int r = 0; r < _kpRowSize; r++)
    {
        digitalWrite (_kpRows [r], LOW);
        pinMode (_kpRows [r], INPUT);
        if (DBG_KEYPAD & debug)
            printf (" %s: row pin %2d\n", __func__, _kpRows [r]);
    }
}

// ---------------------------------------------------------
// scan keypad rows and cols for keypress

#define DEB_PER  10

int
keyscan (
    byte*   row,
    byte*   col )
{
    for (unsigned int r = 0; r < _kpRowSize; r++)
    {
        pinMode (_kpRows [r], OUTPUT);

        if (DBG_KEYPAD & debug)
            delay (500);

        for (unsigned int c = 0; c < _kpColSize; c++)
        {
            if (! digitalRead (_kpCols [c]))
            {
                pinMode (_kpRows [r], INPUT);

                *row = r;
                *col = c;
                int  res = (r << 4) | c;

                if (DBG_KEYPAD & debug)  {
                    Serial.print ("keyscan: row ");
                    Serial.print (r);
                    Serial.print (", col");
                    Serial.print (c);
                    Serial.print (", res ");
                    Serial.println (res);
                }

                return res;
            }
        }

        pinMode (_kpRows [r], INPUT);
    }

    return NO_KEY;
}

// ---------------------------------------------------------
// return debounced key

int
keypad   (
    byte*   row,
    byte*   col )
{
    static char     state = NO_KEY;
    static char     pend  = NO_KEY;
    static uint32_t ms    = 0;

    char            key   = keyscan (row, col);

    if (pend != key)
    {
        pend = key;
        ms   = millis ();

#ifdef DEBUG
        Serial.print ("keypad: new key ");
        Serial.println (key, HEX);
#endif
    }
    if (DEB_PER < (millis () - ms))
    {
        if (state != pend)
        {
#ifdef DEBUG
            Serial.print ("keypad: debounced key ");
            Serial.println (pend, HEX);
#endif

            return state = pend;
        }
    }

    return NO_KEY;
}

// -----------------------------------------------
// check for button held down at startup

int
keyhold (
    byte*   row,
    byte*   col,
    int     minCnt )
{
    int     cnt = 0;
    byte    key = NO_KEY;
    byte    c;

    for (int n = 100; n > 0; n--)
    {
        c   = keyscan (row, col);
        if (NO_KEY != c)
        {
            cnt += key == c ? 1 : 0;
            key  = c;
        }
    }

    if (minCnt > cnt)
    {
        return NO_KEY;
    }

    return c;
}
