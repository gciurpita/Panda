// ---------------------------------------------------------
#include "FS.h"

#include "file.h"
#include "koala.h"
#include "vars.h"
#include "utils.h"

File  file;

// ---------------------------------------------------------
// attempt to list all files
void
fileDir (void)
{
    printf ("%s:\n", __func__);

    File root = SPIFFS.open ("/");
    File f;

    while ((f = root.openNextFile ()))  {
        printf ("  %6d  %s\n", f.size(), f.name ());
    }
}

// ---------------------------------------------------------
// delete selected file
void
fileDelete (void)
{
    printf ("%s: enter 'D' to delete file\n", __func__);

    File root = SPIFFS.open ("/");
    File f;

    while ((f = root.openNextFile ()))  {
        // display filename
        printf ("  %s:\n", f.name ());

        // wait for keypress
        readLine (s, S_SIZE);
        if ('\n' == s [0])  {
            continue;
        }

        // wait for confirmation
        printf ("   confirm deletion of %s: (D)\n", f.name ());
        readLine (s, S_SIZE);
        if ('D' == s [0])  {
            printf ("    deleting %s\n", f.name());
            SPIFFS.remove (f.name ());
        }
    }

    printf (" %s: done\n", __func__);
}
