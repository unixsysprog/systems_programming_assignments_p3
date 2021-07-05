/*
	stty_utils.h
	by Christopher Fuentes
	HUID 60857326
	cfuentes@g.harvard.edu
*/

#include	<termios.h>
#include 	<stdbool.h>

#ifndef STTY_UTILS_H
#define STTY_UTILS_H

typedef struct termios termios;

/* flaginfo taken from showtty.c */
typedef struct flaginfo { 
	tcflag_t fl_value; 
	char *fl_name; 
	char type;  // { i | o | c | l }
} 	flaginfo; 
typedef struct ccinfo 	{ int ccindex; char *desc; } 		ccinfo;
typedef struct baud_val { speed_t speed; char *desc; } 		baud_val;

/* Sets *flagp to a flag with fl_name == find or NULL if not found */
bool find_option_flag(char *find, flaginfo *flagp);
bool find_special_char_flag(char *find, ccinfo *infop);

/* Arrays of flaginfo */
extern flaginfo input_flags[];
extern flaginfo control_flags[];
extern flaginfo output_flags[];
extern flaginfo local_flags[];

/* Array of ccinfo for special chars */
extern ccinfo special_chars[];

/* Array of valid baud vals */
extern baud_val baud_vals[];

#endif //STTY_UTILS_H