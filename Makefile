# Makefile for compiling and running serverM, serverS, serverD, serverU, and client

CC = gcc
CFLAGS = -Wall -Wextra -Werror

# Targets
all: serverM serverS serverD serverU client

# Compile serverM
serverM: serverM.c
	$(CC) $(CFLAGS) serverM.c -o serverM

# Compile serverS
serverS: serverS.c
	$(CC) $(CFLAGS) serverS.c -o serverS

# Compile serverD
serverD: serverD.c
	$(CC) $(CFLAGS) serverD.c -o serverD

# Compile serverU
serverU: serverU.c
	$(CC) $(CFLAGS) serverU.c -o serverU

# Compile client
client: client.c
	$(CC) $(CFLAGS) client.c -o client

# Clean up executable files
clean:
	rm -f serverM serverS serverD serverU client