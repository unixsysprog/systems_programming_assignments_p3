# 
#	Christopher Fuentes
#	cfuentes@g.harvard.edu
#	HUID 60857326
#
# 	Makefile for Project 3 (sttyl)
# 	Builds sttyl, a simplified (lite) version of stty
#
#	Formulas:
#	make 		=> builds sttyl
#

sttyl: sttyl.c stty_utils.o
	$(CC) -Wall sttyl.c stty_utils.o -o sttyl

stty_utils.o: stty_utils.c
	$(CC) -Wall -c stty_utils.c

clean:
	rm -f *.o sttyl
	