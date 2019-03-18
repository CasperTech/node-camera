#include "CameraDummy.h"

using v8::FunctionTemplate;

NAN_MODULE_INIT(InitAll) {
        Camera::Init(target);
}

NODE_MODULE(NativeExtension, InitAll)

