#ifndef __STACK_H
#define __STACK_H

#include <stdint.h>

extern uint8_t _end;
extern uint8_t __stack;

/* The space available for the stack, in bytes */
#define stack_size (uint16_t)(&__stack - &_end + 1)

/* This function is run before initialisation of the stack pointer and aids in
 * measuring stack usage by filling it with a known value. */
void stack_clear(void) __attribute__ ((naked))
                       __attribute__ ((section (".init1")));

/* Returns the amount of stack space used, in bytes */
uint16_t stack_max_usage(void);

#endif /* __STACK_H */
