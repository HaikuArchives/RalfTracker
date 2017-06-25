/**********************************************************

	Project: ralf_tracker
	-------- ------------

	Version : 0.3.1a (alpha)
	6 july 1996 for BeOS DR 7.1 (and DR 7)

	Sample wrapper around the Marc Espie's "tracker 3.19".
	Please feel free to reuse that code, as either a sample code
	for programing onto the BeBox or to add a more robust
	interface with nice features.
	
	Once I get another version of "tracker" from Marc Espie which
	uses something else that K&R formating, I will suppress the tracker_lib.so.
	This library is here because I can't mix C K&R code and C++ code in the
	same project with CodeWarior.

	There is no copyright on this code. You can modify it, destroy it,
	reuse it or simply trash it. If you reuse it, please mention my
	name somewhere. If you did something beautifull, please send me
	a copy. I'd like to see "be-demos" like there was "amiga-demos".

	Raphael MOLL,
	moll@linux.utc.fr

	Disclaimer : there is no link between this code and my work
	at MIPSYS, France.
	
	Bug fixes by Hubert Figuiere (june 1996)
	figuiere@kagi.com

***********************************************************/

#ifndef TRACKER_MESSAGES_H
#define TRACKER_MESSAGES_H



#define K_MSG_PLAY	'Play'
#define K_MSG_STOP	'Stop'
#define K_MSG_NEXT	'Next'
#define K_MSG_QUIT	'Quit'
#define K_MSG_LOAD	'Load'
#define K_MSG_SAVE	'Save'
#define K_MSG_RAND	'Rand'
#define K_MSG_ALFA	'Alfa'


//#define K_MSG_ABOUT	'Abot'



#endif
