#pragma once
#include <nan.h>

class Camera: public Nan::ObjectWrap
{
public:
    static NAN_MODULE_INIT(Init);
    Nan::Callback gotFrameCallback;

private:
    explicit Camera();
    ~Camera();
    static NAN_METHOD(New);
    static NAN_METHOD(open);
    static NAN_METHOD(close);
    static Nan::Persistent<v8::Function> constructor;
};

