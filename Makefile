server: main.c server.c interface.c queryparams.c multiplex.c
	gcc -g main.c server.c interface.c queryparams.c multiplex.c -o server
