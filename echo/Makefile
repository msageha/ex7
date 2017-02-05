TARGETS=echo_client echo_server echo_server_pthread
DEFS=-D_POSIX_C_SOURCE=200809L -D_ISOC99_SOURCE -D_XOPEN_SOURCE=500
CFLAGS=-Wall $(DEFS) -std=c99 # -g

all: $(TARGETS)

echo_client: echo_client.c
	$(CC) $(CFLAGS) -o $@ $< 
echo_server: echo_server.c
	$(CC) $(CFLAGS) -o $@ $<
echo_server_pthread: CFLAGS += -lpthread 
echo_server_pthread: echo_server_pthread.c
	$(CC) $(CFLAGS) -o $@ $<		

clean:
	rm -f  $(TARGETS)
	rm -rf *.dSYM
	rm -f  *.o
