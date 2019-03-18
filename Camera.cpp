#include "Camera.h"

Nan::Persistent<v8::Function> Camera::constructor;

struct gotImageData
{
    uv_work_t request;
    Camera * camera;
    char * imageData;
    size_t imageLength;
};

NAN_MODULE_INIT(Camera::Init) {
        v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New("Camera").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        Nan::SetPrototypeMethod(tpl, "open", open);
        Nan::SetPrototypeMethod(tpl, "close", close);

        constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
        Nan::Set(target, Nan::New("Camera").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(Camera::New)
{
    if (info.IsConstructCall())
    {
        Camera *obj = new Camera();
        obj->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
    }
    else
    {
        const int argc = 1;
        v8::Local <v8::Value> argv[argc] = {};
        v8::Local <v8::Function> cons = Nan::New(constructor);
        info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
    }
}

Camera::Camera()
{
    _camera = V4LCamera::Create();
}

Camera::~Camera()
{

}
NAN_INLINE void noop (uv_work_t* req)
{

}
NAN_INLINE void gotImageCallback(uv_work_t* req)
{
    Nan::HandleScope scope;
    gotImageData * data = static_cast<gotImageData*>(req->data);

    v8::Local<v8::Object> nodeBuf = Nan::NewBuffer(data->imageData, data->imageLength, [](char * buf, void * ignore){
        delete[] buf;
    }, nullptr).ToLocalChecked();

    v8::Local<v8::Value> argv[] = { nodeBuf };

    if (!data->camera->gotFrameCallback.IsEmpty())
    {
        data->camera->gotFrameCallback.Call(1, argv);
    }
    delete data;
}
void Camera::gotImage(uint8_t * buf, size_t bufSize)
{
    gotImageData * data = new gotImageData();
    data->imageData = new char[bufSize];
    data->imageLength = bufSize;
    memcpy(data->imageData, buf, bufSize);
    data->camera = this;
    data->request.data = (void *)data;
    uv_queue_work(uv_default_loop(), &data->request, noop, reinterpret_cast<uv_after_work_cb>(gotImageCallback));
}

NAN_METHOD(Camera::open)
{
    Camera * obj = Nan::ObjectWrap::Unwrap<Camera>(info.Holder());
    if (obj->_listener == nullptr)
    {
        obj->_listener = new VideoListener();
        obj->_listener->setCallback(Camera::gotImageHandler, (void *)obj);
        obj->_camera->setVideoDevice(0);
        obj->_camera->setVideoSize(1280, 720);
        obj->_camera->setFrameRate(25);
        obj->_camera->setListener(obj->_listener);
        obj->_camera->start();
    }

    if (info.Length() > 0)
    {
        obj->gotFrameCallback.Reset(info[0].As<v8::Function>());
    }
}

NAN_METHOD(Camera::close)
{
    Camera * obj = Nan::ObjectWrap::Unwrap<Camera>(info.Holder());
    if (obj->_listener != nullptr)
    {
        obj->_camera->setListener(nullptr);
        delete obj->_listener;
        obj->_listener = nullptr;
        obj->_camera->signalToStop();
        obj->_camera->waitForStop();
    }
}
