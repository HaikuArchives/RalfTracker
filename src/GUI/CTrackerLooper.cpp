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

#include <string.h>


#include "CTrackerLooper.h"
#include "CTrackerApp.h"
#include "TrackerLib.h"
#include "TrackerMessages.h"

#include "DebugUtils.h"

//*******************************************
CTrackerLooper::CTrackerLooper(long priority)
							 :BLooper("player", priority)
//*******************************************
{
	dprintf("Tracker Task launched\n");

	gBeAudioBufSize = 4096;
	gBeAudioPlayBuffer = CTrackerApp::trackerPlayBuffer;
	gBeAudioExit = CTrackerApp::trackerFastExit;
	mIsPlaying = FALSE;
	setup_play_track(FALSE, TRUE);

	Run();	// launch the thread and run separately...
}

//***********************************
CTrackerLooper::~CTrackerLooper(void)
//***********************************
{
	gBeAudioNextModule = TRUE;
	gBeAudioQuitModule = TRUE;
	mIsPlaying = FALSE;
	close_play_track();
}

//**************************************
BOOL CTrackerLooper::QuitRequested(void)
//**************************************
{
	gBeAudioNextModule = TRUE;
	gBeAudioQuitModule = TRUE;
	mIsPlaying = FALSE;
	return TRUE;
}

//*************************************************
void CTrackerLooper::MessageReceived(BMessage *msg)
//*************************************************
{
	if (msg && !mIsPlaying)
	{
		CTrackerApp *app = (CTrackerApp *)be_app;
		mIsPlaying = TRUE;
		gBeAudioNextModule = FALSE;
		gBeAudioQuitModule = FALSE;

		char displayname[256] = "Playing ";
		strcat(displayname, (char *)msg->FindString("displayname"));
		char *filename = (char *)msg->FindString("filename");
		app->mWindow->setPlayNames(displayname, filename);

		play_track(filename);

		app->mWindow->setPlayNames("Stopped", "");

		mIsPlaying = FALSE;
		if (!gBeAudioQuitModule) app->PostMessage(K_MSG_NEXT);
	}
}


//------------------------------------------------------
