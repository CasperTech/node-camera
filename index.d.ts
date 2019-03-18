/// <reference types="node" />
import { Subject } from 'rxjs/internal/Subject';
export declare class Camera {
    onFrame: Subject<Buffer>;
    private camera;
    constructor();
    open(): void;
    close(): void;
}
