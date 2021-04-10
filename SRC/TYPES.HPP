#ifndef TYPES_H
#define TYPES_H

#include <stddef.h>

#define TRUE  ((bool_t)1)
#define FALSE ((bool_t)0)

/** Boolean. */
typedef unsigned char bool_t;

/** Signed 8-bit integer. */
typedef char int8_t;
/** Signed 16-bit integer. */
typedef short int16_t;
/** Signed 32-bit integer. */
typedef long int32_t;

/** Unsigned 8-bit integer. */
typedef unsigned char uint8_t;
/** Unsigned 16-bit integer. */
typedef unsigned short uint16_t;
/** Unsigned 32-bit integer. */
typedef unsigned long uint32_t;

#endif