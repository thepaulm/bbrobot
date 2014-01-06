#ifndef _TIMEVALS_H
#define _TIMEVALS_H

#include <sys/select.h>

void tv_plus_us(struct timeval *tv, unsigned us);
void tv_plus_ms(struct timeval *tv, unsigned ms);
void tv_minus_floor(struct timeval *tv, struct timeval *mtv);
int  tv_compare(const struct timeval& lhs, const struct timeval& rhs);

#endif /* _TIMEVALS_H */
