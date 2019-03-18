#pragma once

#include "../CapturedImage.h"
#include <memory>
#include <string>
#include <list>

// Interface for video source events' listener
class ICameraListener
{
public:
    virtual ~ICameraListener()
    {
    }

    // New video frame notification
    virtual void onNewImage(const std::shared_ptr<const CapturedImage> &image) = 0;

    // Video source error notification
    virtual void onError(const std::string &errorMessage, bool fatal) = 0;
};