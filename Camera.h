#pragma once
#include <nan.h>
#include "V4LCamera.h"
#include "VideoListener.h"

class Camera: public Nan::ObjectWrap
{
public:
    static NAN_MODULE_INIT(Init);
    static void gotImageHandler(void * data, uint8_t * buf, size_t bufSize)
    {
        auto cam = static_cast<Camera *>(data);
        cam->gotImage(buf, bufSize);
    }
    void gotImage(uint8_t * buf, size_t bufSize);
    Nan::Callback gotFrameCallback;

private:
    std::shared_ptr<V4LCamera> _camera;
    VideoListener * _listener = nullptr;

    explicit Camera();
    ~Camera();
    static NAN_METHOD(New);
    static NAN_METHOD(open);
    static NAN_METHOD(close);
    static Nan::Persistent<v8::Function> constructor;
};

