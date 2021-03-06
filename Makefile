CC = gcc
CFLAGS = -c -g -Wall -Wextra
LFLAGS = -g -Wall -Wextra

INPUTFILESIZEMEGABYTES = 1

KILO = 1024
MEGA = $(shell echo $(KILO)\*$(KILO) | bc)
INPUTFILESIZEBYTES = $(shell echo $(MEGA)\*$(INPUTFILESIZEMEGABYTES) | bc)
INPUTBLOCKSIZEBYTES = $(KILO)
INPUTBLOCKS = $(shell echo $(INPUTFILESIZEBYTES)\/$(INPUTBLOCKSIZEBYTES) | bc)

.PHONY: all clean

all: pi pi-sched rw-sched mix-sched rr_quantum

pi: pi.o
	$(CC) $(LFLAGS) $^ -o $@ -lm

pi-sched: pi-sched.o
	$(CC) $(LFLAGS) $^ -o $@ -lm

rw-sched: rw-sched.o rwinput
	$(CC) $(LFLAGS) rw-sched.o -o $@ -lm

mix-sched: mix-sched.o 
	$(CC) $(LFLAGS) mix-sched.o -o $@ -lm

rr_quantum: rr_quantum.o
	$(CC) $(LFLAGS) $^ -o $@ -lm

pi.o: pi.c
	$(CC) $(CFLAGS) $<

pi-sched.o: pi-sched.c
	$(CC) $(CFLAGS) $<

rw-sched.o: rw-sched.c
	$(CC) $(CFLAGS) $<

mix-sched.o: mix-sched.c
	$(CC) $(CFLAGS) $<
	
rwinput: Makefile
	dd if=/dev/urandom of=./rwinput bs=$(INPUTBLOCKSIZEBYTES) count=$(INPUTBLOCKS)

rr_quantum.o: rr_quantum.c
	$(CC) $(CFLAGS) $<

clean: testclean
	rm -f pi pi-sched rw-sched mix-sched rr_quantum
	rm -f rwinput
	rm -f *.o
	rm -f *~
	rm -f handout/*~
	rm -f handout/*.log
	rm -f handout/*.aux

testclean:
	rm -f rwoutput*
	rm -f mixoutput*
