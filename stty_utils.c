/*
	stty_utils.c 
	by Christopher Fuentes
	HUID 60857326
	cfuentes@g.harvard.edu
*/

#include "stty_utils.h"
#include <stdlib.h>
#include <string.h>

/* search a given flag array for a named flag */
bool search_flags(char *find, flaginfo *flagp, flaginfo *flags);

flaginfo control_flags[] = {
	{ HUPCL,  	"hupcl", 	'c'  }, 
	{ 0, 		NULL, 		'\0' }
};

flaginfo input_flags[] = {
	{ ICRNL	,	"icrnl", 	'i'  }, 
	{ 0	,	NULL, 		'\0' } 
};

flaginfo output_flags[] = {
	{ OPOST, 	"opost", 	'o' }, 
	{ 0,		NULL, 		'\0' }
};

flaginfo local_flags[] = {
	{ ECHO	,	"echo", 	'l'  },
	{ ECHOE	,	"echoe", 	'l'  },
	{ ICANON,	"icanon",	'l'  },
	{ ISIG	,	"isig",		'l'  },
	{0	,	NULL, 		'\0' } 
};

ccinfo special_chars[] = {
	{ VERASE, 	"erase", 	},
	{ VINTR,	"intr", 	},
	{ VKILL,	"kill",		},
	{ 0, 		NULL		}
};

/* 	
* 	According to the man page for termios, only the 
* 	constants listed below are possible baud rates. 
*/
baud_val baud_vals[] = {
	{ B0,		"0" 		},		
	{ B50,		"50" 		},		
	{ B110,		"110" 		},		
	{ B134,		"134.5"		},		
	{ B150,		"150" 		},		
	{ B200,		"200" 		},		
	{ B300,		"300" 		},		
	{ B600,		"600" 		}, 		
	{ B1200,	"1200" 		}, 	
	{ B1800,	"1800" 		}, 	
	{ B2400,	"2400" 		}, 	
	{ B4800,	"4800" 		}, 	
	{ B9600,	"9600" 		}, 	
	{ B19200,	"19200" 	}, 	
	{ B38400,	"38400" 	}, 	
	{ B57600,	"57600" 	}, 	
	{ B115200,	"115200" 	}, 	
	{ B230400, 	"230400" 	},
	{ 0,		NULL 		}
};

bool find_special_char_flag(char *find, ccinfo *infop) {
	int 	i = 0;
	ccinfo 	charinfo;
	while ( (charinfo = special_chars[i++] ).desc != NULL) {
		if (strcmp(charinfo.desc, find) == 0) {
			*infop = charinfo;
			return true;
		}
	}
	return false;
}

/*
* 	Search a flag array for a named flag and, if found,
*	set *flagp to that flag. Returns true if found, false otherwise.
*
*	@param find 	name of flag to find
*	@param flagp 	pointer to flag buffer
*	@param flags 	array of flaginfo (control, input, output, local)
*
*	@return 	true if found, false otherwise.
*/
bool search_flags(char *find, flaginfo *flagp, flaginfo *flags) {
	int 		cur;
	flaginfo 	info;

	if (find[0] == '-') {
		int len 	= strlen(find);
		char buf[len + 1];
		memcpy( buf, &(find[1]), len);
		buf[len] 	= '\0';
		find 		= buf;
	}

	cur = 0;
	while ( ( info = flags[cur++] ).fl_name != NULL ) {
		if ( strcmp(find, info.fl_name) == 0 ) {
			*flagp = info;
			return true;
		}
	}
	return false;
}

/*
* 	find_option_flag searches for a named flag using
*	the search_flags helper function to look through
*	each of the input, output, local, and control arrays.
*
*	@param find 	name of flag to find
*	@param flagp 	pointer to flaginfo buffer in which to store result
*	
*	@return 	true if found, false otherwise
*/
bool find_option_flag(char *find, flaginfo *flagp) {
	return (search_flags(find, flagp, input_flags) 		||
		search_flags(find, flagp, output_flags) 	||
		search_flags(find, flagp, control_flags) 	||
		search_flags(find, flagp, local_flags));
}

