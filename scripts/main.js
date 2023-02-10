function make_environment(...envs) {
    return new Proxy(envs, {
        get(target, prop, receiver) {
            for (let env of envs) {
                if (env.hasOwnProperty(prop)) {
                    return env[prop];
                }
            }
            return (...args) => {console.error("NOT IMPLEMENTED: "+prop, args)}
        }
    });
}

const libm = {
    "atan2f": Math.atan2,
    "cosf": Math.cos,
    "sinf": Math.sin,
    "sqrtf": Math.sqrt,
};

let iota = 0;

const CANVAS_PIXELS = iota++;
const CANVAS_WIDTH  = iota++;
const CANVAS_HEIGHT = iota++;
const CANVAS_STRIDE = iota++;
const CANVAS_SIZE   = iota++;

function readCanvasFromMemory(memory_buffer, canvas_ptr)
{
    const canvas_memory = new Uint32Array(memory_buffer, canvas_ptr, CANVAS_SIZE);
    return {
        pixels: canvas_memory[CANVAS_PIXELS],
        width: canvas_memory[CANVAS_WIDTH],
        height: canvas_memory[CANVAS_HEIGHT],
        stride: canvas_memory[CANVAS_STRIDE],
    };
}

async function startDemo(elementId, wasmPath) {
    const app = document.getElementById(elementId);
    if (app === null) {
        console.error(`Could not find element ${elementId}. Skipping demo ${wasmPath}...`);
        return;
    }

    const ctx = app.getContext("2d");
    const w = await WebAssembly.instantiateStreaming(fetch(wasmPath), {
        "env": make_environment(libm)
    });

    const heap_base = w.instance.exports.__heap_base.value;

    let prev = null;
    function first(timestamp) {
        prev = timestamp;
        window.requestAnimationFrame(loop);
    }
    function loop(timestamp) {
        const dt = timestamp - prev;
        prev = timestamp;

        const buffer = w.instance.exports.memory.buffer;
        w.instance.exports.render(heap_base, dt*0.001);
        const canvas = readCanvasFromMemory(buffer, heap_base);
        if (canvas.width != canvas.stride) {
            console.error(`Canvas width (${canvas.width}) is not equal to its stride (${canvas.stride}). Unfortunately we can't easily support that in a browser because ImageData simply does not accept stride. Welcome to 2022.`);
            return;
        }
        const image = new ImageData(new Uint8ClampedArray(buffer, canvas.pixels, canvas.width*canvas.height*4), canvas.width);
        app.width = canvas.width;
        app.height = canvas.height;
        ctx.putImageData(image, 0, 0);
        window.requestAnimationFrame(loop);
    }
    window.requestAnimationFrame(first);
}