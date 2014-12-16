/*      
# _____     ___ ____     ___ ____
#  ____|   |    ____|   |        | |____|
# |     ___|   |____ ___|    ____| |    \    PS2DEV Open Source Project.
#-----------------------------------------------------------------------
# Copyright 2001-2004, ps2dev - http://www.ps2dev.org
# Licenced under Academic Free License version 2.0
# Review ps2sdk README & LICENSE files for further details.
#
# $Id: tamtypes.h 731 2005-01-03 03:21:49Z pixel $
# Common used typedef
*/

#ifndef _TAMTYPES_H_
#define _TAMTYPES_H_ 1

#include <stdint.h>

typedef	uint8_t 		u8;
typedef uint16_t 		u16;

typedef	volatile uint8_t 		vu8;
typedef volatile uint16_t 	vu16;

// #ifdef _EE
// typedef uint32_t		u32;
// typedef uint64_t	u64;
// typedef unsigned int		u128 __attribute__(( mode(TI) ));

// typedef volatile uint32_t		vu32;
// typedef volatile uint64_t	vu64;
// typedef volatile unsigned int		vu128 __attribute__(( mode(TI) ));
// #else
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef volatile uint32_t	vu32;
typedef volatile uint64_t	vu64;
// #endif

typedef int8_t 		s8;
typedef int16_t 		s16;

typedef volatile int8_t	vs8;
typedef volatile int16_t	vs16;

// #ifdef _EE
// typedef signed int		s32;
// typedef signed long int		s64;
// typedef signed int		s128 __attribute__(( mode(TI) ));

// typedef volatile signed int		vs32;
// typedef volatile signed long int	vs64;
// typedef volatile signed int		vs128 __attribute__(( mode(TI) ));
// #else
typedef int32_t		s32;
typedef int64_t	s64;

typedef volatile int32_t	vs32;
typedef volatile int64_t	vs64;
// #endif

#ifndef NULL
#define NULL	(void *)0
#endif

static inline u8  _lb(u32 addr) { return *(vu8 *)addr; }
static inline u16 _lh(u32 addr) { return *(vu16 *)addr; }
static inline u32 _lw(u32 addr) { return *(vu32 *)addr; }
static inline u64 _ld(u32 addr) { return *(vu64 *)addr; }

static inline void _sb(u8 val, u32 addr) { *(vu8 *)addr = val; }
static inline void _sh(u16 val, u32 addr) { *(vu16 *)addr = val; }
static inline void _sw(u32 val, u32 addr) { *(vu32 *)addr = val; }
static inline void _sd(u64 val, u32 addr) { *(vu64 *)addr = val; }

// #ifdef _EE
// static inline u128 _lq(u32 addr) { return *(vu128 *)addr; }
// static inline void _sq(u128 val, u32 addr) { *(vu128 *)addr = val; }
// #endif

#endif
