import * as cmake from '@caspertech/node-cmake'
import {Subject} from 'rxjs/internal/Subject';
const cam = cmake('node_camera', false, __dirname);

export class Camera
{
    onFrame: Subject<Buffer> = new Subject<Buffer>();

    private camera: any;

    constructor() {
        this.camera = new cam.Camera();
    }

    open() {
        this.camera.open((jpegImage: Buffer) => {
            this.onFrame.next(jpegImage);
        });
    }

    close() {
        this.camera.close();
    }
}