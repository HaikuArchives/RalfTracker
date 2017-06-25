/* termio.c */
/* special termio discipline for sun/sgi,
 * for non blocking io and such.
 * These functions should not be too difficult
 * to write for a PC.
 */

/* $Id: termio.c,v 3.14 1993/11/17 15:31:16 espie Exp espie $
 * $Log: termio.c,v $
 * Revision 3.14  1993/11/17  15:31:16  espie
 * *** empty log message ***
 *
 * Revision 3.13  1993/10/06  17:17:45  espie
 * Stupid termio bug: shouldn't restore term to sanity if we don't
 * know what sanity is. For instance, if we haven't modified anything.
 *
 * Revision 3.11  1993/07/17  12:00:30  espie
 * Added other commands (numerous).
 *
 * Revision 3.10  1993/07/14  16:33:41  espie
 * Added partial code for hpux.
 *
 * Revision 3.9  1993/04/28  20:14:41  espie
 * My error...
 *
 * Revision 3.8  1993/04/25  14:50:17  espie
 * cflags interpreted correctly.
 *
 * Revision 3.7  1993/01/16  16:23:33  espie
 * Hsavolai fix.
 *
 * Revision 3.6  1993/01/15  14:00:28  espie
 * Added bg/fg test.
 *
 * Revision 3.5  1993/01/06  17:58:39  espie
 * Added changes for linux.
 *
 * Revision 3.4  1992/12/03  15:00:50  espie
 * restore stty.
 *
 * Revision 3.3  1992/11/27  10:29:00  espie
 * General cleanup
 *
 * Revision 3.2  1992/11/22  17:20:01  espie
 * Added update_frequency call, mostly unchecked
 *
 * Revision 3.1  1992/11/19  20:44:47  espie
 * Protracker commands.
 *
 */

#ifdef dec
#define stub_only
#endif

#if __MWERKS__	// BE
#define stub_only
#endif

#ifdef linux
#include <termios.h>
#else
#ifdef hpux
#include <sys/bsdtty.h>
#endif
//#include <sys/termio.h>
#endif
#include <stdio.h>
#include <signal.h>
#include "defs.h"

/* do not define any stdio routines if it's known not to work */

#ifdef stub_only

BOOL run_in_fg()
	{
	return TRUE;
	}

void nonblocking_io()
	{
	}

void sane_tty()
	{
	}

int may_getchar()
	{
	return EOF;
	}

#else

LOCAL struct termio sanity;
LOCAL struct termio *psanity = 0;

LOCAL BOOL is_fg;

/* signal handler */

LOCAL void goodbye(sig)
int sig;
    {
    fprintf(stderr, "\nSignal %d", sig);
    discard_buffer();
    end_all();
    }

LOCAL void abort_this(sig)
int sig;
	{
	discard_buffer();
	exit(0);
	}

BOOL run_in_fg()
	{
	int val;
	/* real check for running in foreground */
	if (ioctl(fileno(stdin), TIOCGPGRP, &val))
		return FALSE; 
	if (val == getpgrp())
		return TRUE;
	else
		return FALSE;
	}

LOCAL void switch_mode()
	{
	struct termio zap;

	signal(SIGCONT, switch_mode);
	signal(SIGINT, goodbye);
	signal(SIGQUIT, goodbye);
	signal(SIGUSR1, abort_this);

	if (run_in_fg())
		{
		ioctl(fileno(stdin), TCGETA, &zap);
#ifdef linux
		zap.c_cc[VMIN] = 0;
		zap.c_cc[VTIME] = 0;
/* Commented out
As Hannu said:
The current Linux kernel interprets correctly the c_lflags field so it
should be set like for the other systems. 
		zap.c_lflag = 0;
 */
#else
		zap.c_cc[VEOL] = 0;
		zap.c_cc[VEOF] = 0;
#endif
		zap.c_lflag &= ~(ICANON | ECHO);
		ioctl(fileno(stdin), TCSETA, &zap);
		is_fg = TRUE;
		}
	else
		is_fg = FALSE;
	}

void nonblocking_io()
	{
	/* try to renice our own process to get more cpu time */
	if (nice(-15) == -1)
		nice(0);
	if (!psanity)
		{
		psanity = &sanity;
		ioctl(fileno(stdin), TCGETA, psanity);
		}
	switch_mode();
	}


void sane_tty()
	{
	if (psanity)
		ioctl(fileno(stdin), TCSETA, psanity);
	}

int may_getchar()
	{
	char buffer;

	if (run_in_fg() && !is_fg)
		switch_mode();
	if (run_in_fg() && read(fileno(stdin), &buffer, 1))
		return buffer;
	return EOF;
	}

#endif
