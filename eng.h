#ifndef ENG_
# define ENG_

#define WHYTE_LEN   10
#define LOCO_LEN    20

#define N_ENG       20

struct Eng_s {
    float  drvrDia;
    float  cylDia;
    float  cylStr;
    int    numCyl;

    int    psi;
    int    wtAdhT;
    int    wtLocoT;
    int    wtTndrT;

    float  grate;       // unused
    float  blrLen;      // unused
    float  blrDia;      // unused
    float  blrVol;      // unused

    float  pipeDia;
        
    char   whyte [WHYTE_LEN];
    char   name  [LOCO_LEN];
};

extern Eng_s   engs [];
extern Eng_s  *pEng;

// -----------------------------------------------
void    engDisp (Eng_s *p);
char    engEdit (Eng_s *p, const char *desc);
int     engGet  (char* name);
int     engInit (const char *locoName);

#endif
