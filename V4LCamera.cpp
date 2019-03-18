#include "V4LCamera.h"
#include "CapturedImage.h"
#include <chrono>
#include <mutex>

const std::shared_ptr<V4LCamera> V4LCamera::Create( )
{
    return std::shared_ptr<V4LCamera>(new V4LCamera);
}

V4LCamera::V4LCamera( ) :
        _data( new V4LCameraData( ) )
{
}

V4LCamera::~V4LCamera( )
{
    delete _data;
}
bool V4LCamera::start( )
{
    return _data->start();
}

void V4LCamera::signalToStop( )
{
    _data->signalToStop();
}

void V4LCamera::waitForStop( )
{
    _data->waitForStop( );
}

bool V4LCamera::isRunning( )
{
    return _data->isRunning();
}

uint32_t V4LCamera::framesReceived( )
{
    return _data->_framesReceived;
}

// Set video source listener
ICameraListener* V4LCamera::setListener( ICameraListener* listener )
{
    return _data->setListener(listener);
}

// Set/get video device
uint32_t V4LCamera::videoDevice( ) const
{
    return _data->_videoDevice;
}
void V4LCamera::setVideoDevice( uint32_t videoDevice )
{
    _data->setVideoDevice(videoDevice);
}

// Get/Set video size
uint32_t V4LCamera::width( ) const
{
    return _data->_frameWidth;
}
uint32_t V4LCamera::height( ) const
{
    return _data->_frameHeight;
}
void V4LCamera::setVideoSize( uint32_t width, uint32_t height )
{
    _data->setVideoSize(width, height);
}

// Get/Set frame rate
uint32_t V4LCamera::frameRate( ) const
{
    return _data->_frameRate;
}
void V4LCamera::setFrameRate( uint32_t frameRate )
{
    _data->setFrameRate(frameRate);
}

// Enable/Disable JPEG encoding
bool V4LCamera::isJpegEncodingEnabled( ) const
{
    return _data->_jpegEncoding;
}
void V4LCamera::enableJpegEncoding( bool enable )
{
    _data->enableJpegEncoding(enable);
}

// Set the specified video property
void V4LCamera::setVideoProperty( VideoProperty property, int32_t value )
{
    _data->setVideoProperty(property, value);
}

// Get current value if the specified video property
void V4LCamera::getVideoProperty( VideoProperty property, int32_t* value ) const
{
    _data->getVideoProperty(property, value);
}

// Get range of values supported by the specified video property
void V4LCamera::getVideoPropertyRange( VideoProperty property, int32_t* min, int32_t* max, int32_t* step, int32_t* def ) const
{
    _data->getVideoPropertyRange(property, min, max, step, def);
}




// Helper function to decode YUYV data into RGB
void V4LCamera::DecodeYuyvToRgb( const uint8_t* yuyvPtr, uint8_t* rgbPtr, int32_t width, int32_t height, int32_t rgbStride )
{
    /*
        The code below does YUYV to RGB conversion using the next coefficients.
        However those are multiplied by 256 to get integer calculations.

        r = y + (1.4065 * (cr - 128));
        g = y - (0.3455 * (cb - 128)) - (0.7169 * (cr - 128));
        b = y + (1.7790 * (cb - 128));
    */

    int r, g, b;
    int y, u, v;
    int z = 0;

    for (int32_t iy = 0; iy < height; iy++)
    {
        uint8_t * rgbRow = rgbPtr + iy * rgbStride;

        for (int32_t ix = 0; ix < width; ix++)
        {
            y = ((z == 0) ? yuyvPtr[0] : yuyvPtr[2]) << 8;
            u = yuyvPtr[1] - 128;
            v = yuyvPtr[3] - 128;

            r = (y + (360 * v)) >> 8;
            g = (y - (88 * u) - (184 * v)) >> 8;
            b = (y + (455 * u)) >> 8;

            rgbRow[RedIndex] = (uint8_t)(r > 255) ? 255 : ((r < 0) ? 0 : r);
            rgbRow[GreenIndex] = (uint8_t)(g > 255) ? 255 : ((g < 0) ? 0 : g);
            rgbRow[BlueIndex] = (uint8_t)(b > 255) ? 255 : ((b < 0) ? 0 : b);

            if (z++)
            {
                z = 0;
                yuyvPtr += 4;
            }

            rgbRow += 3;
        }
    }
}



