ifeq ($(origin CC),default)
CC := msp430-gcc
endif

# Conditional compilation depending on ARCH will occur here
# at some point in the future :-P

all: libdrivers.a

libdrivers.a: usart.o usci.o xt2.o sched.o pinint.o
	msp430-ar r $@ $^

usart.o: usart.h
usci.o: usci.h
xt2.o: xt2.h
sched.o: sched.h

.PHONY: clean

clean:
	-rm -f *.o
