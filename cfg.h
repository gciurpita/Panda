#ifndef CFG_H
# define CFG_H

# include <Arduino.h>

extern const char *cfgFname;

// -------------------------------------
void cfgClr  (void);
void cfgDispAll (Stream &);
void cfgEdit (Stream &);
int  cfgLoad (const char *filename);
int  cfgDump (const char *filename);
void cfgSave (const char *filename);

#endif
