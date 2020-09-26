/*
 * General purpose macros and c library includes
 */
#ifndef GLOBAL_INCLUDES_H

#include<stdio.h>
#include<assert.h>
#include<stdlib.h>
#include<stdint.h>
#include<limits.h>
#include<string.h>

static_assert(CHAR_BIT == 8, "Char must be 8 bits");

#define SIZE_OF_ARRAY(arr) (sizeof(arr) / sizeof((arr)[0]))

#define BITS_PER_BYTE 8

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;

// Technically correct...the best kind of correct!
// Except when it comes to naming things
#define KIBIBYTES(X) (((u64)X) * 1024)
#define MEBIBYTES(X) (KIBIBYTES((u64)X) * 1024)
#define GIBIBYTES(X) (MEBIBYTES((u64)X) * 1024)
#define TEBIBYTES(X) (GIBIBYTES((u64)X) * 1024)


// Debug stuff

#ifdef STDOUT_DEBUG

#define DEBUG_PRINTF(...) fprintf(stderr, __VA_ARGS__)

#define DEBUG_ASSERT(E) assert(E)

#define FATAL_PRINTF(...)               \
    do                                  \
    {                                   \
        fprintf(stderr, __VA_ARGS__);   \
        exit(1);                        \
    } while(false)

#else

// TODO figure out how to minimize these
#define DEBUG_PRINTF(...)
#define DEBUG_ASSERT(E)
#define FATAL_PRINTF(...)

#endif


#define GLOBAL_INCLUDES_H
#endif