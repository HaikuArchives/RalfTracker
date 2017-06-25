/* main.c */

/* plays sound/noisetracker files on Sparc, silicon graphics.
 * Authors  : Liam Corner - zenith@dcs.warwick.ac.uk
 *            Marc Espie - espie@ens.fr
 *            Steve Haehnichen - shaehnic@ucsd.edu
 *            Andrew Leahy - alf@st.nepean.uws.edu.au
 *
 * Usage    : tracker <filename> 
 *  this version plays compressed files as well.
 */

/* $Id: main.c,v 3.17 1993/11/17 15:31:16 espie Exp espie $
 * $Log: main.c,v $
 * Revision 3.17  1993/11/17  15:31:16  espie
 * New version.
 *
 * Revision 3.16  1993/11/11  20:00:03  espie
 * Amiga support.
 *
 * Revision 3.15  1993/08/04  11:55:21  espie
 * Fixed upo previous song bug.
 *
 * Revision 3.13  1993/07/18  10:39:44  espie
 * Added forking under unix. Experimental...
 *
 * Revision 3.11  1993/05/09  14:06:03  espie
 * Fixed up bug with mix option no longer working.
 *
 * Revision 3.10  1993/04/25  15:13:36  espie
 * Force new version.
 *
 * Revision 3.9  1993/01/15  14:00:28  espie
 * Added bg/fg test.
 *
 * Revision 3.7  1992/12/03  15:00:50  espie
 * restore stty.
 *
 * Revision 3.5  1992/11/24  10:51:19  espie
 * Added loads of new options.
 *
 * Revision 3.3  1992/11/22  17:20:01  espie
 * Augmented usage.
 *
 * Revision 3.2  1992/11/20  14:53:32  espie
 * Added finetune.
 *
 * Revision 3.1  1992/11/19  20:44:47  espie
 * Protracker commands.
 *
 * Revision 3.0  1992/11/18  16:08:05  espie
 * New release.
 *
 * Revision 2.20  1992/11/17  17:06:25  espie
 * Added PREVIOUS_SONG handling ???
 * Use streamio for new interface (obsolescent signal handlers), and
 * related changes.
 * Cleaned up path reader, and better signal handling.
 * Support for open_file.
 * Added imask.
 * Use transparent decompression/path lookup through open_file/close_file.
 * Added setup_audio().
 * Added some frequency/oversample/stereo change on the fly.
 * Necessitates rightful closing/reopening of audio.
 * Added compression methods. Changed getopt.
 * Separated mix/stereo stuff.
 * Added transpose feature.
 * Added possibility to get back to MONO for the sgi.
 * Added stereo capabilities to the indigo version.
 * Added recovery and reread for automatic recognition
 * of old/new tracker files.
 * Added two level of fault tolerancy.
 * Added more rational options.
 * Moved almost everything to audio and automaton.
 * Structured part of the code, especially replay ``automaton''
 * and setting up of effects.
 *
 * Revision 1.26  1991/11/17  17:09:53  espie
 * Added missing prototypes.
 * Some more info while loading files.
 * Added FAULT env variable, FAULT resistant playing,
 * for playing modules which are not quite correct.
 * Serious bug: dochangespeed was not reset all the time.
 * Check all these parameters, they MUST be reset for
 * each new song.
 * Fixed a stupid bug: when env variable LOOPING was
 * undefined, we got a segv on strcmp.
 * Now we just test for its existence, since this is
 * about all we want...
 * Bug correction: when doing arpeggio, there might not
 * be a new note, so we have to save the old note value
 * and do the arppeggio on that note.
 * Completely added control with OVERSAMPLE and FREQUENCY.
 * Added control flow.
 * Added pipe decompression, so that now you can do
 * str32 file.Z directly.
 * stdin may go away.
 * Added arpeggio.
 * Added vibslide and portaslide.
 * Added speed command.
 * Added signal control.
 * Error checking: there shouldn't be that many
 * segv signals any more.
 * Moved every command to commands.c.
 * Added some debug code for showing the full
 * sequence for a file.
 * Corrected the bug in volume slide: there is
 * no default value, i.e., if it is 0, it is 0,
 * as stupid as it may seem.
 * Added vibrato.
 * Added fastskip/corrected skip.
 * Modified control flow of the player till
 * it looks like something reasonable (i.e.,
 * the structure is more natural and reflects
 * the way stuff is played actually...)
 * Do not restart the sound when we change instruments
 * on the fly. A bit strange, but it works that way.
 * Modified main to use new data structures.
 * The sound player is MUCH cleaner, it uses now
 * a 3-state automaton for each voice.
 * Corrected ruckus with data type of sample.
 */
     

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#ifndef AMIGA
//#include <malloc.h>
#endif
     
#include "defs.h"
#include "extern.h"
#include "song.h"
#include "pref.h"
     
#include "getopt.h"
     
LOCAL char *id = "$Id: main.c,v 3.17 1993/11/17 15:31:16 espie Exp espie $";

int pid = 0;
int forked = FALSE;

#define USAGE \
"[options] filename [...]\n\
-help               Display usage information\n\
-quiet              Print no output other than errors\n\
-picky              Do not tolerate any faults (default is to ignore most)\n\
-tolerant           Ignore all faults\n\
-mono               Select single audio channel output\n\
-stereo             Select dual audio channel output\n\
-verbose            Show text representation of song\n\
-repeats <count>    Number of repeats (0 is forever) (default 1)\n\
-speed <speed>      Song speed.  Some songs want 60 (default 50)\n\
-mix <percent>      Percent of channel mixing. (0 = spatial, 100 = mono)\n\
-new -old -both     Select default reading type (default is -both)\n\
-frequency <freq>   Set playback frequency in Hz\n\
-oversample <times> Set oversampling factor\n\
-transpose <n>      Transpose all notes up\n\
-show               Show what's going on\n\
-sync               Try to synch audio output with display\n\
\n\
RunTime:\n\
r       restart current song\n\
e,x     exit program\n\
n       next song\n\
p       previous song (experimental)\n\
>       fast forward\n\
<       rewind\n\
S       NTSC tempo\t s\tPAL tempo\n"

/* Command-line options. */
LOCAL struct long_option long_options[] =
{
  {"help",              0, 'H'},
  {"quiet",             0, 'Q'}, 
  {"picky",             0, 'P'},
  {"tolerant",          0, 'L'},
  {"new",               0, 'N'},
  {"old",               0, 'O'},
  {"both",              0, 'B'},
  {"mono",              0, 'M'},
  {"stereo",            0, 'S'},
  {"verbose",           0, 'V'},
  {"frequency",         1, 'f'},
  {"oversample",        1, 'o'},
  {"transpose",         1, 't'},
  {"repeats",           1, 'r'},
  {"speed",             1, 's'},
  {"mix",               1, 'm'},
  {"start",             1, 'X'},
  {"cut",               1, '-'},
  {"add",               1, '+'},
  {"show",              0, 'v'},
  {"sync",              0, '='},
  {0,                   0,  0 }
};



//**********************************************************************************
//**********************************************************************************
//**********************************************************************************
// prototypes for functions that are exported
void setup_play_track(long freq, int stereo);
void close_play_track(void);
void play_track(char *filename);

//**********************************************************************************
//**********************************************************************************
//**********************************************************************************
// the external function to call when the tracker wants to exit
// (WARNING : these flags are obsoletes and must be removed -- must not be used)
void (*gBeAudioExit)(void) = NULL;

// a flag set before main is called to tell it to random file playing
int gBeAudioRandomFile = TRUE;

// a flag set by the app to go next module or to quit
int gBeAudioNextModule = FALSE;
int gBeAudioQuitModule = FALSE;



//**********************************************************************************
struct SSongName
{
	char *name;
	struct SSongName *next;
};

struct SSongName *gSongNameRoot = NULL;
struct SSongName *gSongNameCurrent = NULL;
long int gNumberOfSongName = 0;

//**********************************************************************************
void addSongName(char *name)
//**********************************************************************************
{
	struct SSongName *cell = (struct SSongName *)malloc(sizeof(struct SSongName));
	struct SSongName *p = gSongNameRoot;
	long int index = gNumberOfSongName;

	cell->name = name;
	cell->next = NULL;

	if (gBeAudioRandomFile) index = rand() % gNumberOfSongName;
	if (index > 0)
	{
		long int count = 0;
		//printf("adding song %s at index %d\n", name, index);
		while(p && p->next && (count++ < index)) p = p->next;
		if (p)
		{
			cell->next = p;
			p->next = cell;
			gNumberOfSongName++;
			return;
		}
	}

	//printf("adding song %s as root\n", name);
	if (gBeAudioRandomFile) srand(time(NULL));
	cell->next = gSongNameRoot;
	gSongNameCurrent = gSongNameRoot = cell;
	gNumberOfSongName++;

	
}


//**********************************************************************************
void listSongName(void)
//**********************************************************************************
{
	struct SSongName *p = gSongNameCurrent;
	while(p)
	{
		printf("%s", p->name);
		p = p->next;
	}
}


//**********************************************************************************
struct SSongName * getSongName(void)
//**********************************************************************************
{
	struct SSongName *p = gSongNameCurrent;
	if (p) gSongNameCurrent = gSongNameCurrent->next;
	return p;
}

//**********************************************************************************
void audioExit(long code)
//**********************************************************************************
{
	if (gBeAudioExit) gBeAudioExit();
	else exit(code);
}

//**********************************************************************************
/* global variable to catch various types of errors
 * and achieve the desired flow of control
 */
int error;

//**********************************************************************************
LOCAL int optvalue(def)
int def;
    {
    int d;
        if (sscanf(optarg, "%d", &d) == 1)
            return d;
        else
            {
            optind--;
            return def;
            }
    }

//**********************************************************************************
void end_all()
    {
	if (run_in_fg())
		printf("\n");
    do_close_audio();
	sane_tty();
    audioExit(0);
    }

//**********************************************************************************
LOCAL struct song *do_read_song(name, type)
char *name;
int type;
    {
    struct song *song;
    FILE *file;

	if (run_in_fg())
		fflush(stdout); 

    file = open_file(name, "r", getenv("MODPATH"));
    if (!file)
        return NULL;
    song = read_song(file, type); 
    close_file(file);
    return song;
    }



//**********************************************************************************
int old_tracker_main(argc, argv)
int argc;
char **argv;
    {
    int ask_freq;
    int oversample;
    int stereo;
    int start;
    int transpose;


    struct pref pref;
    struct song *song;
	BOOL *is_song;
    int c;
	int i;
    int default_type;


	is_song = (BOOL *)malloc(sizeof(BOOL) * argc);
	if (!is_song)
		end_all();

	for (i = 0; i < argc; i++)
		is_song[i] = FALSE;			/* For termination */

    start = 0;
    pref.imask = 0;
    pref.bcdvol = 0;
    pref.dump_song = FALSE;
    show = FALSE;
    pref.sync = FALSE;


    if (argc == 1)
        {
        fprintf(stderr, "Usage: %s %s", argv[0], USAGE);
		end_all();
        }

    ask_freq = read_env("FREQUENCY", 0);
    oversample = read_env("OVERSAMPLE", 1);
    transpose = read_env("TRANSPOSE", 0);
    stereo = !getenv("MONO");
	set_mix(30);

    create_notes_table();

        /* check the command name for default reading type */

    default_type = BOTH;

    pref.type = default_type;
    pref.repeats = 1;
    pref.speed = 50;
    pref.tolerate = 1;

    for (optind = 1; optind < argc; optind++)
        {
        while ((c = getopt(argc, argv, long_options))
                != EOF)
            switch(c)
                {
            case '-':
                if (strcmp(optarg, "all") == 0)
                    pref.imask = ~0;
                else
                    pref.imask |= 1L << optvalue(0);
                break;
            case '+':
                if (strcmp(optarg, "all") == 0)
                    pref.imask = 0;
                else
                    pref.imask &= ~ (1L << optvalue(0));
                break;
            case 'O':   /* old tracker type */
                pref.type = OLD;
                break;
            case 'N':   /* new tracker type */
                pref.type = NEW;
                break;
            case 'v':
                show = TRUE;
                break;
            case '=':
                pref.sync = TRUE;
                break;
            case 'B':   /* both tracker types */
                pref.type = BOTH;
                break;
            case 'r':   /* number of repeats */
                pref.repeats = optvalue(0);
                break;
            case 's':   /* speed */
                pref.speed = optvalue(50);
                break;
            case 'M':   /* MONO */
                stereo = 0;
                break;
            case 'S':   /* STEREO */
                stereo = 1;
                break;
            case 'o':   /* oversample */
                oversample = optvalue(1);
                break;
            case 'f':   /* frequency */
                ask_freq = optvalue(0);
                break;
            case 't':   /* transpose */
                transpose = optvalue(0);
                break;
            case 'P':
                pref.tolerate = 0;
                break;
            case 'L':
                pref.tolerate = 2;
                break;
            case 'm':   /* % of channel mix. 
                         * 0->full stereo, 100->mono */
                set_mix(optvalue(30));
                break;
            case 'X':
                start = optvalue(0);
                break;
            case 'H':   /* template */
                fprintf(stderr, "Usage: %s %s", argv[0], USAGE);
				end_all();
            case 'V':
                pref.dump_song = TRUE;
                }

        if (optind < argc) addSongName(argv[optind]);
		} // for

		while(!gBeAudioQuitModule)
		{
			struct SSongName * cell = getSongName();
			if (!cell) audioExit(0);

			printf("Module : %s\n", cell->name);

			song = do_read_song(cell->name, NEW);
			if (!song && error != NEXT_SONG) song = do_read_song(cell->name, OLD);
			if (!song) audioExit(0);
      if (pref.dump_song) dump_song(song); 
      transpose_song(song, transpose);
      setup_audio(ask_freq, stereo, oversample, pref.sync);
      play_song(song, &pref, start);
      release_song(song);
      printf("\n");
		}
 
 		audioExit(0);
    return 0;
    /* NOT REACHED */
}


//**********************************************************************************
void setup_play_track(long freq, int stereo)
//**********************************************************************************
{
	setup_audio(freq, stereo, TRUE, FALSE);
}

//**********************************************************************************
void close_play_track(void)
//**********************************************************************************
{
	do_close_audio();
}


//**********************************************************************************
void play_track(char *filename)
//**********************************************************************************
{
	struct song *song;
	struct pref pref;

	if (!filename) return;

	pref.imask = 0;
	pref.bcdvol = 0;
	pref.dump_song = FALSE;
	pref.sync = FALSE;
	pref.type = BOTH;
	pref.repeats = 1;
	pref.speed = 50;
	pref.tolerate = 1;

	set_mix(30);
	create_notes_table();

	song = do_read_song(filename, NEW);
	if (!song && error != NEXT_SONG) song = do_read_song(filename, OLD);
	if (song)
	{
		transpose_song(song, FALSE);
		setup_audio(0, TRUE, TRUE, FALSE);
		play_song(song, &pref, 0);
		release_song(song);
	}
}


