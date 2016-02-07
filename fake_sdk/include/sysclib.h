
#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

typedef	uint8_t u8;
typedef uint16_t u16;

typedef	volatile uint8_t vu8;
typedef volatile uint16_t vu16;

typedef uint32_t u32;
typedef uint64_t u64;

typedef volatile uint32_t vu32;
typedef volatile uint64_t vu64;

typedef int8_t s8;
typedef int16_t s16;

typedef volatile int8_t vs8;
typedef volatile int16_t vs16;

typedef int32_t s32;
typedef int64_t s64;

typedef volatile int32_t vs32;
typedef volatile int64_t vs64;

static inline u8  _lb(u32 addr) { return *(vu8 *)addr; }
static inline u16 _lh(u32 addr) { return *(vu16 *)addr; }
static inline u32 _lw(u32 addr) { return *(vu32 *)addr; }
static inline u64 _ld(u32 addr) { return *(vu64 *)addr; }

static inline void _sb(u8 val, u32 addr) { *(vu8 *)addr = val; }
static inline void _sh(u16 val, u32 addr) { *(vu16 *)addr = val; }
static inline void _sw(u32 val, u32 addr) { *(vu32 *)addr = val; }
static inline void _sd(u64 val, u32 addr) { *(vu64 *)addr = val; }

//somebody needs to verify what this does
static inline unsigned char look_ctype_table (char character)
{
  if (isdigit (character))
    return (0x04);
  else if (isalpha (character))
    return (0x02);
  else
    return (0);
}
