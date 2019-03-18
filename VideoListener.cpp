#include "VideoListener.h"
#include "PixelFormat.h"
#include <memory.h>
#include <cstdint>
#define JPEG_BUF_SIZE (1024 * 1024)

VideoListener::VideoListener()
        : _jpegEncoder(85, true)
{
}

VideoListener::~VideoListener()
{
    if (_jpegBuffer != nullptr)
    {
        delete[] _jpegBuffer;
        _jpegBuffer = nullptr;
    }
}

void VideoListener::setCallback(std::function<void(void *, uint8_t *, size_t)> jpegCallbackFunction, void * jpegCallbackData)
{
    _cbFunc = jpegCallbackFunction;
    _cbData = jpegCallbackData;
}

void VideoListener::onNewImage( const std::shared_ptr<const CapturedImage>& image )
{
    std::unique_lock<std::mutex> lk(_imgLock);
    if (image->Format() == PixelFormat::JPEG)
    {
        // check allocated buffer size
        if ( _jpegBufferSize < static_cast<size_t>( image->Width() ) )
        {
            _jpegBufferSize = image->Width();

            if (_jpegBuffer != nullptr)
            {
                delete[] _jpegBuffer;
                _jpegBuffer = nullptr;
            }
            fflush(stdout);
            _jpegBuffer = new uint8_t[_jpegBufferSize];
        }
        if ( _jpegBuffer != nullptr )
        {
            // just copy JPEG data if we got already encoded image
            memcpy( _jpegBuffer, image->Data(), image->Width() );
            _jpegSize = image->Width();
        }
        fflush(stdout);
    }
    else
    {
        // encode image as JPEG (buffer is re-allocated if too small by encoder)
        _jpegSize = _jpegBufferSize;
        try
        {
            _jpegEncoder.encodeToMemory( image, &_jpegBuffer, &_jpegSize );
            fflush(stdout);
        }
        catch(...)
        {

        }
    }
    if (_jpegBuffer != nullptr && _jpegSize > 0 && _cbFunc)
    {
        _cbFunc(_cbData, _jpegBuffer, _jpegSize);
    }
}

void VideoListener::onError( const std::string& errorMessage, bool fatal )
{

}