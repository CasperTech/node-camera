#include "jpegEncoderData.h"
#include "CapturedImage.h"
#include "PixelFormat.h"
#include <memory>

void JPEGEncoderData::encodeToMemory( const std::shared_ptr<const CapturedImage>& image, uint8_t** buffer, uint32_t* bufferSize )
{
    JSAMPROW    row_pointer[1];

    if ( ( !image ) || ( image->Data( ) == nullptr ) || ( buffer == nullptr ) || ( *buffer == nullptr ) || ( bufferSize == nullptr ) )
    {
        throw std::runtime_error("Null Pointer");
    }
    else if ( ( image->Format( ) != PixelFormat::RGB24 ) && ( image->Format( ) != PixelFormat::Grayscale8 ) )
    {
        throw std::runtime_error("Unsupported pixel format");
    }
    else
    {
        try
        {
            // 1 - specify data destination
            unsigned long mem_buffer_size = *bufferSize;
            jpeg_mem_dest( &_cinfo, buffer, &mem_buffer_size );

            // 2 - set parameters for compression
            _cinfo.image_width  = image->Width( );
            _cinfo.image_height = image->Height( );

            if ( image->Format( ) == PixelFormat::RGB24 )
            {
                _cinfo.input_components = 3;
                _cinfo.in_color_space   = JCS_RGB;
            }
            else
            {
                _cinfo.input_components = 1;
                _cinfo.in_color_space   = JCS_GRAYSCALE;
            }

            // set default compression parameters
            jpeg_set_defaults( &_cinfo );
            // set quality
            jpeg_set_quality( &_cinfo, (int) _quality, TRUE /* limit to baseline-JPEG values */ );

            // use faster, but less accurate compressions
            _cinfo.dct_method = ( _fasterCompression ) ? JDCT_FASTEST : JDCT_DEFAULT;

            // 3 - start compressor
            jpeg_start_compress( &_cinfo, TRUE );

            // 4 - do compression
            while ( _cinfo.next_scanline < _cinfo.image_height )
            {
                row_pointer[0] = image->Data( ) + image->Stride( ) * _cinfo.next_scanline;

                jpeg_write_scanlines( &_cinfo, row_pointer, 1 );
            }

            // 5 - finish compression
            jpeg_finish_compress( &_cinfo );

            *bufferSize = (uint32_t) mem_buffer_size;
        }
        catch (...)
        {
            throw std::runtime_error("Failed to encode image");
        }
    }
}
