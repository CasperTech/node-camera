#pragma once

#include <memory>

#include "interfaces/ICamera.h"
#include "interfaces/ICameraListener.h"
#include "interfaces/IUncopyable.h"
#include "V4LCameraData.h"
#include "VideoProperty.h"
#include "ChannelIndex.h"

// Class which provides access to cameras using V4L2 API (Video for Linux, v2)
class V4LCamera : public ICamera, private IUncopyable
{
protected:
    V4LCamera();

public:
    ~V4LCamera() override;
    bool start() override;
    void signalToStop() override;
    void waitForStop() override;
    bool isRunning() override;
    uint32_t framesReceived() override;
    ICameraListener * setListener(ICameraListener * listener);

    static const std::shared_ptr<V4LCamera> Create();
    static void DecodeYuyvToRgb( const uint8_t* yuyvPtr, uint8_t* rgbPtr, int32_t width, int32_t height, int32_t rgbStride );

    uint32_t videoDevice() const;
    void setVideoDevice(uint32_t videoDevice);
    uint32_t width() const;
    uint32_t height() const;
    void setVideoSize(uint32_t width, uint32_t height);
    // Get/Set frame rate
    uint32_t frameRate() const;
    void setFrameRate(uint32_t frameRate);
    bool isJpegEncodingEnabled() const;
    void enableJpegEncoding(bool enable);

public:
    void setVideoProperty(VideoProperty property, int32_t value);

    void getVideoProperty(VideoProperty property, int32_t * value) const;

    void getVideoPropertyRange(VideoProperty property, int32_t * min, int32_t * max, int32_t * step, int32_t * def) const;

private:
    V4LCameraData * _data = nullptr;
};