#ifndef MENU_H
# define MENU_H

# define Clear   "\e[2J\e[0;0H"
# define Normal  "\e[0m"
# define Bold    "\e[1m"
# define Blink   "\e[5m"
# define Red     "\e[31m"
# define Yellow  "\e[33m"
# define Blue    "\e[34m"

#define MAX_DIGIT   5

// button and display function pointers
typedef struct {
    void (*menu) (void *);
    void (*sel)  (void *);
    void (*up)   (void *);
    void (*dn)   (void *);
    void (*disp) (void *);
} Func_t;

// parameter descripton
typedef struct {
    const char *text;
    int        *param;
    int         min;
    int         max;
    void       *p;
    Func_t  func;
} P_t;

// parameter types
typedef enum {
    T_NULL,
    T_NONE,
    T_MENU,
    T_PARAM,
    T_STR,
    T_LIST,
} T_t;

inline const char *
menuType (T_t t)
{
    switch (t)  {
    case T_NULL:
        return "T_NULL";
        break;

    case T_NONE:
        return "T_NONE";
        break;

    case T_MENU:
        return "T_MENU";
        break;

    case T_PARAM:
        return "T_PARAM";
        break;

    case T_STR:
        return "T_STR";
        break;

    case T_LIST:
        return "T_LIST";
        break;

    default:
        return "unknown";
        break;
    }
}

// menu description
typedef struct {
    const char *text;
    const char *text2;
    T_t         type;
    void       *ptr;
    int        *pParam;
} Menu_t;

// menu stimuli
typedef enum  {
    M_NULL,
    M_MENU,
    M_SEL,
    M_UP,
    M_DN,
    M_LAST
} MenuStim_t;

// -------------------------------------------------------------------
// menu functions

void __   (void *);     // null function
void sel  (void *);     // select list item

void sft  (void *);     // shift digit to edit next digit to the right
void sfA  (void *);     // shift digit to right, not past NULL

void deA  (void *);     // decrement value of a character (' ' to '~')
void dec  (void *);     // decrement a digit value

void inA  (void *);     // increment value of a character (' ' to '~')
void inc  (void *);     // increment a digit value

void up   (void *);     // increment value of parameter
void dn   (void *);     // decrement value of paramter

void dspA (void *);     // display string for edit
void dspP (void *);     // display parameter for edit
void dspV (void *);     // display variable for edit


void menu (MenuStim_t);

#endif
