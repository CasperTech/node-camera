#include "CameraDummy.h"

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

}

Camera::~Camera()
{

}
NAN_METHOD(Camera::open)
        {

        }

NAN_METHOD(Camera::close)
        {

        }

