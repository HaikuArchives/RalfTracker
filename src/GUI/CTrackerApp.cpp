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
	
	Bug fixes by Hubert Figuiere (june-july 1996)
	figuiere@kagi.com

***********************************************************/

#include <string.h>

#include "CTrackerApp.h"

#include "TrackerMessages.h"
#include "TrackerLib.h"
#include "DebugUtils.h"
#include "CFileItem.h"

//********************************
CTrackerApp::CTrackerApp(ulong sign) : BApplication(sign)
//********************************
{
	//mTrackerLooper = NULL;
	//mWindow = NULL;
	//mPlayer = NULL;
	mInstantRightVolume = NULL;
	mInstantLeftVolume = NULL;
	
	//*******>>> This part was previously in ReadyToRun() [HF]
	// create the playing thread
	mTrackerLooper = new CTrackerLooper(B_REAL_TIME_PRIORITY);

	// create the window
	mWindow = new CTrackerWin;
	if (mWindow) mWindow->Show();

	createAppMenu();

	// create the audio subscriber
	mPlayer = new BAudioSubscriber("TrackerPlayer");
	if (mPlayer)
	{
		long error;
		error = mPlayer->Subscribe(B_DAC_STREAM,B_INVISIBLE_SUBSCRIBER_ID,TRUE);

		error = mPlayer->SetDACSampleInfo(2, 2, B_BIG_ENDIAN, B_LINEAR_SAMPLES);
		error = mPlayer->SetSamplingRate(44100);

		long bufSize, bufCount, subCount;
		BOOL isRunning;
		subscriber_id clique;
		error = mPlayer->GetStreamParameters(&bufSize, &bufCount, &isRunning, &subCount, &clique);
		
		error = mPlayer->EnterStream(NULL, FALSE, NULL,trackerStreamPlay, NULL, TRUE);
	}
	//<<<******
}


//********************************
CTrackerApp::~CTrackerApp(void)
//********************************
{
long i,n;

	dprintf("deleting audio subscriber\n");
	if (mPlayer) delete mPlayer;
	mPlayer = NULL;

	dprintf("delete tracker task\n");
	if (mTrackerLooper) mTrackerLooper->Quit();

	dprintf("Freing buffers...");
	mBufferLock.Lock();
	n = mBufferList.CountItems();
	dprintf("Number of buffers in the list : %d\n", n);
	for(i = 0; i < n; i++)
	{
		SBufferItem *item = (SBufferItem *)mBufferList.ItemAt(i);
		if (!item) continue;
		if (item->data) delete item->data;
		delete item;
	}
	mBufferLock.Unlock();
	dprintf("End of destructor of CTrackerApp\n");
}


//**********************************************
void CTrackerApp::MessageReceived(BMessage *msg)
//**********************************************
{
	switch(msg->what)
	{
		case K_MSG_PLAY:
			playList();
			break;
		case K_MSG_NEXT:
			playNext();
		case K_MSG_STOP:
			stopPlay();
			break;
		case K_MSG_LOAD:
		case K_MSG_SAVE:
		case K_MSG_RAND:
		case K_MSG_ALFA:
			break;
		case K_MSG_QUIT:
			gBeAudioNextModule = TRUE;
			gBeAudioQuitModule = TRUE;
			PostMessage(B_QUIT_REQUESTED);
			break;
		default:
			// dprintf...
			break;
	}
}


//*******************************************
void CTrackerApp::RefsReceived(BMessage *msg)
//*******************************************
{
BOOL succeed = FALSE;
ULONG type, i;
LONG count;
record_ref item;

	if (!msg) return;
	msg->GetInfo("refs", &type, &count);
	for(i = 0; i < count; i++)
	{
		item = msg->FindRef("refs", i);
		if (item.database >= 0 && item.record >= 0)
			if (does_ref_conform(item, "File")) {
				addFileToList(item);
			}/*if*/
			else {
				if (does_ref_conform(item, "Folder"))
					addDirectoryToList(item);
			}/*else*/
	}
}


//********************************
bool CTrackerApp::QuitRequested(void)
//********************************
{
	if (mTrackerLooper) return mTrackerLooper->QuitRequested();
	return TRUE;
}

//********************************
void CTrackerApp::AboutRequested(void)				//no longer needs K_MSG_ABOUT [HF]
//********************************
{
	uglyAboutBox();
}


#ifdef NO_RUN
//********************************
thread_id CTrackerApp::Run(void)
//********************************
{
	thread_id retVal = BApplication::Run();
	return retVal;
}
#endif

//********************************
void CTrackerApp::ReadyToRun(void)
//********************************
{

}


//***********************************
void CTrackerApp::createAppMenu(void)
//***********************************
{
BMenuItem *item;

	mMainMenu = new BPopUpMenu("", FALSE, FALSE);
	if (!mMainMenu) return;

	item = new BMenuItem("About...", new BMessage(B_ABOUT_REQUESTED));							mMainMenu->AddItem(item);
	mMainMenu->AddSeparatorItem();
	item = new BMenuItem("Load list...", new BMessage(K_MSG_LOAD), 'O');	mMainMenu->AddItem(item);
	item = new BMenuItem("Save list...", new BMessage(K_MSG_SAVE), 'S');	mMainMenu->AddItem(item);
	item = new BMenuItem("Alphabetize", new BMessage(K_MSG_ALFA),  'A');	mMainMenu->AddItem(item);
	item = new BMenuItem("Randomize", new BMessage(K_MSG_RAND),  'R');	mMainMenu->AddItem(item);
	mMainMenu->AddSeparatorItem();
	item = new BMenuItem("Play", new BMessage(K_MSG_PLAY), 'P');					mMainMenu->AddItem(item);
	item = new BMenuItem("Next", new BMessage(K_MSG_NEXT), 'N');					mMainMenu->AddItem(item);
	item = new BMenuItem("Stop", new BMessage(K_MSG_STOP), 'S');					mMainMenu->AddItem(item);
	mMainMenu->AddSeparatorItem();
	item = new BMenuItem("Quit", new BMessage(K_MSG_QUIT), 'Q');					mMainMenu->AddItem(item);

	SetMainMenu(mMainMenu);
}


//***********************************
void CTrackerApp::uglyAboutBox(void)
//***********************************
{
	BAlert *about;

	about = new BAlert(	"R'alf Tracker",
											"R'alf Tracker 0.3.1alpha\n\n"
											"A simple interface wrapped around Marc Espie's tracker player 3.19.\n\n"
											"Freeware by Raphael MOLL (moll@linux.utc.fr)\n"
											"and by Hubert FIGUIERE (figuiere@kagi.com)",
											"OK");
	if (about) about->Go();

} // end of uglyAboutBox


//------------------------------------------------------
#pragma mark -


//********************************
void CTrackerApp::addFileToList(record_ref ref)
//********************************
{
	if (does_ref_conform(ref, "Folder"))
	{
		addDirectoryToList(ref);
		return;
	}

	if (!mWindow || !mWindow->mFileList) return;

	// and why does CodeWarior 8 refuses to call _register_auto_objects here ?
	// this is the brute force answer !
	BFile *file = new BFile;
	BDirectory *dir = new BDirectory;
	BDirectory *dir2 = new BDirectory;

	if (!file || !dir || !dir2) return;

	// create the file instance...
	file->SetRef(ref);

	// create a new file item for the list
	CFileItem *item = new CFileItem();
	if (!item) return;	// critical. Should not occur.
	
	// extract the name to be displayed
	item->mDisplayName = new char[256];
	file->GetName(item->mDisplayName);

	// check if that name already exists in the list...
	ULONG i,n;
	n = mFileList.CountItems();
	for(i=0;i<n;i++)
	{
		CFileItem *item2 = (CFileItem *)mFileList.ItemAt(i);
		if (item2
				&& item2->mDisplayName
				&& strcmp(item2->mDisplayName, item->mDisplayName) == 0)
		{
			// then remove the old item
			mFileList.RemoveItem(item2);
			mWindow->mFileList->RemoveItem(item2->mDisplayName);
			delete item2;
		}
	}

	// extract the directory path
	LONG error;
	char s1[512]="";
	char s2[512]="";
	file->GetName(s2);
	error = file->GetParent(dir);
	while(error == B_NO_ERROR)
	{
		dir->GetName(s1);
		if (strlen(s2))
		{
			strcat(s1, "/");
			strcat(s1, s2);
		}
		strcpy(s2, s1);
		error = dir->GetParent(dir2);
		if (error == B_NO_ERROR) dir->SetRef(dir2->Record()->Ref());
	}
	strcpy(s1, "/");
	strcat(s1, s2);
	strcpy(s2, s1);

	item->mFileName = strdup(s2);
	mFileList.AddItem(item);	// add the item to the list of the app
	
	if (mWindow->Lock()) {		// window should be locked.. [HF]
		mWindow->mFileList->AddItem(item->mDisplayName);	// add the name to be displayed
		mWindow->mFileList->Select(mWindow->mFileList->CountItems()-1);
		mWindow->Unlock();
	}
	delete file;
	delete dir;
	delete dir2;
}


//********************************
void CTrackerApp::addDirectoryToList(record_ref ref)
//********************************
{
	if (does_ref_conform(ref, "File"))
	{
		addFileToList(ref);
		return;
	}

	// and why does CodeWarior 8 refuses to call _register_auto_objects here ?
	BFile *file = new BFile;
	BDirectory *dir = new BDirectory;
	BDirectory *dir2 = new BDirectory;
	long i,n;
	long error;

	if (!file || !dir || !dir2) return;

	dir->SetRef(ref);

	// first load every file
	n = dir->CountFiles();
	for(i=0; i<n; i++)
	{
		error = dir->GetFile(i, file);
		if (error == B_NO_ERROR) addFileToList(file->Record()->Ref());
	}

	// then load every sub directory
	n = dir->CountDirectories();
	for(i=0; i<n; i++)
	{
		error = dir->GetDirectory(i, dir2);
		if (error == B_NO_ERROR) addDirectoryToList(dir2->Record()->Ref());
	}

	delete file;
	delete dir;
	delete dir2;
}


//------------------------------------------------------
#pragma mark -

//********************************
void CTrackerApp::playList(void)
//********************************
{
ULONG index = mWindow->mFileList->CurrentSelection();
CFileItem *item;

	stopPlay();

	item = (CFileItem *)mFileList.ItemAt(index);
	if (item && item->mFileName && item->mDisplayName)
	{
		BMessage *msg = new BMessage(K_MSG_PLAY);
		msg->AddString("filename", item->mFileName);
		msg->AddString("displayname", item->mDisplayName);
		mTrackerLooper->PostMessage(msg);
	}
}


//********************************
void CTrackerApp::playNext(void)
//********************************
{
ULONG index;

	index = mWindow->mFileList->CurrentSelection()+1;
	if (index >= mWindow->mFileList->CountItems()) index = 0;
	mWindow->mFileList->Select(index);
	playList();
}


//********************************
void CTrackerApp::stopPlay(void)
//********************************
{
	if (mTrackerLooper->mIsPlaying)
	{
		gBeAudioNextModule = TRUE;
		gBeAudioQuitModule = TRUE;
	}
}


//------------------------------------------------------
#pragma mark -

//********************************
void CTrackerApp::trackerFastExit(void)
//********************************
{
	dprintf("Fast exit from Tracker Task requested\n");
	be_app->PostMessage(B_QUIT_REQUESTED);
}



//********************************
void CTrackerApp::trackerPlayBuffer(UBYTE *buffer, long length)
//********************************
{
CTrackerApp *app = (CTrackerApp *)be_app;
SBufferItem *item;
long n;

	//dprintf("play buffer : length %8d, ptr %p\n", length, buffer);

	item = new SBufferItem;
	if (!item) return; // can't alloc item
	item->data = new UBYTE[length];
	item->length = length;
	memcpy(item->data, buffer, length);

	app->mBufferLock.Lock();
	app->mBufferList.AddItem(item);
	n = app->mBufferList.CountItems();
	app->mBufferLock.Unlock();

	// each buffer of 4096 bytes takes approximately 23 ms to play.
	// if we have more than 50 buffer, give enougth time to suppress 30 buffers
	// from the list.
	if (n > 50) snooze(23000*30);
}


//********************************
bool CTrackerApp::trackerStreamPlay(void *user, char *buffer, long count)
//********************************
{
CTrackerApp *app = (CTrackerApp *)be_app;
SBufferItem *item;
long n;

	app->mBufferLock.Lock();
	item = (SBufferItem *)app->mBufferList.RemoveItem(0L);
	n = app->mBufferList.CountItems();
	app->mBufferLock.Unlock();
	if (item)
	{
		if (item->data)
		{
			//dprintf("buffer %08p, count %6d, data %08p, length %6d, items %d\n", buffer, count,item->data, item->length,n);
			memcpy(buffer, item->data, min(count, item->length));
			delete item->data;
		}
		delete item;
	}

	return TRUE;
}
