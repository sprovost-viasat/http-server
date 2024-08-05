# Define the source directory
SRCDIR = src

# server: main.c server.c interface.c queryparams.c multiplex.c
# 	gcc -g main.c server.c interface.c queryparams.c multiplex.c -o server

# Find all .c files in the src directory
SRCS = $(wildcard $(SRCDIR)/*.c)

# Build the executable
server: $(SRCS)
	gcc -g $(SRCS) -o server
