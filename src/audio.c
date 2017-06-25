/* audio.c */

/* $Id: audio.c,v 3.9 1993/11/17 15:30:16 espie Exp espie $
 * $Log: audio.c,v $
 * Revision 3.9  1993/11/17  15:30:16  espie
 * Added one level of abstraction. Look through player.c for the
 * old high-level functions.
 *
 * Revision 3.8  1993/08/04  11:34:33  espie
 * *** empty log message ***
 *
 * Revision 3.7  1993/07/18  10:39:44  espie
 * Cleaned up some code.
 *
 * Revision 3.6  1992/11/27  10:29:00  espie
 * General cleanup
 *
 * Revision 3.5  1992/11/24  10:51:19  espie
 * Optimized output and fixed up some details.
 * Unrolled code for oversample = 1 to be more efficient at
 * higher frequency (since a high frequency is better than
 * a higher oversample).
 *
 * Revision 3.4  1992/11/23  17:18:59  espie
 * *** empty log message ***
 *
 * Revision 3.3  1992/11/23  10:12:23  espie
 * Fixed up BIG bug.
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
 * Revision 2.14  1992/11/06  19:31:53  espie
 * Fixed missing parameter type.
 * fix_xxx for better speed.
 * set_volume.
 * Added possibility to get back to MONO for the sgi.
 * Added stereo capabilities to the indigo version.
 * Minor bug: a SAMPLE_FAULT is a minor error,
 * we should first check that there was no other
 * error before setting it.
 * New resample function coming from the player.
 * Added more notes.
 *
 * Revision 2.1  1991/11/17  23:07:58  espie
 * Just computes some frequency-related parameters.
 *
 *
 */

#include <math.h>
//#include <malloc.h>
#include <stdio.h>

#include "defs.h"
#include "extern.h"
#include "song.h"
#include "channel.h"
     
LOCAL char *id = "$Id: audio.c,v 3.9 1993/11/17 15:30:16 espie Exp espie $";


/* DO_NOTHING is also used for the automaton */
#define DO_NOTHING 0
#define PLAY 1
#define REPLAY 2

struct audio_channel
	{
	struct sample_info *samp;
	int mode;
	unsigned long pointer;
	unsigned long step;
	int volume;
	int pitch;
	} chan[4];

LOCAL struct sample_info dummy =
	{
	NULL,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	NULL,
	NULL
	};

LOCAL int allocated = 0;

struct audio_channel *new_channel()
	{
	struct audio_channel *new;
	new = &chan[allocated++];
	new->mode = DO_NOTHING;
	new->pointer = 0;
	new->step = 0;
	new->pitch = 0;
	new->volume = 0;
	new->samp = &dummy;
	return new;
	}

void no_audio_channels()
	{
	allocated = 0;
	}

/* Have to get some leeway for vibrato (since we don't bound pitch with
 * vibrato). This is conservative.
 */
#define VIB_MAXDEPTH 150


#define C fix_to_int(ch->pointer)

LOCAL unsigned long step_table[MAX_PITCH + VIB_MAXDEPTH];  
                    /* holds the increment for finding the next sampled
                     * byte at a given pitch (see resample() ).
                     */

/* creates a table for converting ``amiga'' pitch
 * to a step rate at a given resampling frequency.
 * For accuracy, we don't use floating point, but
 * instead fixed point ( << ACCURACY).
 * IMPORTANT NOTE: we need to make it fit within 32 bits (long), which
 * must be enough for ACCURACY + log2(max sample length)
 */
LOCAL void create_step_table(oversample, output_fr)
int oversample;     /* we sample oversample i for each byte output */
int output_fr;      /* output frequency */
    {
    double note_fr; /* note frequency (in Hz) */
    double step;
    int pitch;      /* amiga pitch */

    step_table[0] = 0;
    for (pitch = 1; pitch < MAX_PITCH + VIB_MAXDEPTH; pitch++)
        {
        note_fr = AMIGA_CLOCKFREQ / pitch;
            /* int_to_fix(1) is the normalizing factor */
        step = note_fr / output_fr * int_to_fix(1) / oversample;
        step_table[pitch] = (long)step;
        }
    }
         
LOCAL void readjust_pitch()
	{
	int i;
	for (i = 0; i < allocated; i++)
		chan[i].step = step_table[chan[i].pitch];
	}

void init_tables(oversample, frequency, chan)
int oversample, frequency;
struct channel *chan;
    {
    create_step_table(oversample, frequency);
	readjust_pitch();
    }


/* The playing mechanism itself.
 * According to the current channel automata,
 * we resample the instruments in real time to
 * generate output.
 */
void resample(chan, oversample, number)
struct channel *chan;
int oversample;
int number;
    {
    int i;          /* sample counter */
    int channel;    /* channel counter */
    int sampling;   /* oversample counter */
    SAMPLE sample;  /* sample from the channel */
    int value[NUMBER_TRACKS];
                    /* recombinations of the various data */
    struct audio_channel *ch;

		/* safety check: we can't have a segv there, provided
		 * chan points to a valid sample.
		 * For `empty' samples, what is needed is fix_length = 0
		 * and rp_start = NULL
		 */
	/* special case optimization */
	if (oversample == 1)
		{
			/* do the resampling, i.e., actually play sounds */
		for (i = 0; i < number; i++) 
			{
			for (channel = 0; channel < NUMBER_TRACKS; channel++)
				{
				ch = chan[channel].audio;
				switch(ch->mode)
					{
				case DO_NOTHING:
					value[channel] = 0;
					break;
				case PLAY:
						/* Since we now have fix_length, we can
						 * do that check with improved performance
						 */
					if (ch->pointer < ch->samp->fix_length)
						{
						value[channel] = ch->samp->start[C] * ch->volume;
						ch->pointer += ch->step;
						break;
						}
					else
						{
						ch->mode = REPLAY;
						ch->pointer -= ch->samp->fix_length;
						}
				case REPLAY:
							/* is there a replay ? */
					if (!ch->samp->rp_start)
						{
						ch->mode = DO_NOTHING;
						break;
						}
					while (ch->pointer >= ch->samp->fix_rp_length)
						ch->pointer -= ch->samp->fix_rp_length;
					value[channel] = ch->samp->rp_start[C] * ch->volume;
					ch->pointer += ch->step;
					break;
					}
				} 
			output_samples(value[0]+value[3], value[1]+value[2]);
			}
		}   
	else
		{
		for (i = 0; i < number; i++) 
			{
			for (channel = 0; channel < NUMBER_TRACKS; channel++)
				{
				value[channel] = 0;
				for (sampling = 0; sampling < oversample; sampling++)
					{
					ch = chan[channel].audio;
					switch(ch->mode)
						{
					case DO_NOTHING:
						break;
					case PLAY:
							/* Since we now have fix_length, we can
							 * do that check with improved performance
							 */
						if (ch->pointer < ch->samp->fix_length)
							{
							value[channel] += ch->samp->start[C] * ch->volume;
							ch->pointer += ch->step;
							break;
							}
						else
							{
							ch->mode = REPLAY;
							ch->pointer -= ch->samp->fix_length;
							}
					case REPLAY:
								/* is there a replay ? */
						if (!ch->samp->rp_start)
							{
							ch->mode = DO_NOTHING;
							break;
							}
						while (ch->pointer >= ch->samp->fix_rp_length)
							ch->pointer -= ch->samp->fix_rp_length;
						value[channel] += ch->samp->rp_start[C] * ch->volume;
						ch->pointer += ch->step;
						break;
						}
					} 
				}
			output_samples((value[0]+value[3])/oversample, 
				(value[1]+value[2])/oversample);
			}   
		}


    flush_buffer();
    }


/* setting up a given note */

void play_note(au, samp, pitch)
struct audio_channel *au;
struct sample_info *samp;
int pitch;
    {
	au->pointer = 0;
	au->pitch = pitch;
	au->step = step_table[pitch];
	if (samp)
		{
		au->samp = samp;
		au->mode = PLAY;
		}
	else
		au->mode = DO_NOTHING;
    }

/* changing the current pitch (value
 * may be temporary, and not stored
 * in channel pitch, for instance vibratos.
 */
void set_play_pitch(au, pitch)
struct audio_channel *au;
int pitch;
    {
		/* save current pitch in case we want to change
		 * the step table on the run
		 */
	au->pitch = pitch;
    au->step = step_table[pitch];
    }

/* changing the current volume. You HAVE to get through
 * there so that it will work on EVERY machine.
 */
void set_play_volume(au, volume)
struct audio_channel *au;
int volume;
    {
    au->volume = volume;
    }

void set_play_position(au, pos)
struct audio_channel *au;
int pos;
	{
	au->pointer = int_to_fix(pos);
	}

