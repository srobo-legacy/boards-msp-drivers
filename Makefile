ifeq ($(origin CC),default)
CC := msp430-gcc
endif

# Conditional compilation depending on ARCH will occur here
# at some point in the future :-P

all: libdrivers.a

libdrivers.a: usart.o
	msp430-ar r $@ $^

usart.o: usart.h

.PHONY: clean

clean:
	-rm -f *.o
