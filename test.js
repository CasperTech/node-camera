const cam = require('./index');

const camera = new cam.Camera();
camera.onFrame.subscribe((img) => {
    console.log('Got frame of ' + img.length + ' bytes');
});
camera.open();

setTimeout(() => {
    camera.close();
}, 10000);