// serial command processor
// mode: cfg, cmd ad pins

#include <Arduino.h>

#include "brakes.h"
#include "cfg.h"
#include "file.h"
#include "koala.h"
#include "pcRead.h"
#include "vars.h"

enum { CmdMode, CfgMode, PinMode };

static int _mode    = CmdMode;
static int _modeLst = CmdMode;

// -----------------------------------------------------------------------------
static void
_dispVars (
    Stream &Serial)
{
    Vars_s   *v = pVars;

    sprintf (s, "%s:", __func__);
    Serial.println (s);

    for (int i = 0; 0 != v->p; i++, v++)  {
        sprintf (s, "    %6d  %s\n", *(v->p), v->desc);
        Serial.print (s);
    }
}

// -----------------------------------------------------------------------------
static void
_cmdModeHelp (
    Stream &Serial)
{
    Serial.println ("\ncmdMode:");
    Serial.println ("   # B - set button #");
    Serial.println ("   # b - set air-brake position");
    Serial.println ("   # c - set cars to #");
    Serial.println ("     D - set debug to #");
    Serial.println ("     d - display config variables");
    Serial.println ("     E - display configuration variables");
    Serial.println ("     e - switch to cfgMode");
    Serial.println ("     F - list SPIFFS files");
    Serial.println ("     f - toggle decoder function #");
    Serial.println ("   # i - set independent brake to #");
    Serial.println ("     L - load configuration");
    Serial.println ("   # l - set loco address to #");
    Serial.println ("     p - switch to pin mode");
    Serial.println ("     R - reset");
    Serial.println ("   # r - set reverser #");
    Serial.println ("     S - save configuration");
    Serial.println ("   # t - set throttle #");
    Serial.println ("     v - display state variables");
    Serial.println ("     V - print version");
    Serial.println ("     X - delete SPIFF file");
}

// -----------------------------------------------------------------------------
// process single character commands from the PC
static int
_cmdMode (
    Stream &Serial )
{
    static int  val  = 0;
    static int  func;
    static unsigned long msecFunc = 0;
           unsigned long msec     = millis ();

    char buf [40] = {};
    int  idx      = 0;

    // process function timeouts
#define MsecFunc    1000
    if (msecFunc && msec - msecFunc > MsecFunc)  {
        msecFunc = 0;
        jmriFuncKey ((unsigned int) func, FUNC_CLR);
    }

    // process serial input
    while (Serial.available())
        buf [idx++] = Serial.read ();

    if (0 == idx)
        return 0;

    for (int i = 0; i < idx; i++)  {
        char c = buf [i];

        switch (c)  {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            val = c - '0' + (10 * val);
            continue;     // only time val needs to be preserved

        case 'B':
            // reserved for buttons
            break;

        case 'b':   // air brake
            airPos = BRK_A_LAST > val ? val : BRK_A_EMER;
            break;

        case 'c':
            cars = val;
            break;

        case 'D':
            debug = val;
            break;

        case 'd':
            cfgDispAll (Serial);
            break;

        case 'E':
            cfgEdit (Serial);
            break;

        case 'F':
            fileDir ();
            break;

        case 'f':
            func     = val;
            msecFunc = msec;
            jmriFuncKey ((unsigned int) func, FUNC_TGL);
            break;

        case 'i':   // independent brake
            indPos = BRK_I_LAST > val ? val : BRK_I_APP_QUICK;
            break;

        case 'L':
            cfgLoad (cfgFname);
            break;

        case 'l':
            if (N_LOCO <= val)
                printf ("%s: Error - max loco %d\n", __func__, N_LOCO - 1);
            else  {
                locoIdx = val;
                printf ("%s: locoIdx %d\n", __func__, locoIdx);
            }
            val = 0;
            break;

        case 'p':
            _mode = PinMode;
            break;

        case 'R':
            exit (0);
            break;

        case 'r':
            reverser = MAX_REV < val ? MAX_REV : val;
            val = 0;
            break;

        case 'S':
            cfgSave (cfgFname);
            break;

        case 's':
            state = val;
            break;

        case 't':
            throttle = val;
            throttle = MAX_THR < throttle ? MAX_THR : throttle;
            val = 0;
            break;

        case 'V':
            Serial.print ("\nversion: ");
            Serial.println (version);
            break;

        case 'v':
            _dispVars (Serial);
            break;

        case 'X':
            fileDelete ();
            break;

        case 'Z':
            cfgDump (cfgFname);
            break;

        case '\n':      // ignore
        case '\r':      // ignore
            if (1 == idx)
                return 0;
            break;

        case '?':
            _cmdModeHelp (Serial);
            break;

        default:
            Serial.print ("unknown char ");
            Serial.println (c,HEX);
            break;
        }

        val = 0;
    }

    return 1;
}


// -----------------------------------------------------------------------------
static void
_pinModeHelp (
    Stream &Serial)
{
    Serial.println ("\npinMode:");
    Serial.println ("   # A - set analog pin #");
    Serial.println ("   # d - set debug to #");
    Serial.println ("   # I - set pin # to INPUT");
    Serial.println ("   # O - set pin # to OUTPUT");
    Serial.println ("   # P - set pin # to INPUT_PULLUP");
    Serial.println ("   # a - analogRead (pin #)");
    Serial.println ("   # c - digitalWrite (pin #, LOW)");
    Serial.println ("   # p - analogWrite (analogPin, #)");
    Serial.println ("     q - switch to cmd _mode");
    Serial.println ("   # r - digitalRead (pin #)");
    Serial.println ("   # s - digitalWrite (pin #, HIGH)");
    Serial.println ("   # t - toggle pin # output");
    Serial.println ("     v - print version");
}

// -----------------------------------------------------------------------------
// process single character commands from the PC
static int
_pinMode (
    Stream &Serial )
{
    static int  analogPin = 0;
    static int  val  = 0;

    if (Serial.available()) {
        char c   = Serial.read ();

        switch (c)  {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            val = c - '0' + (10 * val);
            return 1;     // only time val needs to be preserved

        case 'A':
            analogPin = val;
            Serial.print   ("analogPin = ");
            Serial.println (val);
            val = 0;
            break;

        case 'a':
            Serial.print   ("analogRead: ");
            Serial.println (analogRead (val));
            val = 0;
            break;

        case 'c':
            digitalWrite (val, LOW);
            Serial.print   ("digitalWrite: LOW  ");
            Serial.println (val);
            val = 0;
            break;

        case 'd':
            debug = val;
            val   = 0;
            break;

        case 'I':
            pinMode (val, INPUT);
            Serial.print   ("pinMode ");
            Serial.print   (val);
            Serial.println (" INPUT");
            val = 0;
            break;

        case 'O':
            pinMode (val, OUTPUT);
            Serial.print   ("pinMode ");
            Serial.print   (val);
            Serial.println (" OUTPUT");
            val = 0;
            break;

        case 'P':
            pinMode (val, INPUT_PULLUP);
            Serial.print   ("pinMode ");
            Serial.print   (val);
            Serial.println (" INPUT_PULLUP");
            val = 0;
            break;

        case 'p':
         // analogWrite (analogPin, val);
            Serial.print   ("analogWrite: pin ");
            Serial.print   (analogPin);
            Serial.print   (", ");
            Serial.println (val);
            val = 0;
            break;

        case 'q':
            _mode = CmdMode;
            break;

        case 'r':
            Serial.print   ("digitalRead: pin ");
            Serial.print   (val);
            Serial.print   (", ");
            Serial.println (digitalRead (val));
            val = 0;
            break;

        case 's':
            digitalWrite (val, HIGH);
            Serial.print   ("digitalWrite: HIGH ");
            Serial.println (val);
            val = 0;
            break;

        case 't':
            digitalWrite (val, ! digitalRead (val));
            val = 0;
            break;

        case 'v':
            Serial.print ("\nversion: ");
            Serial.println (version);
            break;

        case '\n':      // ignore
            break;

        case '?':
            _pinModeHelp (Serial);
            break;

        default:
            Serial.print ("unknown char ");
            Serial.println (c,HEX);
            break;
        }

        val = 0;
        return 1;
    }

    return 0;
}


// -----------------------------------------------------------------------------
// process single character commands from the PC
int
pcRead (
    Stream &Serial)
{
    // display help screen when switching modes
    if (_modeLst != _mode)  {
        _modeLst = _mode;

        switch (_mode)  {
        case CmdMode:
            _cmdModeHelp (Serial);
            break;

        case PinMode:
            _pinModeHelp (Serial);
            break;
        }
    }

    // invoke mode specific routine
    switch (_mode)  {
    case PinMode:
        return _pinMode (Serial);

    default:
    case CmdMode:
        return _cmdMode (Serial);
    }

    return 0;
}
