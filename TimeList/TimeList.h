#ifndef WEBSERVER_TIMELIST_H
#define WEBSERVER_TIMELIST_H

#include <vector>
#include "TimeListNode.h"

class TimeList
{
    private:
        std::vector<TimeListNode> fd2NodeMap;
        TimeListNode * head;
        TimeListNode *tail;

        void addToTail(TimeListNode *node);

    public:
        TimeList();
        ~TimeList();
        void attachTimer(HttpConnection *connection);
        void removeTimer(const int &fd);
        void tick();
        void updateTimer(const int &fd);
};

#endif