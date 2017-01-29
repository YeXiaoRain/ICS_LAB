TEAM = NOBODY
VERSION = 1

CC = gcc
CFLAGS = -Wall -g 
LDFLAGS = -lpthread

OBJS = proxy.o csapp.o

all: proxy

proxy: $(OBJS)

csapp.o: csapp.c
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: proxy.c
	$(CC) $(CFLAGS) -c proxy.c

clean:
	rm -r -f *~ *.o proxy core proxy.log proxy_run.log test

