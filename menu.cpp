// menu engine

#ifdef Sim
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

#else
# include <Arduino.h>
#endif

#include "cfg.h"
#include "file.h"
#include "koala.h"
#include "menu.h"
#include "menus.h"
#include "vars.h"
#include "utils.h"

static char s0 [MAX_CHAR+1];
static char s1 [MAX_CHAR+1];


int  digit;     // digit to edit in multi-digit value

// -----------------------------------------------------------------------------
// display two strings
void
disp (
    const char    *s0,
    const char    *s1 )
{
#if 0               // for simulation on laptop
    printf (Clear);
    printf ("    %-12s\n", s0);
    printf ("    %-12s\n", s1);
#else
    dispOled (0, 0, s0, s1, CLR);
#endif
}

// -----------------------------------------------------------------------------
// null routine
void
__ (void *)
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);
}

// -------------------------------------
// select the specific list item
void
sel (
    void *v)
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);

    P_t  *p  = (P_t*) v;
    int  *pV = (int *) p->p;
    *pV      = *p->param;
}

// -------------------------------------
// shift digit to right, not past NULL
void
sfA (
    void *v)
{
    P_t     *p = (P_t*) v;
    char    *s = (char *) p->param;

    if (DBG_MENU & debug)
        printf ("%s: '%c', digit %d\n", __func__, s [digit], digit);

    if (0 == s [digit] || (MAX_CHAR-2) == digit)
        digit = 0;
    else
        digit++;
}

// -------------------------------------
// shift digit to edit next digit to the right
void
sft (void *)
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);

    digit = MAX_DIGIT-1 > digit ? digit+1 : 0; 
}

// -------------------------------------
// decrement value of a character (' ' to '~')
void
deA (
    void  *v )
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);

    P_t  *p  = (P_t*) v;
    char *s  = (char*)p->param;
    char  c  = s [digit];
    
    if (0 == c)                  // appending
        c = '~';
    else if ('!' == c)  {
        c = 0;                  // replace last character w/ NULL
        if (0 != s [digit+1])   // don't insert NULL in middle of string
            c = '~';
    }
    else
        c--;

    s [digit] = c;
}

// -------------------------------------
// decrement a digit value
void
dec (
    void  *v )
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);

    P_t  *p  = (P_t*) v;

    sprintf (s0, "%05d", *p->param);
    char c = s0 [digit];

    c = '0' == c ? '9' : c-1;

    s0 [digit] = c;
    *p->param = atoi (s0);
}

// -------------------------------------
// increment value of a character (' ' to '~')
//     if NULL, replace with 'A'
//     if '~', replace with NULL at end of string
//     if '~', replace with '!' in middle of string
void
inA (
    void  *v )
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);

    P_t  *p  = (P_t*) v;
    char *s  = (char*)p->param;
    char  c  = s [digit];
    
    if (0 == c)                  // appending
        c = 'A';
    else if ('~' == c)  {
        c = '!';                // don't insert NULL in middle of string
        if (0 == s [digit+1])  {
            s [digit--] = 0;    // replace w/ null & move to previous char
            return;
        }
    }
    else
        c++;

    s [digit] = c;
}

// -------------------------------------
// increment a digit value
void
inc (
    void  *v )
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);

    P_t  *p  = (P_t*) v;

    sprintf (s0, "%05d", *p->param);
    char c = s0 [digit];

    c = ' ' == c ? '1' : ('9' > c ? c+1 : '0');
    s0 [digit] = c;
    *p->param = atoi (s0);
}

// -------------------------------------
// increment the value of a variable
void
up (
    void  *v )
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);

    P_t  *p  = (P_t*) v;
    *p->param = p->max > *p->param ? *p->param+1 : p->min;
}

// -------------------------------------
// decrement the value of a variable
void
dn (
    void  *v )
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);

    P_t  *p  = (P_t*) v;
    *p->param = p->min < *p->param ? *p->param-1 : p->max;
}

// -------------------------------------
void
dspA (
    void    *v )
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);

    P_t    *p = (P_t*) v;
    char   *str = (char*) p->param;
    char    s[MAX_CHAR] = {};
    char    t[MAX_CHAR] = {};

#define N_DISP_CHAR   20
#define N_SHIFT_CHAR  10
    int shift = (digit -5) / N_SHIFT_CHAR;
    int shDig = shift * N_SHIFT_CHAR;
#if 1
    strncpy (t, & str [shDig], N_DISP_CHAR);  // truncated string
#else
    strcpy (t, & str [shDig]);              // truncated string
#endif

    int  x1 = 0;
    if (0 < digit)  {
        strncpy (s, t, digit - shDig);
        x1 = fontStringWid (s);
    }
    display.drawString (0,  DISP_Y2, t);

#if 0
    printf ("%s: digit %d, shift %d, shDig %d, x1 %d %s, %s\n",
        __func__, digit, shift, shDig, x1, s, t);
#endif

    display.drawString (x1, DISP_Y3, "^");

    display.display ();
}

// -------------------------------------
void
dspV (
    void    *v )
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);

    P_t     *p = (P_t*) v;

    sprintf (s0, "%05d", *p->param);
    strcpy  (s1, "_____");
    s1 [digit] = '^';
    disp (s0, s1);
}

// -------------------------------------
void
dspP (
    void    *v )
{
    if (DBG_MENU & debug)  printf ("%s:\n", __func__);

    P_t     *p = (P_t*) v;
    char   **s = (char **) p->p;
    
    if (s)
        sprintf (s0, "%s - %s", p->text, s [*p->param]);
    else
        sprintf (s0, "%05d", *p->param);

    sprintf (s1, "____________________");
    s1 [3 + strlen (p->text)] = '^';

    disp  (s0, s1);
}

// -----------------------------------------------------------------------------
#define MAX_LVL 5
void
menu (
    MenuStim_t stim )
{
#if 0
    if (DBG_MENU & debug)
        printf (Clear);
#endif

    static int      lvl = 0;
    static Menu_t  *list  [MAX_LVL] = { menuTop };
    static int      idx   [MAX_LVL] = { };
    static P_t     *t   = NULL;

           Menu_t  *m   = list [lvl];
           Menu_t  *q   = & m [idx [lvl]];

    switch (stim) {
    case M_MENU:
        if (t)  {
            t->func.menu ((void*) t);
            t = NULL;
            digit = 0;
        }
        else if (0 < lvl)  {
            if (DBG_MENU & debug)  printf ("%s: M_MENU dec lvl\n", __func__);
            m   = list [--lvl];
            q   = & m [idx [lvl]];
        }

        cfgSave (cfgFname);
        break;

    case M_SEL:
        if (t)
            t->func.sel ((void*) t);

        else if (T_PARAM == q->type
              || T_STR == q->type
              || T_LIST == q->type)  {
            if (DBG_MENU & debug)  printf ("%s: M_SEL assign p\n", __func__);
            t = (P_t*) q->ptr;
        }

        else if (T_MENU == q->type)  {
            if (DBG_MENU & debug)  printf ("%s: M_SEL inc lvl\n", __func__);
            list [++lvl] = (Menu_t *) q->ptr;
            idx  [lvl]   = 0;
            m            = list [lvl];
            q            = & m [idx [lvl]];
        }
        break;

    case M_UP:
        if (t)
            t->func.up ((void*) t);
        else  {
            if (DBG_MENU & debug)  printf ("%s: M_SEL inc idx\n", __func__);
            idx [lvl] ++;
            q++;
            if (T_NULL == q->type)  {
                idx [lvl] = 0;
                q         = & m [idx [lvl]];
            }
        }
        break;

    case M_DN:
        if (t)
            t->func.dn ((void*) t);
        else if (0 < idx [lvl])  {
            if (DBG_MENU & debug)  printf ("%s: M_SEL dec idx\n", __func__);
            idx [lvl] --;
            q--;
            // don't know max idx
        }
        break;

    default:
        break;
    }

    if (t)
        t->func.disp (q->ptr);

    else if (T_PARAM == q->type) {
        if (DBG_MENU & debug)  printf ("%s: display T_PARAM\n", __func__);

        P_t *p = (P_t*) q->ptr;
        char   **s = (char **) p->p;
    
        if (s)
            sprintf (s1, "%s", s [*p->param]);
        else
            sprintf (s1, "%05d", *p->param);
        disp (p->text, s1);
    }

    else if (T_STR == q->type)  {
        if (DBG_MENU & debug)  printf ("%s: display T_STR\n", __func__);

        P_t *p = (P_t*) q->ptr;
        disp (p->text, (char*) p->param);
    }

    else if (T_LIST == q->type)  {
        if (DBG_MENU & debug)  printf ("%s: display T_LIST\n", __func__);

        P_t *p = (P_t*) q->ptr;
        sprintf (s1, "%5d", *p->param);
        disp (p->text, s1);
    }

    else  {
        if (DBG_MENU & debug)  printf ("%s: display default\n", __func__);

        if (q->pParam)  {
            sprintf (s1, "%5d", *q->pParam);
            disp (q->text, s1);
        }
        else
            disp (q->text, q->text2);
    }
}

