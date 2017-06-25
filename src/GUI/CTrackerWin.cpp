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


#include "CTrackerWin.h"

#include "TrackerMessages.h"
#include "CBackView.h"

//********************************
CTrackerWin::CTrackerWin(void)
						:BWindow(BRect(100,100,100+450,100+230), "R'alf Tracker 0.3.1a", B_TITLED_WINDOW, B_NOT_RESIZABLE|B_NOT_ZOOMABLE)
//********************************
{
	BButton *button;
	ULONG x1=5,	x2=145,		x3=170, x4=445,
												x5=170, x6=250,		x7=255, x8=335,		x9=340, x10=390,		x11=395, x12=445;
	ULONG y1=5, y2=225,		y3=180, y4=200,		y5=205, y6=225;

	// background
	CBackView *back = new CBackView(BRect(x1-5,y1-5,x4+5,y2+5), "", B_FOLLOW_ALL, B_WILL_DRAW);
	if (back) AddChild(back);
	else return;	// things are getting harder...

	// file scroll list
	mFileList = new CFileList(BRect(x1,y1, x2,y2), "", B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM);
	mFileList->SetInvocationMessage(new BMessage(K_MSG_PLAY));
	mFileScroll = new BScrollView("fileScroll", mFileList, B_FOLLOW_LEFT|B_FOLLOW_TOP_BOTTOM, 0, FALSE, TRUE);
	back->AddChild(mFileScroll);

	// stack of buttons
	button = new BButton(BRect(x5, y3, x6, y4), "", "Load list", new BMessage(K_MSG_LOAD), B_FOLLOW_BOTTOM);
	back->AddChild(button);
	button = new BButton(BRect(x7, y3, x8, y4), "", "Alphabetize", new BMessage(K_MSG_ALFA), B_FOLLOW_BOTTOM);
	back->AddChild(button);
	button = new BButton(BRect(x9, y3, x10, y4), "", "Play", new BMessage(K_MSG_PLAY), B_FOLLOW_BOTTOM);
	back->AddChild(button);
	button = new BButton(BRect(x11,y3, x12, y4), "", "Stop", new BMessage(K_MSG_STOP), B_FOLLOW_BOTTOM);
	back->AddChild(button);

	button = new BButton(BRect(x5, y5, x6, y6), "", "Save list", new BMessage(K_MSG_SAVE), B_FOLLOW_BOTTOM);
	back->AddChild(button);
	button = new BButton(BRect(x7, y5, x8, y6), "", "Randomize", new BMessage(K_MSG_RAND), B_FOLLOW_BOTTOM);
	back->AddChild(button);
	button = new BButton(BRect(x9, y5, x10, y6), "", "Next", new BMessage(K_MSG_NEXT), B_FOLLOW_BOTTOM);
	back->AddChild(button);
	button = new BButton(BRect(x11,y5, x12, y6), "", "Quit", new BMessage(K_MSG_QUIT), B_FOLLOW_BOTTOM);
	back->AddChild(button);

	// strings
	mDisplayName = new BStringView(BRect(x3, 5, x4, 5+13), "", "", B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	mFileName = new BStringView(BRect(x3, 25, x4, 25+13), "", "", B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	back->AddChild(mDisplayName);
	back->AddChild(mFileName);

	// elctrogramme
	mElectroLeft = new CElectrogramme(BRect(x3,45,x4,45+60), "", FALSE, B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	mElectroRight = new CElectrogramme(BRect(x3,110,x4,110+60), "", TRUE, B_FOLLOW_LEFT_RIGHT|B_FOLLOW_TOP);
	back->AddChild(mElectroLeft);
	back->AddChild(mElectroRight);
	
	SetPulseRate(60);
}


//********************************
void CTrackerWin::MessageReceived(BMessage *msg)
//********************************
{
	switch(msg->what)
	{
		case K_MSG_PLAY:
		case K_MSG_STOP:
		case K_MSG_NEXT:
		case K_MSG_QUIT:
		case K_MSG_LOAD:
		case K_MSG_SAVE:
		case K_MSG_RAND:
		case K_MSG_ALFA:
			msg = DetachCurrentMessage();
			be_app->PostMessage(msg);
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}

//********************************
bool CTrackerWin::QuitRequested(void)
//********************************
{
	be_app->PostMessage(B_QUIT_REQUESTED);
	return TRUE;
}


//********************************
void CTrackerWin::setPlayNames(char *display, char *file)
//********************************
{
	Lock();
	mDisplayName->SetText(display);
	mFileName->SetText(file);
	Unlock();
}

