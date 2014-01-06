#include "timevals.h"

#define US_PER_S (1000 * 1000)
#define US_PER_MS (1000)

void
tv_plus_us(struct timeval *tv, unsigned us)
{
    unsigned seconds = us / US_PER_S;
    tv->tv_sec += seconds;
    tv->tv_usec += us % US_PER_S;
    if (tv->tv_usec > US_PER_S) {
        tv->tv_sec ++;
        tv->tv_usec -= (US_PER_S);
    }
}

void
tv_plus_ms(struct timeval *tv, unsigned ms)
{
    tv_plus_us(tv, ms * US_PER_MS);
}

void
tv_minus_floor(struct timeval *tv, struct timeval *mtv)
{
    /* If we are subtracting something after us, 0 this out */
    if (tv->tv_sec < mtv->tv_sec) {
        tv->tv_sec = 0;
        tv->tv_usec = 0;
        return;
    }

    /* Subtract off the (must be less than) seconds */
    tv->tv_sec -= mtv->tv_sec;

    /* If the less guy has less usecs, just subtract them */
    if (tv->tv_usec >= mtv->tv_usec) {
        tv->tv_usec -= mtv->tv_usec;
    /* othewise do the math to line it up right */
    } else {
        tv->tv_sec --;
        tv->tv_usec = US_PER_S - (mtv->tv_usec - tv->tv_usec);
    }
}

int
tv_compare(const struct timeval& lhs, const struct timeval& rhs)
{
    if (lhs.tv_sec < rhs.tv_sec)
        return -1;
    if (lhs.tv_sec > rhs.tv_sec)
        return 1;
    if (lhs.tv_usec < rhs.tv_usec)
        return -1;
    if (lhs.tv_usec > rhs.tv_usec)
        return 1;
    return 0;
}
