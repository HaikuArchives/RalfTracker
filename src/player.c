/* player.c */

/* $Id: player.c,v 3.17 1993/11/19 14:27:06 espie Exp espie $
 * $Log: player.c,v $
 * Revision 3.17  1993/11/19  14:27:06  espie
 * Stupid bug.
 *
 * Revision 3.16  1993/11/17  15:31:16  espie
 * New high-level functions.
 *
 * Revision 3.15  1993/11/11  20:00:03  espie
 * Amiga support.
 *
 * Revision 3.14  1993/08/04  11:34:33  espie
 * *** empty log message ***
 *
 * Revision 3.13  1993/07/18  11:49:29  espie
 * Bug with delay_pattern: can't factorize the check for effect thingy.
 *
 * Revision 3.12  1993/07/18  10:39:44  espie
 * Added forking under unix.
 *
 *
 * Revision 3.10  1993/05/09  14:06:03  espie
 * Reniced verbose output display.
 *
 * Revision 3.9  1993/04/25  14:08:15  espie
 * Bug fix: now use correct finetune when loading samples/starting notes.
 *
 * Revision 3.8  1993/01/15  14:00:28  espie
 * Added bg/fg test.
 *
 * Revision 3.6  1992/11/27  10:29:00  espie
 * General cleanup
 *
 * Revision 3.5  1992/11/24  10:51:19  espie
 * un#ifdef'ed showseq code.
 *
 * Revision 3.3  1992/11/22  17:20:01  espie
 * Added <> operators.
 * Added update frequency on the fly.
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
 * Revision 2.19  1992/11/17  17:15:37  espie
 * Added interface using may_getchar(). Still primitive, though.
 * imask, start.
 * Added transpose feature.
 * Added possibility to get back to MONO for the sgi.
 * Added stereo capabilities to the indigo version.
 * Added two level of fault tolerancy.
 * Added some control on the number of replays,
 * and better error recovery.
 */
     
#include <stdio.h>
     
#include "defs.h"
#include "extern.h"
#include "song.h"
#include "channel.h"
#include "pref.h"
     

LOCAL char *id = "$Id: player.c,v 3.17 1993/11/19 14:27:06 espie Exp espie $";
     

// a flag set by the app to go next module or to quit
extern int gBeAudioNextModule;

void make_effects(int cmd, struct automaton *a, struct channel *ch);

/* setting up a given note */

void reset_note(ch, note, pitch)
struct channel *ch;
int note;
int pitch;
    {
		/*
    ch->pointer = 0;
    ch->mode = PLAY;
	set_current_pitch(ch, pitch);
		*/
    ch->pitch = pitch;
    ch->note = note;
    ch->viboffset = 0;
	play_note(ch->audio, ch->samp, pitch);
    }

/* changing the current pitch (value
 * may be temporary, and not stored
 * in channel pitch, for instance vibratos.
 */
void set_current_pitch(ch, pitch)
struct channel *ch;
int pitch;
    {
		/* save current pitch in case we want to change
		 * the step table on the run
	ch->cpitch = pitch;
    ch->step = step_table[pitch];
		*/
	set_play_pitch(ch->audio, pitch);
    }

/* changing the current volume. You HAVE to get through
 * there so that it will work on EVERY machine.
 */
void set_current_volume(ch, volume)
struct channel *ch;
int volume;
    {
    ch->volume = MAX(MIN(volume, MAX_VOLUME), MIN_VOLUME);
	set_play_volume(ch->audio, volume);
    }

void set_position(ch, pos)
struct channel *ch;
int pos;
	{
	set_play_position(ch->audio, pos);
	/*
	ch->pointer = int_to_fix(pos);
	*/
	}

/* init_channel(ch, dummy):
 * setup channel, with initially
 * a dummy sample ready to play,
 * and no note.
 */
LOCAL void init_channel(ch)
struct channel *ch;
    {
    ch->samp = NULL;
	ch->finetune = 0;
	ch->audio = new_channel();
    ch->volume = 0; 
    ch->pitch = 0; 
    ch->note = NO_NOTE;

        /* we don't setup arpeggio values. */
    ch->viboffset = 0; 
    ch->vibdepth = 0;

    ch->slide = 0; 

    ch->pitchgoal = 0; 
    ch->pitchrate = 0;

    ch->volumerate = 0;

    ch->vibrate = 0;
    ch->adjust = do_nothing;
    }



LOCAL int VSYNC;          /* base number of sample to output */
void (*eval[NUMBER_EFFECTS])();
                    /* the effect table */
LOCAL int oversample;     /* oversample value */
LOCAL int frequency;      /* output frequency */
LOCAL int channel;        /* channel loop counter */

LOCAL struct channel chan[NUMBER_TRACKS];
                    /* every channel */
LOCAL int countdown;      /* keep playing the tune or not */

LOCAL struct song_info *info;
LOCAL struct sample_info *voices;

LOCAL struct automaton a;


void init_player(o, f)
int o, f;
    {
    oversample = o;
    frequency = f;
    init_tables(o, f, NULL);
    init_effects(eval);
	init_display();
    }

LOCAL void setup_effect(ch, a, e, imask, bcdvol)
struct channel *ch;
struct automaton *a;
struct event *e;
unsigned long imask;
int bcdvol;
    {
    int samp, cmd;

        /* retrieves all the parameters */
    samp = e->sample_number;

        /* load new instrument */
    if (samp)  
        {
            /* note that we can change sample in the middle
             * of a note. This is a *feature*, not a bug (see
             * made). Precisely: the sampel change will be taken
             * into account for the next note, BUT the volume change
             * takes effect immediately.
             */
        ch->samp = voices + samp;
		ch->finetune = voices[samp].finetune;
        if ((1L<<samp) & imask)
            ch->samp = voices;
        set_current_volume(ch, voices[samp].volume);
        }

    a->note = e->note;
    if (a->note != NO_NOTE)
        a->pitch = pitch_table[a->note][ch->finetune];
    else
        a->pitch = e->pitch;
    cmd = e->effect;
    a->para = e->parameters;

    if (a->pitch >= MAX_PITCH)
        {
        fprintf(stderr, "Pitch out of bounds %d\n", a->pitch);
        a->pitch = 0;
        error = FAULT;
        }

	if (show && run_in_fg())
		dump_event(ch, e, imask);
        /* check for a new note: portamento
         * is the special case where we do not restart
         * the note.
         */
    if (a->pitch && cmd != EFF_PORTA && cmd != EFF_PORTASLIDE)
        reset_note(ch, a->note, a->pitch);
    ch->adjust = do_nothing;
        /* do effects */
    (eval[cmd])(a, ch);
		//make_effects(cmd, a, ch); // in commands.c
    }


LOCAL void adjust_sync(ofreq, tempo)
int ofreq, tempo;
	{
	VSYNC = ofreq * NORMAL_FINESPEED / tempo;
	}

LOCAL void play_once(a, pref)
struct automaton *a;
struct pref *pref;
	{
	int channel;

	if (a->do_stuff & DELAY_PATTERN)
		for (channel = 0; channel < NUMBER_TRACKS; channel++)
			/* do the effects */
			(chan[channel].adjust)(chan + channel);
	else
		{	
		if (a->counter == 0)
			for (channel = 0; channel < NUMBER_TRACKS; channel++)
				/* setup effects */
				setup_effect(chan + channel, a, 
					&(a->pattern->e[channel][a->note_num]), 
					pref->imask, pref->bcdvol);
		else
			for (channel = 0; channel < NUMBER_TRACKS; channel++)
				/* do the effects */
				(chan[channel].adjust)(chan + channel);
		}

		/* advance player for the next tick */
	next_tick(a);
		/* actually output samples */
	resample(chan, oversample, VSYNC / a->finespeed);
	}



void play_song(song, pref, start)
struct song *song;
struct pref *pref;
int start;
    {
	int tempo;

	tempo = pref->speed;

	adjust_sync(frequency, tempo);
    /* a repeats of 0 is infinite replays */
    if (pref->repeats)
        countdown = pref->repeats;
    else
        countdown = 1;

    info = &song->info;
    voices = song->samples; 

    init_automaton(&a, song, start);

	no_audio_channels();

    for (channel = 0; channel < NUMBER_TRACKS; channel++) 
        init_channel(chan + channel);


    while(countdown)
        {
		play_once(&a, pref);

		if (gBeAudioNextModule)
		{
			gBeAudioNextModule = FALSE;
			error = NEXT_SONG;
			return;
		}

		switch(may_getchar())
			{
		case 'n':
#ifdef FORKING
		if (forked)
			break;
		forked = TRUE;
		if (pid = fork())
			{
			nice(15);
			error = NEXT_SONG;
			return;
			}
		else
			break;
#else
			discard_buffer();
			error = NEXT_SONG;
			return;
#endif
		case 'p':
#ifdef FORKING
		if (forked)
			break;
		forked = TRUE;
		if (pid = fork())
			{
			nice(15);
			error = PREVIOUS_SONG;
			return;
			}
		else 
			break;
#else
			discard_buffer();
			error = PREVIOUS_SONG;
			return;
#endif
		case 'x':
		case 'e':
		case 'q':
			discard_buffer();
			end_all();
		case 's':
			tempo = 50;
			adjust_sync(frequency, tempo);
			break;
		case 'S':
			tempo = 60;
			adjust_sync(frequency, tempo);
			break;
		case 'r':
			discard_buffer();
			init_automaton(&a, song, start);
			no_audio_channels();
			for (channel = 0; channel < NUMBER_TRACKS; channel++) 
				init_channel(chan + channel);
			break;
		case '>':
			discard_buffer();
			init_automaton(&a, song, a.pattern_num+1);
			break;
		case '<':
			discard_buffer();
			if (a.note_num < 4)
				init_automaton(&a, song, a.pattern_num -1);
			else
				init_automaton(&a, song, a.pattern_num);
			break;
		case '?':
			dump_song(song);
			show = TRUE;
			break;
		case '!':
			show = FALSE;
			break;
		case ' ':
			while (may_getchar() == EOF)
				;
			break;
		default:
			break;
			}
		{
		int new_freq;
		if (new_freq = update_frequency())
			{
			frequency = new_freq;
			adjust_sync(frequency, tempo);
			init_tables(oversample, frequency, chan);
			}
		}

        switch(error)
            {
		case NONE:
			break;
		case ENDED:
			if (pref->repeats)
				countdown--;
			break;
		case SAMPLE_FAULT:
			if (!pref->tolerate)
				countdown = 0;
			break;
		case FAULT:
			if (pref->tolerate < 2)
				countdown = 0;
			break;
		case PREVIOUS_SONG:
		case NEXT_SONG:
		case UNRECOVERABLE:
			countdown = 0;
			break;
		default:
			break;
			}
        error = NONE;
        }
         
    }

