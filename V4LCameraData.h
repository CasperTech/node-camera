#pragma once

#include "interfaces/ICameraListener.h"
#include "VideoProperty.h"
#include "CapturedImage.h"
#include "ManualResetEvent.h"
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <map>
#include <thread>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <cstring>
#include <sys/mman.h>
#include <unistd.h>

#define BUFFER_COUNT        (4)

class V4LCameraData
{
public:
    V4LCameraData() :
            _sync(), _configSync(), _controlThread(), _needToStop(), _listener(nullptr), _running(false),
            _videoFd(-1), _videoStreamingActive(false), _mappedBuffers(), _mappedBufferLength(), _propertiesToSet(),
            _videoDevice(0), _framesReceived(0), _frameWidth(640), _frameHeight(480), _frameRate(30), _jpegEncoding(true)
    {
    }

    bool start();
    void signalToStop();
    void waitForStop();
    bool isRunning();

    ICameraListener * setListener(ICameraListener * listener);

    void notifyNewImage(const std::shared_ptr<const CapturedImage> &image);
    void notifyError(const std::string &errorMessage, bool fatal = false);

    static void controlThreadHandler(V4LCameraData * me);
    void setVideoDevice(uint32_t videoDevice);
    void setVideoSize(uint32_t width, uint32_t height);
    void setFrameRate(uint32_t frameRate);
    void enableJpegEncoding(bool enable);
    void setVideoProperty(VideoProperty property, int32_t value);
    void getVideoProperty(VideoProperty property, int32_t * value) const;
    void getVideoPropertyRange(VideoProperty property, int32_t * min, int32_t * max, int32_t * step, int32_t * def) const;

    uint32_t _videoDevice;
    uint32_t _framesReceived;
    uint32_t _frameWidth;
    uint32_t _frameHeight;
    uint32_t _frameRate;
    bool _jpegEncoding;

private:
    bool Init();
    void videoCaptureLoop();
    void cleanup();

    mutable std::recursive_mutex _sync;
    std::recursive_mutex _configSync;
    std::thread _controlThread;
    ManualResetEvent _needToStop;
    ICameraListener * _listener;
    bool _running;

    int _videoFd;
    bool _videoStreamingActive;
    uint8_t * _mappedBuffers[BUFFER_COUNT];
    uint32_t _mappedBufferLength[BUFFER_COUNT];
    std::map<VideoProperty, int32_t> _propertiesToSet;



};