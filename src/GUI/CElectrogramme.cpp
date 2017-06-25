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

#include "CElectrogramme.h"
#include "RalfTypes.h"
#include "CTrackerApp.h"



//#define K_BACKGROUND_GRAY 144,144,144
#define K_ELECTRO_GRAY    88,88,88
#define K_ELECTRO_GREEN		0,153,0



//********************************
CElectrogramme::CElectrogramme(BRect frame, const char *name, BOOL rightChannel,
															 ULONG resizeMode, ULONG flags)
							 :BView(frame, name, resizeMode, flags|B_PULSE_NEEDED)
//********************************
{
	mRightChannel = rightChannel;
	mAllocWidth = max(200,frame.Width());
	mValue = new SHORT[mAllocWidth];
	if (!mValue) mAllocWidth = 0;
	else memset(mValue, 0, sizeof(SHORT)*mAllocWidth);
}


//********************************
CElectrogramme::~CElectrogramme(void)
//********************************
{
	if (mValue) delete mValue;
	mAllocWidth = 0;
}


//*****************************************
void CElectrogramme::AttachedToWindow(void)
//*****************************************
{
	if (Window() && Window()->Lock())
	{
		SetViewColor(K_ELECTRO_GRAY);
		Window()->Unlock();
	}
}


//********************************
void CElectrogramme::Pulse(void)
//********************************
{
	if (Window() && Window()->Lock())
	{
		Draw(Bounds());
		Window()->Unlock();
	}
}

//********************************
void CElectrogramme::Draw(BRect r)
//********************************
// be sure this is not efficient at all. Ou presque.
{
register ULONG nbuf;
register ULONG w,h;
register ULONG i;
rgb_color backgroundColor = {K_ELECTRO_GRAY};
rgb_color liveColor = {K_ELECTRO_GREEN};
register CTrackerApp *app = (CTrackerApp *)be_app;
register SBufferItem *item;

	if (!Window() || !Window()->Lock()) return;

	w = min(Bounds().Width(), mAllocWidth);
	h = Bounds().Height();

	// first eraase the current data buffer
	memset(mValue, 0, w*sizeof(SHORT));
	
	app->mBufferLock.Lock();

	register SHORT *val = mValue;
	register DOUBLE haut= (DOUBLE)h/2/16.0/32768.0;
	for(i=0, nbuf=0; i<w; /* no inc */ )
	{
		item = (SBufferItem *)app->mBufferList.ItemAt(nbuf);
		if (item && item->data)
		{
			register ULONG j;
			register SHORT *ptr = (SHORT *)item->data;
			if (mRightChannel) ptr++;
			for(j=0; j<2048 && i<w; j+=32, ptr+=32, i++)
			{
				register LONG a;
				a  = ptr[ 0];
				a += ptr[ 2];
				a += ptr[ 4];
				a += ptr[ 6];
				a += ptr[ 8];
				a += ptr[10];
				a += ptr[12];
				a += ptr[14];
				a += ptr[16];
				a += ptr[18];
				a += ptr[20];
				a += ptr[22];
				a += ptr[24];
				a += ptr[26];
				a += ptr[28];
				a += ptr[30];
				*(val++) = haut*a;	// division par 16 dans 'haut'
			}
		}
		else break;
		nbuf++;
	}

	app->mBufferLock.Unlock();

	// first erase old chart
	SetHighColor(backgroundColor);
	FillRect(r);

	// second draw new chart
	if (w>0)
	{
		BeginLineArray(w);
		//for(i=0; i<w; i++)
		//	AddLine(BPoint(i,mid), BPoint(i,mid-mValue[i]), liveColor);
		register ULONG mid=h/2;
		register ULONG end=w-1;
		register ULONG i2=1;
		register USHORT a=mid-mValue[0];
		for(i=0;i<end;i++,i2++)
			AddLine(BPoint(i,a), BPoint(i2,(a=mid-mValue[i2])), liveColor);
	}
	EndLineArray();
	Sync();

	Window()->Unlock();
}


//**********************************************************
void CElectrogramme::FrameResized(float width, float height)
//**********************************************************
{
ULONG w=width;
	if (!Window() || !Window()->Lock()) return;

	if (w > mAllocWidth)
	{
		SHORT *newPtr = new SHORT[w];
		if (newPtr)
		{
			memset(newPtr, 0, w*sizeof(SHORT));
			if (mValue)
			{
				memcpy(newPtr, mValue, mAllocWidth*sizeof(SHORT));
				delete mValue;
			}
			mValue = newPtr;
			mAllocWidth = w;
		}
	}

	Window()->Unlock();
}


//--------------------------------------------------------------------------------
