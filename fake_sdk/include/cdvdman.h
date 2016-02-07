#pragma once

#include <time.h>

typedef struct {
	u8 stat;  			
	u8 second; 			
	u8 minute; 			
	u8 hour; 			
	u8 week; 			
	u8 day; 			
	u8 month; 			
	u8 year; 			
} cd_clock_t;

#define itob(i)   ((i)/10*16 + (i)%10)  // int to BCD
#define btoi(b) ((b)/16*10 + (b)%16)

#define sceCdRI(x, y) 1
#define sceCdCLOCK cd_clock_t

static inline int
sceCdReadClock (cd_clock_t *rtc)
{
  time_t now = time (NULL);
  struct tm *tm = localtime (&now);

  /* it seems that time is encoded in bcd; day in [1, 31], mon in [1, 12],
   * year is relative against 2000 */
  rtc->stat = 0;
  rtc->second = itob (tm->tm_sec);
  rtc->minute = itob (tm->tm_min);
  rtc->hour = itob (tm->tm_hour >= 4 ? tm->tm_hour - 4 : 20 + tm->tm_hour);
  rtc->week = 0;
  rtc->day = itob (tm->tm_mday);
  rtc->month = itob (tm->tm_mon + 1);
  rtc->year = itob (tm->tm_year - 100);
  return (1);
}

static inline int
CdReadIlinkID (u8 *id/*[32?]*/, int *err)
{
  memcpy (id, "01234567", 8);
  if (err) *err = 0;
  return (1);
}
