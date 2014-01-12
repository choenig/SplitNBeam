#ifndef CUSTOMTIME_H_
#define CUSTOMTIME_H_

// Usage:
//
// Inside your code define CUSTOM_TIME and include customtime.h:
//     #define CUSTOM_TIME "23:59:10"
//     #include "customtime.h"
//
// Then call ADJUST_TIME_TO_CUSTOM_TIME(t) everywhere you use 'struct tm'

#ifdef CUSTOM_TIME

// timeString must have the format "HH:MM:SS
static struct tm parseHHMMSS(const struct tm * baseT, const char * timeString)
{
    struct tm retval = *baseT;

    char tmp[] = "00";
    strncpy(tmp, timeString, sizeof(tmp));
    retval.tm_hour = atoi(tmp);

    strncpy(tmp, timeString+3, sizeof(tmp));
    retval.tm_min  = atoi(tmp);

    strncpy(tmp, timeString+6, sizeof(tmp));
    retval.tm_sec  = atoi(tmp);

    return retval;
}

static void adjustTime(struct tm * t, const char * timeString)
{
    static int offset = 0;
    if (offset == 0) {
        struct tm x = parseHHMMSS(t, timeString);
        offset = mktime(&x) - mktime(t);
    }

    t->tm_sec += offset;
    mktime(t);


    char txt[] = "00:00:00";
    strftime(txt, sizeof(txt), clock_is_24h_style() ? "%H:%M:%S" : "%I%M", t);
    app_log(APP_LOG_LEVEL_DEBUG, "", 0, "%s", txt);
}

#define ADJUST_TIME_TO_CUSTOM_TIME(t) adjustTime(t, CUSTOM_TIME);
#else
#define ADJUST_TIME_TO_CUSTOM_TIME(t)
#endif

#endif // CUSTOMTIME_H_
