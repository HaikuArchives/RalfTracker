/* display.c */

/* $Id: display.c,v 1.6 1993/11/17 15:31:16 espie Exp espie $
 * $Log: display.c,v $
 * Revision 1.6  1993/11/17  15:31:16  espie
 * *** empty log message ***
 *
 * Revision 1.4  1993/07/18  10:39:44  espie
 * Added last displays.
 *
 * Revision 1.3  1993/07/17  22:23:41  espie
 * Fixed bug with bad loops.
 *
 * Revision 1.2  1993/07/17  12:00:30  espie
 * Added other commands (numerous).
 *
 */
     
#include <stdio.h>
     
#include "defs.h"
#include "extern.h"
#include "song.h"
#include "channel.h"
#include "pref.h"
     
LOCAL char *num[] = {
" 0", " 1", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9",
"10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
"30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
"40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
"50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
"60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
"70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
"80", "81", "82", "83", "84", "85", "86", "87", "88", "89"};

LOCAL char instname[] = { ' ', '1', '2', '3', '4', '5', '6', '7', '9',
'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
LOCAL char *cmdname[] = {
"arp", "dwn", "up ", "prt", "vib", "pts", "vbs", "7  ", "8  ", "off", "svl", 
"ff ", "vol", "skp", "ext", "spd"};

LOCAL char *id = "$Id: display.c,v 1.6 1993/11/17 15:31:16 espie Exp espie $";
     
LOCAL void (*table[NUMBER_EFFECTS])();

BOOL show;

LOCAL char *name_note(note)
int note;
	{
	if (note != NO_NOTE)
		return note_name[note];
	else
		return "   ";
	}

void disp_default(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s        |", instname[samp], name_note(note));
	}

void disp_speed(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (para < 32)
		printf("%c %s SPD  %2d|", instname[samp], name_note(note), para);
	else
		printf("%c %s spd%%%3d|", instname[samp], name_note(note), 
			para * 100/NORMAL_FINESPEED);
	}

void disp_nothing(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
		printf("             |");
	}

void disp_portamento(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (para)
		printf("%c -->%s(%3d)|", instname[samp], name_note(note),
			para);
	else
		printf("%c -->%s     |", instname[samp], name_note(note));
	}

void disp_portaslide(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (LOW(para))
		printf("%c -->%s  -%2d|", instname[samp], name_note(note), LOW(para));
	else
		printf("%c -->%s  +%2d|", instname[samp], name_note(note), HI(para));
	}

void disp_upslide(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (para)
		printf("%c %s   -%3d |", instname[samp], name_note(note), para);
	else
		printf("%c %s   -    |", instname[samp], name_note(note));
	}

void disp_downslide(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (para)
		printf("%c %s   +%3d |", instname[samp], name_note(note), para);
	else
		printf("%c %s   +    |", instname[samp], name_note(note));
	}

void disp_vibrato(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (para)
		printf("%c %s vb%2d/%2d|", instname[samp], name_note(note),
			LOW(para), HI(para));
	else
		printf("%c %s vb     |", instname[samp], name_note(note));
	}

void disp_vibratoslide(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (LOW(para))
		printf("%c %s vibs-%2d|", instname[samp], name_note(note),
			LOW(para));
	else
		printf("%c %s vibs+%2d|", instname[samp], name_note(note),
			HI(para));
	}

void disp_slidevol(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (LOW(para))
		printf("%c %s vol -%2d|", instname[samp], name_note(note),
			LOW(para));
	else
		if (HI(para))
			printf("%c %s vol +%2d|", instname[samp], name_note(note),
				HI(para));
		else
			printf("%c %s        |", instname[samp], name_note(note));
	}

void disp_volume(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (para)
		printf("%c %s vol %3d|", instname[samp], name_note(note),
			para);
	else
		printf("%c %s silent |", instname[samp], name_note(note));
	}

void disp_arpeggio(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (note != NO_NOTE)
		printf("%c %s %s %s|", instname[samp], note_name[note], 
			note_name[note + LOW(para)], note_name[note + HI(para)]);
	else
		if (ch->note == NO_NOTE)
			printf("Arpeggio error  |");
		else
			printf("%c     %s %s|", instname[samp], 
			note_name[ch->note + LOW(para)], note_name[ch->note + HI(para)]);
	}

void disp_retrig(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s rtg%3d |", instname[samp], name_note(note), para);
	}

void disp_note_cut(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s cut%3d |", instname[samp], name_note(note), para);
	}

void disp_late_start(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s lte%3d |", instname[samp], name_note(note), para);
	}

void disp_offset(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s off%3d%%|", instname[samp], name_note(note), 
		para * 25600/ ch->samp->length);
	}

void disp_smooth_up(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s sth-%3d|", instname[samp], name_note(note),
		para);
	}

void disp_smooth_down(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s sth+%3d|", instname[samp], name_note(note),
		para);
	}
void disp_smooth_upvolume(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s   ++%3d|", instname[samp], name_note(note),
		para);
	}
void disp_smooth_downvolume(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s   --%3d|", instname[samp], name_note(note),
		para);
	}

void disp_change_finetune(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s fine %2d|", instname[samp], name_note(note),
		para);
	}

void disp_skip(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (para)
		printf("%c %s skp %3d|", instname[samp], name_note(note), para);
	else
		printf("%c %s  next  |", instname[samp], name_note(note));
	}

void disp_fastskip(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s ff  %3d|", instname[samp], name_note(note), para);
	}

void disp_loop(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	if (para == 0)
		printf("%c %s SETLOOP|", instname[samp], name_note(note));
	else
		printf("%c %s LOOP%3d|", instname[samp], name_note(note), para);
	}

void disp_delay_pattern(samp, para, note, ch)
int samp, para, note;
struct channel *ch;
	{
	printf("%c %s DLAY%3d|", instname[samp], name_note(note), para);
	}

#define disp_nothing disp_default

void init_display()
	{
	int i;

	for (i = 0; i < NUMBER_EFFECTS; i++)
		table[i] = disp_nothing;
    table[EFF_ARPEGGIO] = disp_arpeggio;
    table[EFF_SPEED] = disp_speed;
    table[EFF_SKIP] = disp_skip;
    table[EFF_FF] = disp_fastskip;
    table[EFF_VOLUME] = disp_volume;
    table[EFF_VOLSLIDE] = disp_slidevol;
    table[EFF_OFFSET] = disp_offset;
    table[EFF_PORTA] = disp_portamento;
    table[EFF_PORTASLIDE] = disp_portaslide;
    table[EFF_UP] = disp_upslide;
    table[EFF_DOWN] = disp_downslide;
    table[EFF_VIBRATO] = disp_vibrato;
    table[EFF_VIBSLIDE] = disp_vibratoslide;
	table[EFF_SMOOTH_UP] = disp_smooth_up;
	table[EFF_SMOOTH_DOWN] = disp_smooth_down;
	table[EFF_CHG_FTUNE] = disp_change_finetune;
	table[EFF_LOOP] = disp_loop;
	table[EFF_RETRIG] = disp_retrig;
	table[EFF_S_UPVOL] = disp_smooth_upvolume;
	table[EFF_S_DOWNVOL] = disp_smooth_downvolume;
	table[EFF_NOTECUT] = disp_note_cut;
	table[EFF_LATESTART] = disp_late_start;
	table[EFF_DELAY] = disp_delay_pattern;
    }

void dump_event(ch, e, imask)
struct channel *ch;
struct event *e;
unsigned long imask;
	{
	int samp;
	int cmd;

	(*table[e->effect])(e->sample_number, e->parameters, e->note, ch);
	
	fflush(stdout);
	}

