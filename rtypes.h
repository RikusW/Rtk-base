
#ifndef RTYPES_H
#define RTYPES_H

#ifdef _WIN32

typedef   signed __int8  s8;
typedef unsigned __int8  u8;
typedef   signed __int16 s16;
typedef unsigned __int16 u16;
typedef   signed __int32 s32;
typedef unsigned __int32 u32;
typedef   signed __int64 s64;
typedef unsigned __int64 u64;

#ifdef _WIN64
typedef   signed __int64 sptr;
typedef unsigned __int64 uptr;
#else
typedef   signed __int32 sptr;
typedef unsigned __int32 uptr;
#endif

typedef unsigned int uint; //to make gcc+printf happy...

#else
#include <stdint.h>

typedef  int8_t  s8;
typedef uint8_t  u8;
typedef  int16_t s16;
typedef uint16_t u16;
typedef  int32_t s32;
typedef uint32_t u32;
typedef  int64_t s64;
typedef uint64_t u64;

typedef  intptr_t sptr;
typedef uintptr_t uptr;

typedef unsigned int uint; //to make gcc+printf happy...

#endif

/* the old way
typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed long s32;
typedef unsigned long u32;

typedef signed long long s64;
typedef unsigned long long u64;

#if __WORDSIZE == 64
typedef s64 sptr;
typedef u64 uptr;
#else
typedef s32 sptr;
typedef s32 uptr;
#endif

typedef unsigned int uint; //to make gcc+printf happy...

*/

#endif //RTYPES_H

