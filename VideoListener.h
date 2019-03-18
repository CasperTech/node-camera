#pragma once

#include "CapturedImage.h"
#include "jpegEncoder.h"
#include "interfaces/ICameraListener.h"
#include <mutex>
#include <string>
#include <memory>
#include <functional>

class VideoListener : public ICameraListener
{
public:

    VideoListener();
    ~VideoListener();

    void onNewImage( const std::shared_ptr<const CapturedImage>& image );

    void onError( const std::string& errorMessage, bool fatal );

    void setCallback(std::function<void(void *, uint8_t *, size_t)> jpegCallbackFunction, void * jpegCallbackData);

private:
    JPEGEncoder _jpegEncoder;
    void * _cbData = nullptr;
    std::function<void(void *, uint8_t *, size_t)> _cbFunc;

    uint8_t * _jpegBuffer = nullptr;
    size_t _jpegBufferSize = 0;
    size_t _jpegSize = 0;
    std::mutex _imgLock;
};

