#include <stdexcept>
#include <cstring>
#include "CapturedImage.h"

// Returns number of bits required for pixel in certain format
uint32_t CapturedImage::ImageBitsPerPixel( PixelFormat format )
{
    static int sizes[]     = { 0, 8, 24, 32, 8 };
    int        formatIndex = static_cast<int>( format );

    return ( formatIndex >= ( sizeof( sizes ) / sizeof( sizes[0] ) ) ) ? 0 : sizes[formatIndex];
}

// Returns number of bytes per stride when number of bits per line is known (stride is always 32 bit aligned)
uint32_t CapturedImage::ImageBytesPerStride( uint32_t bitsPerLine )
{
    return ( ( bitsPerLine + 31 ) & ~31 ) >> 3;
}

// Returns number of bytes per line when number of bits per line is known (line is always 8 bit aligned)
uint32_t CapturedImage::ImageBytesPerLine( uint32_t bitsPerLine )
{
    return ( bitsPerLine + 7 ) >> 3;
}

// Create empty image
CapturedImage::CapturedImage( uint8_t* data, int32_t width, int32_t height, int32_t stride, PixelFormat format, bool ownMemory ) :
        _data( data ), _width( width ), _height( height ), _stride( stride ), _format( format ), _ownMemory( ownMemory )
{
}

// Destroy image
CapturedImage::~CapturedImage( )
{
    if ( ( _ownMemory ) && ( _data != nullptr ) )
    {
        free( _data );
    }
}

// Allocate image of the specified size and format
std::shared_ptr<CapturedImage> CapturedImage::Allocate( int32_t width, int32_t height, PixelFormat format, bool zeroInitialize )
{
    int32_t  stride = (int32_t) ImageBytesPerStride( width * ImageBitsPerPixel( format ) );
    CapturedImage*  image  = nullptr;
    uint8_t* data   = nullptr;

    if ( zeroInitialize )
    {
        data = (uint8_t*) calloc( 1, height * stride );
    }
    else
    {
        data = (uint8_t*) malloc( height * stride );
    }

    if ( data != nullptr )
    {
        image = new (std::nothrow)CapturedImage( data, width, height, stride, format, true );
    }

    return std::shared_ptr<CapturedImage>( image );
}

// Create image by wrapping existing memory buffer
std::shared_ptr<CapturedImage> CapturedImage::Create( uint8_t* data, int32_t width, int32_t height, int32_t stride, PixelFormat format )
{
    return std::shared_ptr<CapturedImage>( new (std::nothrow)CapturedImage( data, width, height, stride, format, false ) );
}

// Clone image - make a deep copy of it
std::shared_ptr<CapturedImage> CapturedImage::Clone( ) const
{
    std::shared_ptr <CapturedImage> clone;

    if (_data != nullptr)
    {
        clone = Allocate(_width, _height, _format);

        if (clone)
        {
            try
            {
                CopyData(clone);
            }
            catch(...)
            {
                clone.reset();
            }
        }
    }

    return clone;
}

// Copy content of the image - destination image must have same width/height/format
void CapturedImage::CopyData( const std::shared_ptr<CapturedImage>& copyTo ) const
{
    if ((_data == nullptr) || (!copyTo) || (copyTo->_data == nullptr))
    {
        throw std::runtime_error("Null pointer");
    }
        // for JPEGs we just make sure there is enough space to copy the image data,
        // but for all uncompressed formats we check for exact match of width/height
    else if ((_height != copyTo->_height) || (_format != copyTo->_format))
    {
        throw std::runtime_error("Image parameters mismatch");
    }
    else if (((_format != PixelFormat::JPEG) && (_width != copyTo->_width)) ||
             ((_format == PixelFormat::JPEG) && (_stride > copyTo->_stride)))
    {
        throw std::runtime_error("Image parameters mismatch");
    }
    else
    {
        uint32_t lineSize = ImageBytesPerLine(_width * ImageBitsPerPixel(_format));
        uint8_t * srcPtr = _data;
        uint8_t * dstPtr = copyTo->_data;
        int32_t dstStride = copyTo->_stride;

        for (int y = 0; y < _height; y++)
        {
            memcpy(dstPtr, srcPtr, lineSize);
            srcPtr += _stride;
            dstPtr += dstStride;
        }
    }
}

// Copy content of the image into the specified one if its size/format is same or make a clone
void CapturedImage::CopyDataOrClone( std::shared_ptr<CapturedImage>& copyTo ) const
{
    if ((!copyTo) ||
        (copyTo->Height() != _height) ||
        (copyTo->Format() != _format) ||
        ((_format != PixelFormat::JPEG) && (copyTo->Width() != _width)) ||
        ((_format == PixelFormat::JPEG) && (copyTo->Stride() < _stride))
            )
    {
        copyTo = Clone();
        if (!copyTo)
        {
            throw std::runtime_error("Out of memory when copying image");
        }
    }
    else
    {
        CopyData(copyTo);
    }
}
