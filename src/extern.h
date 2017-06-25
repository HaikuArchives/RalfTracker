/* extern.h */

/* $Id: extern.h,v 3.13 1993/11/17 15:31:16 espie Exp espie $
 * $Log: extern.h,v $
 * Revision 3.13  1993/11/17  15:31:16  espie
 * *** empty log message ***
 *
 * Revision 3.12  1993/11/11  20:00:03  espie
 * Amiga support.
 *
 * Revision 3.10  1993/07/18  10:39:44  espie
 * Added pid, fork, show.
 *
 * Revision 3.8  1993/01/15  14:00:28  espie
 * Added bg/fg test.
 *
 * Revision 3.7  1992/12/03  15:00:50  espie
 * stty_sane.
 *
 * Revision 3.5  1992/11/24  10:51:19  espie
 * New audio functions.
 *
 * Revision 3.2  1992/11/22  17:20:01  espie
 * Simplified delay_pattern.
 *
 * Revision 3.1  1992/11/19  20:44:47  espie
 * Protracker commands.
 *
 * Revision 3.0  1992/11/18  16:08:05  espie
 * New release.
 *
 * Revision 2.19  1992/11/17  17:06:25  espie
 * Lots of new functions to account for new interface.
 * open_file support.
 * Separated mix/stereo stuff.
 * Added possibility to get back to MONO for the sgi.
 * Added stereo capabilities to the indigo version.
 * Added some new song types to automatize the choice process.
 * Moved resampling to audio, added prototype.
 * Added SAMPLE_FAULT, for trying to play
 * a note without a sample (not really an error).
 *
 * Revision 1.7  1991/11/08  14:25:55  espie
 * Modified audio prototype so that you can change
 * frequency.
 * Added prototype for release_song.
 * Added arpeggio effect.
 * Added entries for new effects.
 * Added entries for commands.c.
 */





/* audio.c */
#define ACCURACY 12
#define fix_to_int(x) ((x) >> ACCURACY)
#define int_to_fix(x) ((x) << ACCURACY)

/* init_tables(oversample, frequency, chan):
 * precomputes the step_table and the pitch_table
 * according to the desired oversample and frequency.
 * This is static, you can call it again whenever you want.
 * adjusting current parameters of channel if needed.
 */
XT void init_tables();

/* resample(chan, oversample, number):
 * send number samples out computed according
 * to the current state of chan[0:NUMBER_CHANNELS],
 * and oversample.
 */
XT void resample();

/* play_note(au, samp, pitch)
 * set audio channel au to play samp at pitch
 */
XT void play_note();

/* set_play_pitch(audio, pitch):
 * set channel audio to play at pitch pitch
 */
XT void set_play_pitch();

/* set_play_volume(audio, volume):
 * set channel audio to play at volume volume
 */
XT void set_play_volume();

/* set_play_position(audio, pos):
 * set position in sample for current channel at given offset
 */
XT void set_play_position();






/* automaton.c */
/* init_automaton(a, song):
 * put the automaton a in the right state to play song.
 */
XT void init_automaton();
/* next_tick(a):
 * set up everything for the next tick.
 */
XT void next_tick();







/* commands.c */
/* init_effects(): sets up all data for the effects */
XT void init_effects();
/* do_nothing: this is the default behavior for an effect.
 */
XT void do_nothing();








/* dump_song */
/* dump_song(s): 
 * displays some information pertinent to the given 
 * song s.
 */
XT void dump_song();


XT void init_display();





/* getopt.c */
/* n = getopt(argc, argv, options):
 * try to parse options out of argv, using
 * ways similar to standard getopt
 */
XT int getopt();






/* main.c */
XT int pid, forked;
#define OLD 0
#define NEW 1
/* special new type: for when we try to read it as both types.
 */
#define BOTH 2
/* special type: does not check the signature */
#define NEW_NO_CHECK 3


/* error types. Everything is centralized,
 * and we check in some places (see read, player and str32)
 * that there was no error. Additionnally signal traps work
 * that way too.
 */
 
/* normal state */
#define NONE 0  
/* read error */
#define FILE_TOO_SHORT 1
#define CORRUPT_FILE 2
/* trap error: goto next song right now */
#define NEXT_SONG 3
/* run time problem */
#define FAULT 4
/* the song has ended */
#define ENDED 5
/* unrecoverable problem: typically, trying to 
 * jump to nowhere land.
 */
#define UNRECOVERABLE 6
/* Missing sample. Very common error, not too serious. */
#define SAMPLE_FAULT 7
/* New */
#define PREVIOUS_SONG 8
XT int error;

/* end_all(): faaaast exit */
XT void end_all();




/* notes.c */
#define NUMBER_NOTES 120
#define NUMBER_FINETUNES 17
XT int pitch_table[NUMBER_NOTES][NUMBER_FINETUNES];
XT char note_name[NUMBER_NOTES][4];
/* create_notes_table():
 * build the correspondance pitch -> note
 */
XT void create_notes_table();
/* note = find_note(pitch):
 * find note corresponding to a given pitch
 */
XT int find_note();
/* oldtranspose = transpose_song(song, newtranspose):
 * tranpose song to a new pitch
 */
XT int transpose_song();






/* open.c */
/* handle = open_file(filename, mode, path):
 * transparently open a compressed file.
 */
XT FILE *open_file();

/* close_file(handle):
 * close a file that was opened with open_file.
 */
XT void close_file();



/* display.c */
/* dump_event(ch, e, imask):
 */
XT void dump_event();

/* show
 */
XT BOOL show;


/* player.c */

/* reset_note(ch, note, pitch):
 * set channel ch to play note at pitch pitch
 */
XT void reset_note();

/* set_current_pitch(ch, pitch):
 * set ch to play at pitch pitch
 */
XT void set_current_pitch();

/* set_current_volume(ch, volume):
 * set channel ch to play at volume volume
 */
XT void set_current_volume();

/* set_position(ch, pos):
 * set position in sample for current channel at given offset
 */
XT void set_position();

/* init_player(oversample, frequency):
 * sets up the player for a given oversample and
 * output frequency.
 * Note: we can call init_player again to change oversample and
 * frequency.
 */
XT void init_player();

/* play_song(song, pref):
 * plays the song according to the current pref.
 */
XT void play_song();







/* read.c */
/* s = read_song(f, type):
 * tries to read f as a song of type NEW/OLD.
 * returns NULL (and an error) if it doesn't work.
 * Returns a dynamic song structure if successful.
 */
XT struct song *read_song();

/* release_song(s):
 * release all the memory song occupies.
 */
XT void release_song();







/* setup_audio.c */
/* setup_audio(ask_freq, stereo, oversample, sync):
 * setup the audio output with these values 
 */
XT void setup_audio();
/* do_close_audio():
 * close audio only if needed
 */
XT void do_close_audio();







/* xxx_audio.c */
/* frequency = open_audio(f, s):
 * try to open audio with a sampling rate of f, and eventually stereo.
 * We get the real frequency back. If we ask for 0, we
 * get the ``preferred'' frequency.
 * Note: we have to close_audio() before we can open_audio() again.
 * Note: even if we don't ask for stereo, we still have to give a
 * right and left sample.
 */
XT int open_audio();
/* close_audio():
 * returns the audio to the system control, doing necessary
 * cleanup
 */
XT void close_audio();
/* set_mix(percent): set mix channels level.
 * 0: spatial stereo. 100: mono.
 */
XT void set_mix();
/* output_samples(l, r): outputs a pair of stereo samples.
 * Samples are 15 bits signed.
 */
XT void output_samples();
/* flush_buffer(): call from time to time, because buffering
 * is done by the program to get better (?) performance.
 */
XT void flush_buffer();
/* discard_buffer(): try to get rid of the buffer contents
 */
XT void discard_buffer();
/* new_freq = update_frequency():
 * if !0, frequency changed and playing should be updated accordingly
 */
XT int update_frequency();
/* set_synchro(bool):
 * try to synchronize audio output by using a smaller buffer
 */
XT void set_synchro();







/* termio.c */
/* nonblocking_io():
 * try to setup the keyboard to non blocking io
 */
XT void nonblocking_io();
XT void sane_tty();
/* may_getchar():
 * return EOF, or a char if available
 */
XT int may_getchar();
/* run_in_fg():
 * try to decide whether or not we're currently running in the foreground.
 */
XT BOOL run_in_fg();







/* tools.c */
/* v = read_env(name, default):
 * read a scalar value in the environment
 */
XT int read_env();

#ifdef AMIGA
XT FILE *popen(char *command, char *mode);
XT void pclose(FILE *f);
#else
#define alloc_sample(len)		calloc(len, 1)
#define free_sample(sample)		free(sample)
#endif


XT void reset_channels();
XT struct audio_channel *new_channel();
