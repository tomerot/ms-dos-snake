#ifndef COMMON_H
#define COMMON_H

/* C89 has no bool type, so it is defined here. */
#define FALSE 0
#define TRUE  1
typedef unsigned char bool;

/* An 8-bit value that is neither a character nor a boolean. */
typedef unsigned char byte;

/* A position: x counts across from the left, y down from the top, both
   from 0. */
typedef struct {
    int y;
    int x;
} pos;

#endif
