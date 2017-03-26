#pragma once

#include <stdlib.h>
#include <time.h>

typedef struct
{
    u8 stat;
    u8 second;
    u8 minute;
    u8 hour;
    u8 pad;
    u8 day;
    u8 month;
    u8 year;
} sceCdCLOCK;

#define itob(i) ((i) / 10 * 16 + (i) % 10) // int to BCD
#define btoi(b) ((b) / 16 * 10 + (b) % 16) // BCD to int

#define sceCdRI(x, y) 1

static inline int sceCdReadClock(sceCdCLOCK *clock)
{
#ifndef USE_LOCAL_TZ
    char *orig_tz = getenv("TZ");
    setenv("TZ", "Japan", 1);
    tzset();
    if (orig_tz) {
        setenv("TZ", orig_tz, 1);
    } else {
        unsetenv("TZ");
    }
#endif
    time_t now = time(NULL);
    struct tm *tm = localtime(&now);

    /* it seems that time is encoded in bcd; day in [1, 31], mon in [1, 12],
   * year is relative against 2000 */
    clock->stat = 0;
    clock->second = itob(tm->tm_sec);
    clock->minute = itob(tm->tm_min);
    clock->hour = itob(tm->tm_hour >= 4 ? tm->tm_hour - 4 : 20 + tm->tm_hour);
    clock->day = itob(tm->tm_mday);
    clock->month = itob(tm->tm_mon + 1);
    clock->year = itob(tm->tm_year - 100);
#ifndef USE_LOCAL_TZ
    tzset();
#endif

    return (1);
}
