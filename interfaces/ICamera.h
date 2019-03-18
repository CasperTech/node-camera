#pragma once

#include <stdint.h>
#include "ICameraListener.h"

class ICamera
{
public:
    virtual ~ICamera()
    {
    }

    virtual bool start() = 0;
    virtual void signalToStop() = 0;
    virtual void waitForStop() = 0;
    virtual bool isRunning() = 0;

    virtual uint32_t framesReceived() = 0;

    virtual ICameraListener * setListener(ICameraListener * listener) = 0;
};
