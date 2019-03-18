"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const cmake = require("node-cmake");
const Subject_1 = require("rxjs/internal/Subject");
const cam = cmake('node_camera');
class Camera {
    constructor() {
        this.onFrame = new Subject_1.Subject();
        this.camera = new cam.Camera();
    }
    open() {
        this.camera.open((jpegImage) => {
            this.onFrame.next(jpegImage);
        });
    }
    close() {
        this.camera.close();
    }
}
exports.Camera = Camera;
//# sourceMappingURL=index.js.map