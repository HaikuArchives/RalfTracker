/**********************************************************

	Project: ralf_tracker
	-------- ------------

	Version : 0.3a (alpha)
	9 April 96 for BeOS 1.1d7

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

#ifndef CTRACKER_APP_H
#define CTRACKER_APP_H

#include "RalfTypes.h"
#include "CTrackerLooper.h"
#include "CTrackerWin.h"

//*************************************
class CTrackerApp : public BApplication
//*************************************
#pragma mark CTrackerApp
{
public:

	CTrackerApp(ulong sign);
	~CTrackerApp(void);
	//thread_id Run(void);
	void ReadyToRun(void);
	void MessageReceived(BMessage *msg);
	bool QuitRequested(void);
	void RefsReceived(BMessage *msg);
	void AboutRequested(void);				//no longer needs K_MSG_ABOUT [HF]

	void createAppMenu(void);
	void uglyAboutBox(void);

	CTrackerLooper *mTrackerLooper;
	CTrackerWin *mWindow;
	BAudioSubscriber *mPlayer;
	BLocker mBufferLock;
	BList mBufferList;	// a list of SBufferItem ptrs to be played
	BList mFileList;		// a list of CFileItem ptrs to be selected
	BPopUpMenu *mMainMenu;

	ULONG mInstantRightVolume, mInstantLeftVolume;

	void addFileToList(record_ref ref);
	void addDirectoryToList(record_ref ref);

	void playList(void);
	void playNext(void);
	void stopPlay(void);

	static long trackerTask(void *data);
	static void trackerPlayBuffer(UBYTE *buffer, long length);
	static void trackerFastExit(void);
	static bool trackerStreamPlay(void *user, char *buffer, long count);
};

#endif