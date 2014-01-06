#include <sys/time.h>
#include <iostream>
#include <vector>
#include <queue>
#include "../timevals.h"

using namespace std;

class tv_sched_item
{
public:
    tv_sched_item(struct timeval _tv)
        : tv(_tv)
    {}
    struct timeval tv;
};

class tv_sched_comparitor
{
public:
    bool operator()(const tv_sched_item& lhs,
                    const tv_sched_item& rhs) const;
};

bool
tv_sched_comparitor::operator()(const tv_sched_item& lhs,
                                const tv_sched_item& rhs) const
{
    if (tv_compare(lhs.tv, rhs.tv) <= 0)
        return false;
    return true;
}

std::priority_queue<tv_sched_item,
                    std::vector<tv_sched_item>,
                    tv_sched_comparitor> pq;

int
main(int argc, char *argv[])
{
    cout << "Hi, I am test" << endl;
    for (int i = 0; i < 10; i++) {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        tv_sched_item item(tv);
        pq.push(tv);

        usleep(1000 * 680);
    }

    while (pq.size()) {
        struct timeval tv;
        tv = pq.top().tv;
        cout << "tv_sec: " << tv.tv_sec << " tv_usec: " << tv.tv_usec << endl;
        pq.pop();
    }
    return 0;
}
