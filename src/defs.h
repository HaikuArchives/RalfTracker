/* defs.h */

/* $Id: defs.h,v 3.4 1993/11/17 15:31:16 espie Exp espie $ 
 * $Log: defs.h,v $
 * Revision 3.4  1993/11/17  15:31:16  espie
 * *** empty log message ***
 *
 * Revision 3.3  1993/11/11  20:00:03  espie
 * Amiga support.
 *
 * Revision 3.1  1992/11/19  20:44:47  espie
 * Protracker commands.
 *
 * Revision 3.0  1992/11/18  16:08:05  espie
 * New release.
 */

#define LOCAL static
/* X is too short */
#define XT extern

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

/* needed for the amiga include files */
typedef short BOOL;

#define MIN(A,B) ((A)<(B) ? (A) : (B))
#define MAX(A,B) ((A)>(B) ? (A) : (B))
     
#define D fprintf(stderr, "%d\n", __LINE__);

#undef FORKING