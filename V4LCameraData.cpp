#include "V4LCameraData.h"
#include "V4LCamera.h"

static const uint32_t nativeVideoProperties[] =
        {
                V4L2_CID_BRIGHTNESS,
                V4L2_CID_CONTRAST,
                V4L2_CID_SATURATION,
                V4L2_CID_HUE,
                V4L2_CID_SHARPNESS,
                V4L2_CID_GAIN,
                V4L2_CID_BACKLIGHT_COMPENSATION,
                V4L2_CID_RED_BALANCE,
                V4L2_CID_BLUE_BALANCE,
                V4L2_CID_AUTO_WHITE_BALANCE,
                V4L2_CID_HFLIP,
                V4L2_CID_VFLIP
        };

// Start video source so it initializes and begins providing video frames
bool V4LCameraData::start( )
{
    std::lock_guard<std::recursive_mutex> lock(_sync);

    if (!isRunning())
    {
        _needToStop.reset();
        _running = true;
        _framesReceived = 0;

        _controlThread = std::thread(controlThreadHandler, this);
    }

    return true;
}

// Signal video to stop, so it could finalize and clean-up
void V4LCameraData::signalToStop( )
{
    std::lock_guard<std::recursive_mutex> lock(_sync);

    if (isRunning())
    {
        _needToStop.signal();
    }
}

// Wait till video source (its thread) stops
void V4LCameraData::waitForStop( )
{
    signalToStop();

    if ((isRunning()) || (_controlThread.joinable()))
    {
        _controlThread.join();
    }
}

// Check if video source is still running
bool V4LCameraData::isRunning( )
{
    std::lock_guard<std::recursive_mutex> lock(_sync);

    if ((!_running) && (_controlThread.joinable()))
    {
        _controlThread.join();
    }

    return _running;
}

// Set video source listener
ICameraListener* V4LCameraData::setListener( ICameraListener* listener )
{
    std::lock_guard<std::recursive_mutex> lock(_sync);
    ICameraListener * oldListener = listener;

    _listener = listener;

    return oldListener;
}

// Notify listener with a new image
void V4LCameraData::notifyNewImage( const std::shared_ptr<const CapturedImage>& image )
{
    ICameraListener * myListener;

    {
        std::lock_guard<std::recursive_mutex> lock(_sync);
        myListener = _listener;
    }

    if (myListener != nullptr)
    {
        myListener->onNewImage(image);
    }
}

// Notify listener about error
void V4LCameraData::notifyError( const std::string& errorMessage, bool fatal )
{
    ICameraListener * myListener;

    {
        std::lock_guard<std::recursive_mutex> lock(_sync);
        myListener = _listener;
    }

    if (myListener != nullptr)
    {
        myListener->onError(errorMessage, fatal);
    }
}

// Initialize camera and start capturing
bool V4LCameraData::Init( )
{
    std::lock_guard<std::recursive_mutex> lock(_configSync);
    char strVideoDevice[32];
    bool ret = true;
    int ecode;

    sprintf(strVideoDevice, "/dev/video%d", _videoDevice);

    // open video device
    _videoFd = open(strVideoDevice, O_RDWR);
    if (_videoFd == -1)
    {
        notifyError("Failed opening video device", true);
        ret = false;
    }
    else
    {
        v4l2_capability videoCapability = {0};

        // get video capabilities of the device
        ecode = ioctl(_videoFd, VIDIOC_QUERYCAP, &videoCapability);
        if (ecode < 0)
        {
            notifyError("Failed getting video capabilities of the device", true);
            ret = false;
        }
            // make sure device supports video capture
        else if ((videoCapability.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0)
        {
            notifyError("Device does not support video capture", true);
            ret = false;
        }
        else if ((videoCapability.capabilities & V4L2_CAP_STREAMING) == 0)
        {
            notifyError("Device does not support streaming", true);
            ret = false;
        }
    }

    // configure video format
    if (ret)
    {
        v4l2_format videoFormat = {0};
        uint32_t pixelFormat = (_jpegEncoding) ? V4L2_PIX_FMT_MJPEG : V4L2_PIX_FMT_YUYV;

        videoFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        videoFormat.fmt.pix.width = _frameWidth;
        videoFormat.fmt.pix.height = _frameHeight;
        videoFormat.fmt.pix.pixelformat = pixelFormat;
        videoFormat.fmt.pix.field = V4L2_FIELD_ANY;

        ecode = ioctl(_videoFd, VIDIOC_S_FMT, &videoFormat);
        if (ecode < 0)
        {
            notifyError("Failed setting video format", true);
            ret = false;
        }
        else if (videoFormat.fmt.pix.pixelformat != pixelFormat)
        {
            notifyError(std::string("The camera does not support requested format: ") + ((_jpegEncoding) ? "MJPEG" : "YUYV"),
                        true);
            ret = false;
        }
        else
        {
            // update width/height in case camera does not support what was requested
            _frameWidth = videoFormat.fmt.pix.width;
            _frameHeight = videoFormat.fmt.pix.height;
        }
    }

    // request capture buffers
    if (ret)
    {
        v4l2_requestbuffers requestBuffers = {0};

        requestBuffers.count = BUFFER_COUNT;
        requestBuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        requestBuffers.memory = V4L2_MEMORY_MMAP;

        ecode = ioctl(_videoFd, VIDIOC_REQBUFS, &requestBuffers);
        if (ecode < 0)
        {
            notifyError("Unable to allocate capture buffers", true);
            ret = false;
        }
        else if (requestBuffers.count < BUFFER_COUNT)
        {
            notifyError("Not enough memory to allocate capture buffers", true);
            ret = false;
        }
    }

    // map capture buffers
    if (ret)
    {
        v4l2_buffer videoBuffer;

        for (int i = 0; i < BUFFER_COUNT; i++)
        {
            memset(&videoBuffer, 0, sizeof(videoBuffer));

            videoBuffer.index = i;
            videoBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            videoBuffer.memory = V4L2_MEMORY_MMAP;

            ecode = ioctl(_videoFd, VIDIOC_QUERYBUF, &videoBuffer);
            if (ecode < 0)
            {
                notifyError("Unable to query capture buffer", true);
                ret = false;
                break;
            }

            _mappedBuffers[i] = (uint8_t *)mmap(0, videoBuffer.length, PROT_READ, MAP_SHARED, _videoFd,
                                               videoBuffer.m.offset);
            _mappedBufferLength[i] = videoBuffer.length;

            if (_mappedBuffers[i] == nullptr)
            {
                notifyError("Unable to map capture buffer", true);
                ret = false;
                break;
            }
        }
    }

    // enqueue capture buffers
    if (ret)
    {
        v4l2_buffer videoBuffer;

        for (int i = 0; i < BUFFER_COUNT; i++)
        {
            memset(&videoBuffer, 0, sizeof(videoBuffer));

            videoBuffer.index = i;
            videoBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            videoBuffer.memory = V4L2_MEMORY_MMAP;

            ecode = ioctl(_videoFd, VIDIOC_QBUF, &videoBuffer);

            if (ecode < 0)
            {
                notifyError("Unable to enqueue capture buffer", true);
                ret = false;
            }
        }
    }

    // enable video streaming
    if (ret)
    {
        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        ecode = ioctl(_videoFd, VIDIOC_STREAMON, &type);
        if (ecode < 0)
        {
            notifyError("Failed starting video streaming", true);
            ret = false;
        }
        else
        {
            _videoStreamingActive = true;
        }
    }

    // configure all properties, which were set before device got running
    if (ret)
    {
        bool configOK = true;

        for (auto property : _propertiesToSet)
        {
            try
            {
                setVideoProperty(property.first, property.second);
            }
            catch(...)
            {
                notifyError("Failed applying video configuration");
            }
        }
        _propertiesToSet.clear();
    }

    return ret;
}

// Stop camera capture and clean-up
void V4LCameraData::cleanup()
{
    std::lock_guard<std::recursive_mutex> lock(_configSync);

    // disable vide streaming
    if (_videoStreamingActive)
    {
        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        ioctl(_videoFd, VIDIOC_STREAMOFF, &type);
        _videoStreamingActive = false;
    }

    // unmap capture buffers
    for (int i = 0; i < BUFFER_COUNT; i++)
    {
        if (_mappedBuffers[i] != nullptr)
        {
            munmap(_mappedBuffers[i], _mappedBufferLength[i]);
            _mappedBuffers[i] = nullptr;
            _mappedBufferLength[i] = 0;
        }
    }

    // close the video device
    if (_videoFd != -1)
    {
        close(_videoFd);
        _videoFd = -1;
    }
}

// Do video capture in an end-less loop until signalled to stop
void V4LCameraData::videoCaptureLoop( )
{
    v4l2_buffer videoBuffer;
    uint32_t sleepTime = 0;
    uint32_t frameTime = 1000 / _frameRate;
    uint32_t handlingTime;
    int ecode;

    // If JPEG encoding is used, client is notified with an image wrapping a mapped buffer.
    // If not used howver, we decode YUYV data into RGB.
    std::shared_ptr<CapturedImage> rgbImage;

    if (!_jpegEncoding)
    {
        rgbImage = CapturedImage::Allocate(_frameWidth, _frameHeight, PixelFormat::RGB24);

        if (!rgbImage)
        {
            notifyError("Failed allocating an image");
            return;
        }
    }

    // acquire images untill we've been told to stop
    while (!_needToStop.wait(sleepTime))
    {
        std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

        // dequeue buffer
        memset(&videoBuffer, 0, sizeof(videoBuffer));

        videoBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        videoBuffer.memory = V4L2_MEMORY_MMAP;

        ecode = ioctl(_videoFd, VIDIOC_DQBUF, &videoBuffer);
        if (ecode < 0)
        {
            notifyError("Failed to dequeue capture buffer");
        }
        else
        {
            std::shared_ptr <CapturedImage> image;

            _framesReceived++;

            if (_jpegEncoding)
            {
                image = CapturedImage::Create(_mappedBuffers[videoBuffer.index], videoBuffer.bytesused, 1, videoBuffer.bytesused,
                                              PixelFormat::JPEG);
            }
            else
            {
                V4LCamera::DecodeYuyvToRgb(_mappedBuffers[videoBuffer.index], rgbImage->Data(), _frameWidth, _frameHeight,
                                rgbImage->Stride());
                image = rgbImage;
            }

            if (image)
            {
                notifyNewImage(image);
            }
            else
            {
                notifyError("Failed allocating an image");
            }

            // put the buffer back into the queue
            ecode = ioctl(_videoFd, VIDIOC_QBUF, &videoBuffer);
            if (ecode < 0)
            {
                notifyError("Failed to requeue capture buffer");
            }
        }

        handlingTime = static_cast<uint32_t>( std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count());
        sleepTime = (handlingTime > frameTime) ? 0 : (frameTime - handlingTime);
    }
}


// Background control thread - performs camera init/clean-up and runs video loop
void V4LCameraData::controlThreadHandler( V4LCameraData* me )
{
    if (me->Init())
    {
        me->videoCaptureLoop();
    }

    me->cleanup();

    {
        std::lock_guard<std::recursive_mutex> lock(me->_sync);
        me->_running = false;
    }
}

void V4LCameraData::setVideoDevice( uint32_t videoDevice )
{
    std::lock_guard<std::recursive_mutex> lock(_sync);

    if (!isRunning())
    {
        _videoDevice = videoDevice;
    }
}

void V4LCameraData::setVideoSize( uint32_t width, uint32_t height )
{
    std::lock_guard<std::recursive_mutex> lock(_sync);

    if (!isRunning())
    {
        _frameWidth = width;
        _frameHeight = height;
    }
}

void V4LCameraData::setFrameRate( uint32_t frameRate )
{
    std::lock_guard<std::recursive_mutex> lock(_sync);

    if (!isRunning())
    {
        _frameRate = frameRate;
    }
}


void V4LCameraData::enableJpegEncoding( bool enable )
{
    std::lock_guard<std::recursive_mutex> lock(_sync);

    if (!isRunning())
    {
        _jpegEncoding = enable;
    }
}

// Set the specified video property
void V4LCameraData::setVideoProperty( VideoProperty property, int32_t value )
{
    std::lock_guard<std::recursive_mutex> lock(_sync);

    if ((property < VideoProperty::Brightness) || (property > VideoProperty::Gain))
    {
        throw std::runtime_error("Unknown property");
    }
    else if ((!_running) || (_videoFd == -1))
    {
        // save property value and try setting it when device gets runnings
        _propertiesToSet[property] = value;
    }
    else
    {
        v4l2_control control;

        control.id = nativeVideoProperties[static_cast<int>( property )];
        control.value = value;

        if (ioctl(_videoFd, VIDIOC_S_CTRL, &control) < 0)
        {
            throw std::runtime_error("Failed to set video property");
        }
    }
}

// Get current value if the specified video property
void V4LCameraData::getVideoProperty( VideoProperty property, int32_t* value ) const
{
    std::lock_guard<std::recursive_mutex> lock(_sync);

    if (value == nullptr)
    {
        throw std::runtime_error("Null pointer");
    }
    else if ((property < VideoProperty::Brightness) || (property > VideoProperty::Gain))
    {
        throw std::runtime_error("Unknown property");
    }
    else if ((!_running) || (_videoFd == -1))
    {
        throw std::runtime_error("Device not ready");
    }
    else
    {
        v4l2_control control;

        control.id = nativeVideoProperties[static_cast<int>( property )];

        if (ioctl(_videoFd, VIDIOC_G_CTRL, &control) < 0)
        {
            throw std::runtime_error("Failed to get video property");
        }
        else
        {
            *value = control.value;
        }
    }
}

// Get range of values supported by the specified video property
void V4LCameraData::getVideoPropertyRange( VideoProperty property, int32_t* min, int32_t* max, int32_t* step, int32_t* def ) const
{



    std::lock_guard<std::recursive_mutex> lock(_sync);

    if ((min == nullptr) || (max == nullptr) || (step == nullptr) || (def == nullptr))
    {
        throw std::runtime_error("Null pointer");
    }
    else if ((property < VideoProperty::Brightness) || (property > VideoProperty::Gain))
    {
        throw std::runtime_error("Unknown property");
    }
    else if ((!_running) || (_videoFd == -1))
    {
        throw std::runtime_error("Device not ready");
    }
    else
    {
        v4l2_queryctrl queryControl;

        queryControl.id = nativeVideoProperties[static_cast<int>( property )];

        if (ioctl(_videoFd, VIDIOC_QUERYCTRL, &queryControl) < 0)
        {
            throw std::runtime_error("Failed");
        }
        else if ((queryControl.flags & V4L2_CTRL_FLAG_DISABLED) != 0)
        {
            throw std::runtime_error("Configuration not supported");
        }
        else if ((queryControl.type & (V4L2_CTRL_TYPE_BOOLEAN | V4L2_CTRL_TYPE_INTEGER)) != 0)
        {

            *min = queryControl.minimum;
            *max = queryControl.maximum;
            *step = queryControl.step;
            *def = queryControl.default_value;
        }
        else
        {
            throw std::runtime_error("Configuration not supported");
        }
    }
}
