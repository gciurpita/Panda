
#include <Arduino.h>

#include "pins.h"
#include "pots.h"
#include "vars.h"

#define ADC_MAX   4095

static int _sloLst;
static int _whiLst;
static int _revLst;
static int _thrLst;

// -----------------------------------------------------------------------------
// only update changes allowing cmds to overwrite
void
potChange (
    int  & var,
    int  & valLst,
    byte  pin )
{
    int val = 100 * analogRead (pin) / ADC_MAX;

    if (valLst != val)
        var = valLst = val;
}

// -----------------------------------------------------------------------------
void
potsRead (void)
{
    potChange (grX10,    _sloLst, ADC_SLOPE);
    potChange (whistle,  _whiLst, ADC_WHISTLE);
    potChange (reverser, _revLst, ADC_REV);
    potChange (throttle, _thrLst, ADC_THR);
}

