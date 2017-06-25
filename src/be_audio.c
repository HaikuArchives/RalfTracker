/* linux_audio.c */
/* Modified from soundblaster_audio.c by Hannu Savolainen */
/* hsavolai@cs.helsinki.fi */

//#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "defs.h"
#include "extern.h"

/*	This should be sys/soundcard.h	*/
//#include <sys/soundcard.h>

LOCAL char *id = "$Id: linux_audio.c,v 1.5 1993/11/15 17:35:55 espie Exp espie $";

LOCAL unsigned char *buffer;/* buffer for ready-to-play samples */
LOCAL short *buffer16;	/* Sure this isn't unsigned short ? */
LOCAL int buf_index;   /* can't call this index, conflicts with index(3) */
LOCAL int buf_max;
LOCAL int audio;            /* /dev/dsp */

/* are we playing stereo or not ? */
LOCAL int stereo;
/* 256th of primary/secondary source for that side. */
LOCAL int primary=512, secondary=0;
LOCAL int dsp_samplesize = 16; /* must be 8 or 16 */


void init_BeAudio(long buf_size);

long gBeAudioBufSize = 4096;
void (*gBeAudioPlayBuffer)(unsigned char *buffer, long length) = NULL;


void set_mix(percent)
int percent;
    {
    percent *= 256;
    percent /= 100;
    primary = percent;
    secondary = 512 - percent;
    }


int open_audio(f, s)
int f;
int s;
    {
       dsp_samplesize = 16;
    stereo = s;
    if (f==0) f = 44100;
    buf_max=gBeAudioBufSize;
  
     buffer = (unsigned char *)malloc(gBeAudioBufSize);    /* Stereo makes x2 */
    buffer16 = (short *)buffer;
    buf_index = 0;

        return f;
    }

LOCAL void actually_flush_buffer()
    {
    int l,i;

    l = sizeof(*buffer) * buf_index;
    if (dsp_samplesize !=8) l *= 2;
    //write(audio, buffer, l);

		if (gBeAudioPlayBuffer) gBeAudioPlayBuffer(buffer, l);

    buf_index = 0;
    }

void output_samples(left, right)
int left, right;
    {
    if (dsp_samplesize != 8)	/* Cool! 16 bits/sample */
    {
	    if (stereo)
	        {
	        if (buf_index * 2 >= buf_max - 1) 
	           actually_flush_buffer();

	        buffer16[buf_index++] = 
	           ((left*primary + right*secondary) / 256);
	        buffer16[buf_index++] = 
	           ((right*primary + left*secondary) / 256);
	        }
	    else
	        {
	        if (buf_index * 2 >= buf_max) 
	           actually_flush_buffer();
	        buffer16[buf_index++] = (left + right);
	        }
    }
    else
    {
	    if (stereo)
	        {
	        if (buf_index >= buf_max - 1) 
				actually_flush_buffer();
	        buffer[buf_index++] = ((left*primary + right*secondary) >> 16)
	             + 128;
	        buffer[buf_index++] = ((right*primary + left*secondary) >> 16)
	             + 128;
	        }
	    else
	        {
	        if (buf_index >= buf_max) 
				actually_flush_buffer();
	        buffer[buf_index++] = ((left + right) >> 8) + 128;
	        }
	    }
    }

void flush_buffer()
    {	/* Dummy version */
    }

/*
 * Closing the Linux sound device waits for all pending samples to play.
 */
void close_audio()
    {
    actually_flush_buffer();
    //close(audio);
    free(buffer);
    }

/* dummy system calls, to patch ? */
void set_synchro(s)
	{
	}

int update_frequency()
	{
	return 0;
	}

void discard_buffer()
	{
	}

