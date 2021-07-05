/*
	sttyl.c 
	by Christopher Fuentes
	HUID 60857326
	cfuentes@g.harvard.edu
*/

#include 	"stty_utils.h"
#include 	<sys/ioctl.h>
#include	<termios.h>
#include 	<stdlib.h>
#include 	<string.h>
#include 	<errno.h>
#include 	<ctype.h>
#include	<stdio.h>

#define INVALID_CHAR_FMT 	"sttyl: invalid integer argument `%s'\n"
#define INVALID_ARG_FMT 	"sttyl: invalid argument `%s'\n"
#define MISSING_ARG_FMT 	"sttyl: missing argument to `%s'\n"

#define ENV_VAR_COLUMNS 	"COLUMNS"
#define ENV_VAR_LINES 		"LINES"

#define ASCII_DELETE		127

/* wrapper function to print all tty info */
void show_stty_info(termios ttyinfo);

/* prints input, output, control, and local flag sets from a struct termios */
void print_tty_flagsets(struct termios *ttyp);

/* prints a single flagset via bit masking an int val */
void print_flagset(int val, struct flaginfo flags[]);

/* prints the baudspeed */
void showbaud(speed_t thespeed);

/* prints intr, erase, and kill chars */
void show_special_chars(termios ttyinfo);

/* prints nrows and ncols */
void show_dimensions(void);

/* get tty info from kernal */
void get_tty_info(termios *ttyp);

/* (un)set a given flag from a struct termios */
void set_flag(bool set, flaginfo finfo, termios *ttyp);

/* attempt to parse a special char flag (which requires an argument) */
void handle_cc(int ac, char **av, int cur, termios *ttyp, ccinfo charinfo);

/* attempt to parse the next command line arg */
int handle_next_arg(int ac, char **av, int cur_arg, termios *ttyp);

/* saves current tty info via tcsetattr() */
void set_tty_info(termios *ttyp);

///////////////////////////////////////////////////////////////////////////////

int main(int ac, char **av) {
	int 	cur_arg = 1;
	termios tty;

	get_tty_info(&tty);

	if (ac == 1)  	show_stty_info(tty);
	else  {
		while( (cur_arg = handle_next_arg( ac, av, cur_arg, &tty )) );
		set_tty_info(&tty);
	}
	return 0;
}

/*
* 	handle_cc attempts to parse an argument
*	for a flag indicating a special character, then apply that 
*	special character to the termios *'s c_cc array.
*
*	@param ac 		arg count from main()
*	@param av 		arg vector from main()
* 	@param cur 		index of current argument to parse
*	@param ttyp 		pointer to initialized termios bufer
*	@param charinfo 	struct containing char flag's description
*				as well as the constant representing the 
*				index into the ttyp's c_cc array for that flag
*/
void handle_cc(int ac, char **av, int cur, termios *ttyp, ccinfo charinfo) {
	char flagval;

	if (cur + 1 >= ac) 
	{
		fprintf(stderr, MISSING_ARG_FMT, av[cur]);
		exit(1);
	} 
	else if (strlen(av[++cur]) > 1)  
	{
		fprintf(stderr, INVALID_CHAR_FMT, av[cur]);
		exit(1);
	} 
	else 
	{
		flagval = av[cur][0];
		ttyp->c_cc[charinfo.ccindex] = flagval; /* set char in tty */
	}
}

/*
* 	set_flag will set or unset the setting specified by
*	finfo in the termios *tp. 
*
*	@param set 	whether to set or unset the flag
*	@param finfo 	flaginfo object about the flag to be (un)set,
*			including the bitmask.
*	@param tp 	pointer to the termios to be manipulated
*/
void set_flag(bool set, flaginfo finfo, termios *tp) {
	int 	mask = set ? finfo.fl_value : ~finfo.fl_value;

	switch(finfo.type) {
		case 'i':  	/* input */
			tp->c_iflag = set ? 
				tp->c_iflag | mask : 
				tp->c_iflag & mask;
			break;
		case 'o':	/* output */
			tp->c_oflag = set ? 
				tp->c_oflag | mask : 
				tp->c_oflag & mask;
			break;	
		case 'l':	/* local */
			tp->c_lflag = set ? 
				tp->c_lflag | mask : 
				tp->c_lflag & mask;
			break;
		case 'c':	/* control */
			tp->c_cflag = set ? 
				tp->c_cflag | mask : 
				tp->c_cflag & mask;
			break;
		default:
			fprintf(stderr, "Invalid flag type %c\n", finfo.type);
			exit(1);
	}
}

/*
* 	handle_next_arg attempts to parse the next arg and 
*	resolve it as a flag. Encountering an invalid flag
*	or erroneous/missing argument for a flag will
*	print a message and exit. 
*
*	@param ac 	arg count from main()
*	@param av 	arg vector from main()
*	@param cur_arg 	index of the next arg to parse
*	@param ttyp 	pointer to an initialized struct termios,
*			to which we will write new values
*
*	@return 	Index of next arg to parse, or 0 when
*			cur_arg == ac. Exits program for bad args.
*/
int handle_next_arg(int ac, char **av, int cur_arg, termios *ttyp) {
	flaginfo 	flag;
	ccinfo 		charinfo;

	char 		*flagstr = av[cur_arg];

	if (find_special_char_flag(flagstr, &charinfo)) 
		handle_cc(ac, av, cur_arg++, ttyp, charinfo);
	else if (find_option_flag(flagstr, &flag))
		set_flag(flagstr[0] != '-', flag, ttyp);
	else 
	{
		fprintf(stderr, INVALID_ARG_FMT, flagstr);
		exit(1);
	}

	cur_arg++;

	if (cur_arg == ac) 	return 0;
	else 			return cur_arg;
}


void set_tty_info(termios *ttyp) {
	if ( tcsetattr( 0, TCSANOW, ttyp ) == -1 ){		
		perror("Unable to set stty info");
		exit(1);
	}
}

/*
	get_tty_info accepts a pointer to an empty struct
	termios and fills it via tcgetattr

	@param ttyinfo		Pointer to empty struct termios
*/
void get_tty_info(termios *ttyp) {
	if ( tcgetattr( 0, ttyp ) == -1 ){		
		perror("Unable to get stty info");
		exit(1);
	}
}

/* 
* 	show_stty_info prints out a basic subset of stty info of the form:
*
*	speed <int> baud; rows <int>; columns <int>;
*	intr = <char>; erase = <char>; kill = <char>;
*	[-]icrnl
*	[-]opost, 
*	[-]icanon [-]isig [-]echo [-]echoe 
*	[-]hupcl 
*	
*
*	Function largely adapted from main() in showtty.c by Bruce Molay
*
*	@param ttyinfo 	A struct termios that has been initialized with 
*			current parameters.
*/
void show_stty_info(termios ttyinfo){
	showbaud ( cfgetospeed( &ttyinfo ) );	/* Show baud rate */
	show_dimensions();			/* Show nrows and ncols */
	show_special_chars(ttyinfo);		/* Show special characters */
	print_tty_flagsets( &ttyinfo );	/* Show misc. flags	*/
}

/*
*	show_special_chars prints the intr, kill, and erase
*	chars from a struct termios. 
*
*	@param ttyinfo	An initialized struct termios (via tcgetattr )
*/
void show_special_chars(termios ttyinfo) {
	int 	i = 0;
	char 	schar;
	ccinfo 	charinfo;

	while ( ( charinfo = special_chars[i++] ).desc != NULL ) {
		printf("%s = ", charinfo.desc);
		schar = ttyinfo.c_cc[charinfo.ccindex];
		if (schar == ASCII_DELETE) 
			printf("^? ");
		else if (isprint(schar)) 
			printf("%c ", schar);
		else
			printf("^%c ", schar - 1 + 'A');
	}
	printf("\n");
}

/*
*	Show the number of rows and columns in the current 
*	terminal params.
*
*	NOTE: Apparently there's no 'good' way to do this for tis
*	assignment, as per https://piazza.com/class/i5fnskarx8i1nj?cid=73
*	Therefore, the function attempts to use TIOCGWINSZ if it is defined, 
*	else tries to use the LINES and COLUMNS environment variables.
*	If parsing either environment variable is problematic, we just
*	don't print the rows and cols. 
*
*/
void show_dimensions() {
#ifdef TIOCGWINSZ
	struct winsize ws;
	if (ioctl(0, TIOCGWINSZ, &ws) >= 0) 
		printf("rows %u; columns %u\n", ws.ws_row, ws.ws_col);
	else 
	{  /* closing brace appears in second #ifdef */
#else 
		char 	*rows = getenv(ENV_VAR_LINES), 
			*cols = getenv(ENV_VAR_COLUMNS);
		long int r, c;
		int 	rerr, cerr;

		errno = 0;	/* reset so we know if the call fails */
		if (rows != NULL && cols != NULL) {
			r = strtol(rows, NULL, 10);
			rerr = errno;
			c = strtol(cols, NULL, 10); 
			cerr = errno;
			if (!(rerr || cerr) 
				printf("rows %d; columns %d\n", r, c);
		}
#endif
#ifdef TIOCGWINSZ
	} /* Closing brace for 'else' */
#endif
}

/*
* 	Matches ttyinfo's baudspeed flag with a constant and prints out
*	the corresponding speed. Since baud_vals contains all valid 
*	values for baudspeed (according to the man pages), if no match 
*	is found then the baudspeed is erroneous and no constant is defined
*	for it.
*
*	@param speed 	the baudspeed returned from cfgetospeed()
*/
void showbaud(speed_t speed) {
	int i;

	for (i=0; baud_vals[i].desc != NULL; i++){
		if (baud_vals[i].speed == speed) {
			printf("speed %s baud; ", baud_vals[i].desc);
			return;
		}
	}
	printf("baudspeed not defined; ");
} 

/*
 *	print_tty_flagsets attempts to print out each
 *	set of flags from a termios *'s input, output, control
 *	and local flags. 
 *
 *	@param 	ttyp 	pointer to a struct termios
 */

void print_tty_flagsets( struct termios *ttyp ) {
	print_flagset( ttyp->c_iflag, input_flags );
	print_flagset( ttyp->c_oflag, output_flags );
	print_flagset( ttyp->c_lflag, local_flags );
	print_flagset( ttyp->c_cflag, control_flags );
}

/*
*	print_flagset accepts an int value containing bits
*	which specify settings for a given flag type (input, output, 
*	control or local) and a table that maps bitmasks to strings
*	and prints out a description of which flags are on and off.
*	Flags that are off are preceded with a -
*
*	@param 	value 	An integer containing the bit pattern for a 
*			flag type {input|output|contro|local} from a 
*			struct termios
*
*	@param flags 	A table mapping bitmasks to string descriptions
*			of the masks. 
*/
void print_flagset( int value, struct flaginfo flags[] ) {
	int	i;
	
	for ( i=0; flags[i].fl_value != 0 ; i++ ) {
		if ( ! (value & flags[i].fl_value) )
			printf("-");	//negation
		printf("%s ", flags[i].fl_name);
	}
	printf("\n");
}
