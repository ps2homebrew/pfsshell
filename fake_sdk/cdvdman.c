#include "types.h"
#include "cdvdman.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>


/* DONE[!] */
#define TO_BCD(n) ((((n) / 10) << 4) | (n))

static int
to_bcd (int n)
{
  int hi = n / 10;
  int lo = n - hi * 10;
  return ((hi << 4) | lo);
}

int
sceCdReadClock (cd_clock_t *rtc)
{
  time_t now = time (NULL);
  struct tm *tm = localtime (&now);

  /* it seems that time is encoded in bcd; day in [1, 31], mon in [1, 12],
   * year is relative against 2000 */
  rtc->stat = 0;
  rtc->second = to_bcd (tm->tm_sec);
  rtc->minute = to_bcd (tm->tm_min);
  rtc->hour = to_bcd (tm->tm_hour >= 4 ? tm->tm_hour - 4 : 20 + tm->tm_hour);
  rtc->week = 0;
  rtc->day = to_bcd (tm->tm_mday);
  rtc->month = to_bcd (tm->tm_mon + 1);
  rtc->year = to_bcd (tm->tm_year - 100);
  return (1);
}


int
CdReadIlinkID (u8 *id/*[32?]*/, int *err)
{
  memcpy (id, "01234567", 8);
  if (err) *err = 0;
  return (1);
}
