#pragma once
#include <stdio.h>
#include <cstdint>
#include <unistd.h>
#include <jpeglib.h>
#include <memory>

class CapturedImage;

class JPEGEncoderData
{

public:

    static void errorExit( j_common_ptr /* cinfo */ )
    {
        throw std::runtime_error("Jpeg exception");
    }

    static void outputMessage( j_common_ptr /* cinfo */ )
    {
        // do nothing - kill the message
    }

    JPEGEncoderData(uint16_t quality, bool fasterCompression) :
            _quality(quality), _fasterCompression(fasterCompression)
    {
        if (_quality > 100)
        {
            _quality = 100;
        }

        // allocate and initialize JPEG compression object
        _cinfo.err = jpeg_std_error(&_jerr);
        _jerr.error_exit = errorExit;
        _jerr.output_message = outputMessage;

        jpeg_create_compress(&_cinfo);
    }

    ~JPEGEncoderData()
    {
        jpeg_destroy_compress(&_cinfo);
    }

    void encodeToMemory(const std::shared_ptr<const CapturedImage> &image, uint8_t ** buffer, uint32_t * bufferSize);

    uint16_t _quality;
    bool _fasterCompression;

private:
    struct jpeg_compress_struct _cinfo;
    struct jpeg_error_mgr _jerr;
};