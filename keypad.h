#ifndef KEYPAD_H
# define KEYPAD_H

# include "Arduino.h"

#define NO_KEY  0xff

void
keypadSetup (
    byte*           kpCols,
    unsigned int    kpColSize,
    byte*           kpRows,
    unsigned int    kpRowSize );

int
keyscan (
    byte*   row,
    byte*   col );

int
keypad (
    byte*   col,
    byte*   row );

int
keyhold (
    byte*   col,
    byte*   row,
    int     minCnt );

#endif
