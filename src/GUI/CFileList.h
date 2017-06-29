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


#ifndef CFILE_LIST_H
#define CFILE_LIST_H

#include <ListView.h>

#include "RalfTypes.h"

//*************************************
class CFileList : public BListView
//*************************************
#pragma mark CFileList
{
public:
	CFileList(BRect frame, char *name,
						ULONG resizeMode = B_FOLLOW_LEFT | B_FOLLOW_TOP,
						ULONG flags = B_WILL_DRAW | B_FRAME_EVENTS);
	~CFileList(void);
	bool MessageDropped(BMessage *msg, BPoint point, BPoint offset);
virtual void MouseMoved(BPoint pt, ulong transit, BMessage * msg);

	void HiliteDragRegion(bool hilite);
};

#endif
