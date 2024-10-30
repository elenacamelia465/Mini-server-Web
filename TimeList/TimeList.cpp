#include "TimeList.h"

TimeList::TimeList(): fd2NodeMap(Config::maxConnectFd), head(new TimeListNode()), tail (new TimeListNode())
{
    head->next = tail;
    tail->prev = head;
}

TimeList::~TimeList()
{
    delete head;
    delete tail;
}

void TimeList::addToTail(TimeListNode * node)
{
    node->next = tail;
    node->prev = tail->prev;
    tail->prev->next = node;
    tail->prev = node;
}

void TimeList::attachTimer(HttpConnection *connection)
{
    removeTimer(connection->getFd());
    fd2NodeMap[connection->getFd()].expire = time(nullptr) + Config::timeoutSecond;
    fd2NodeMap[connection->getFd()].httpConnection = connection;
    addToTail(&fd2NodeMap[connection->getFd()]);
}

void TimeList::removeTimer(const int &fd)
{
    auto &timeListNode = fd2NodeMap[fd];
    if(timeListNode.next == nullptr || timeListNode.prev == nullptr)
        return;
    timeListNode.prev->next = timeListNode.next;
    timeListNode.next->prev= timeListNode.prev;
    timeListNode.next= nullptr;
    timeListNode.prev = nullptr;
}

void TimeList::tick()
{
    if(head->next == tail)
    {
        return;
    }
    time_t current = time(nullptr);
    while(head->next !=tail && head->next->expire < current)
    {
        auto httpConnection = head->next->httpConnection;
        removeTimer(httpConnection->getFd());
        httpConnection->closeConnection();
    }
}

void TimeList::updateTimer(const int &fd)
{
    auto &timeListNode = fd2NodeMap[fd];
    timeListNode.expire = time(nullptr) + Config::timeoutSecond;
    removeTimer(fd);
    addToTail(&timeListNode);
}