#include "jpegEncoder.h"
#include "jpegEncoderData.h"

JPEGEncoder::JPEGEncoder( uint16_t quality, bool fasterCompression ) :
        _data( new JPEGEncoderData( quality, fasterCompression ) )
{

}

JPEGEncoder::~JPEGEncoder()
{
    delete _data;
}

uint16_t JPEGEncoder::quality() const
{
    return _data->_quality;
}

void JPEGEncoder::setQuality( uint16_t quality )
{
    _data->_quality = quality;
    if (_data->_quality > 100)
        _data->_quality = 100;
    if (_data->_quality < 1)
        _data->_quality = 1;
}

bool JPEGEncoder::fasterCompression( ) const
{
    return _data->_fasterCompression;
}
void JPEGEncoder::setFasterCompression( bool faster )
{
    _data->_fasterCompression = faster;
}

void JPEGEncoder::encodeToMemory( const std::shared_ptr<const CapturedImage>& image, uint8_t** buffer, uint32_t* bufferSize )
{
    _data->encodeToMemory(image, buffer, bufferSize);
}
