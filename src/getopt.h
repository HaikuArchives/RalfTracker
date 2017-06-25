/* getopt.h */
/* $Id: getopt.h,v 1.3 1993/11/17 15:31:16 espie Exp espie $
 * $Log: getopt.h,v $
 * Revision 1.3  1993/11/17  15:31:16  espie
 * *** empty log message ***
 *
 */

struct long_option
	{
	char *fulltext;
	int argn;
	char abbrev;
	};

extern int optind;
extern char *optarg;
