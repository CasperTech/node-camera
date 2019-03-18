#pragma once

#include <memory>

#include "interfaces/IUncopyable.h"
#include "PixelFormat.h"

// Class encapsulating image data
class CapturedImage : private IUncopyable
{
private:
    CapturedImage(uint8_t * data, int32_t width, int32_t height, int32_t stride, PixelFormat format, bool ownMemory);

public:
    ~CapturedImage();

    static uint32_t ImageBitsPerPixel( PixelFormat format );
    static uint32_t ImageBytesPerStride( uint32_t bitsPerLine );
    static uint32_t ImageBytesPerLine( uint32_t bitsPerLine );

    // Allocate image of the specified size and format
    static std::shared_ptr<CapturedImage>
    Allocate(int32_t width, int32_t height, PixelFormat format, bool zeroInitialize = false);

    // Create image by wrapping existing memory buffer
    static std::shared_ptr<CapturedImage>
    Create(uint8_t * data, int32_t width, int32_t height, int32_t stride, PixelFormat format);

    // Clone image - make a deep copy of it
    std::shared_ptr<CapturedImage> Clone() const;

    // Copy content of the image - destination image must have same width/height/format
    void CopyData(const std::shared_ptr<CapturedImage> &copyTo) const;

    // Copy content of the image into the specified one if its size/format is same or make a clone
    void CopyDataOrClone(std::shared_ptr<CapturedImage> &copyTo) const;

    // Image properties
    int32_t Width() const
    {
        return _width;
    }

    int32_t Height() const
    {
        return _height;
    }

    int32_t Stride() const
    {
        return _stride;
    }

    PixelFormat Format() const
    {
        return _format;
    }

    // Raw data of the image
    uint8_t * Data() const
    {
        return _data;
    }

private:
    uint8_t * _data;
    int32_t _width;
    int32_t _height;
    int32_t _stride;
    PixelFormat _format;
    bool _ownMemory;
};