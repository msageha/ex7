TARGETS=target echo_server
OBJS = color.o board.o player.o game.o 

DEFS = -D_POSIX_C_SOURCE=200809L -D_ISOC99_SOURCE -D_XOPEN_SOURCE=500
OPTS = -std=c99 -Wall -g $(DEFS)
CFLAGS=-Wall $(DEFS) -std=c99 # -g

.PHONY: doc

CC = gcc $(OPTS) 
INCLUDE = -I ./

TARGET_SINGLE  = single_play

all: $(TARGETS)

target: $(TARGET_SINGLE)

$(TARGET_SINGLE): single_play.c $(OBJS)
	$(CC) $(INCLUDE) -o $(TARGET_SINGLE) $(OBJS) $<

%.o:%.c %.h
	$(CC) $(INCLUDE) -o $@ -c $<

doc:
	doxygen doxygen.conf

echo_server: echo_server.c $(OBJS)
	$(CC) -o $@ $(OBJS) $<

clean:
	rm -f  $(TARGET_SINGLE)
	rm -rf *.dSYM
	rm -f $(OBJS)
	rm -rf ./doc
