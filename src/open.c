/* open.c */

/* Magic open file: path lookup and transparent decompression */

/* $Id: open.c,v 3.9 1993/11/17 15:31:16 espie Exp espie $ 
 * $Log: open.c,v $
 * Revision 3.9  1993/11/17  15:31:16  espie
 * *** empty log message ***
 *
 * Revision 3.8  1993/11/11  20:00:03  espie
 * Amiga support.
 *
 * Revision 3.7  1993/08/17  16:53:09  espie
 * New gzip suffix.
 *
 * Revision 3.3  1993/07/14  16:33:41  espie
 * Added gzip/shorten.
 *
 * Revision 3.2  1992/12/03  15:00:50  espie
 * restore stty.
 *
 * Revision 3.1  1992/11/19  20:44:47  espie
 * Protracker commands.
 *
 * Revision 3.0  1992/11/18  16:08:05  espie
 * New release.
 *
 * Revision 1.5  1992/11/01  13:10:06  espie
 * Cleaned up path handler, and some more bugs.
 * Check for size now.
 * Added path support. Transparent interface. We look up through the file
 * list, which is small anyway.
 */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef AMIGA
#include <stdlib.h>
#else
//#include <malloc.h>
#endif

#include "defs.h"
#include "extern.h"

LOCAL char *id = "$Id: open.c,v 3.9 1993/11/17 15:31:16 espie Exp espie $";

#define MAX_DESC 50 /* Maximum number of opened files */

LOCAL struct exfile 
    {
    FILE *handle;
    int type;
    } desc[MAX_DESC];

LOCAL int ixfile = 0;

#define REALFILE 1
#define PIPEFILE 2

/* compression methods we do know about.
 * Important restriction: for the time being, the output
 * must be a single module.
 */

LOCAL struct compression_method
    {
    char *extension;
    char *command;
    } comp_table[] =
    {
	".gz",	"gzip -dc %s",
#ifdef GZIP
	".z",	"gzip -dc %s",
#else
    ".Z",   "zcat %s",
#endif
	".s",	"shorten -x %s -",
	".shn",	"shorten -x %s -",
    ".zoo", "zoo xpq %s",
#ifdef AMIGA
    ".lzh", "lha -q p %s",
    ".lha", "lha -q p %s",
#else
    ".lzh", "lha pq %s",
    ".lha", "lha pq %s",
#endif
    ".zip", "unzip -pqq %s",
    ".arc", "arc pn %s",
    NULL,   NULL
    };

/***
 *
 *  Handling extensions.
 *
 ***/

LOCAL BOOL check_ext(s, ext)
char *s, *ext;
    {
    int ext_len, s_len;
    char *c;

    ext_len = strlen(ext);
    s_len = strlen(s);
    if (s_len < ext_len)
        return FALSE;
    for (c = s + s_len - ext_len; *c; c++, ext++)
        if (tolower(*c) != tolower(*ext))
            return FALSE;
    return TRUE;
    }

LOCAL BOOL exist_file(fname)
char *fname;
    {
    FILE *temp;

    temp = fopen(fname, "r");
    if (temp)
        {
        fclose(temp);
        return TRUE;
        }
    else
        return FALSE;
    }

#ifndef MAXPATHLEN
#define MAXPATHLEN 350
#endif

LOCAL char *find_file(fname, path)
char *fname;
char *path;
    {
    char *sep;
    static char buffer[MAXPATHLEN];
    int len;

        /* first, check the current directory */
    if (exist_file(fname))
        return fname;
    while(path)
        {
        sep = strchr(path, ':');
        if (sep)
            len = sep - path;
        else
            len = strlen(path);
        if (len < MAXPATHLEN)
            {
            strncpy(buffer, path, len);
            buffer[len] = '/';
            if (len + strlen(fname) < MAXPATHLEN - 5)
                {
                strcpy(buffer + len + 1, fname);
                puts(buffer);
                if (exist_file(buffer))
                    return buffer;
                }
            }
        if (sep)
            path = sep + 1;
        else
            return NULL;
        }
    return NULL;
    }

FILE *open_file(fname, mode, path)
char *fname;
char *mode; /* right now, only mode "r" is supported */
char *path; 
    {
    struct exfile *new;
    struct compression_method *comp;

    if (mode[0] != 'r' || mode[1] != 0)
        return NULL;
    
    if (ixfile == MAX_DESC)
        return NULL;

    new = desc + ixfile++;

    fname = find_file(fname, path);
    if (!fname)
        return NULL;
#ifdef __SUPPORT_PIPE__
    for (comp = comp_table; comp->extension; comp++)
        if (check_ext(fname, comp->extension))
            {
            char pipe[MAXPATHLEN + 25];

            sprintf(pipe, comp->command, fname);
            new->type = PIPEFILE;
            if (new->handle = popen(pipe, "r"))
                return new->handle;
            else
                {
                ixfile--;
                return NULL;
                }
            }
#endif // of __SUPPORT_PIPE__
    new->type = REALFILE;
    if ((new->handle = fopen(fname, "r")))
        return new->handle;
    else
        {
        ixfile--;
        return NULL;
        }
    }


void close_file(file)
FILE *file;
    {
    if (file)
        {
        int i;

        for (i = 0; i < ixfile; i++)
            if (desc[i].handle == file)
                {
                switch(desc[i].type)
                    {
                case REALFILE:
                    fclose(file);
                    break;
#ifdef __SUPPORT_PIPE__
                case PIPEFILE:
                    pclose(file);
                    break;
#endif // of __SUPPORT_PIPE__
                    }
                ixfile--;
                desc[i].handle = desc[ixfile].handle;
                desc[i].type = desc[ixfile].type;
                }
            }
    }

