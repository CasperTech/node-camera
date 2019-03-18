#pragma once

#include <stdint.h>
#include <memory>
#include "interfaces/IUncopyable.h"
#include "CapturedImage.h"
#include <stdexcept>

class JPEGEncoderData;


class JPEGEncoder : private IUncopyable
{
public:

    JPEGEncoder( uint16_t quality = 85, bool fasterCompression = false );
    ~JPEGEncoder( );

    uint16_t quality( ) const;
    void setQuality( uint16_t quality );
    bool fasterCompression( ) const;
    void setFasterCompression( bool faster );
    void encodeToMemory( const std::shared_ptr<const CapturedImage>& image, uint8_t** buffer, uint32_t* bufferSize );

private:
    JPEGEncoderData* _data;
};
