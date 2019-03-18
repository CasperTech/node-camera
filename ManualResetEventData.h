#pragma once
#include <mutex>
#include <condition_variable>

class ManualResetEventData
{
public:
    ManualResetEventData() :
            _counter(0), _triggered(false),
            _mutex(), _condVariable()
    {

    }

    uint32_t _counter;
    bool _triggered;
    std::mutex _mutex;
    std::condition_variable _condVariable;
};
