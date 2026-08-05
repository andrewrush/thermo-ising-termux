// 2D Ising Model — WebGL Visualization
// Metropolis algorithm in JS, rendered as pixel texture

const canvas = document.getElementById('glcanvas');
const gl = canvas.getContext('webgl', { preserveDrawingBuffer: true });

// UI elements
const tSlider = document.getElementById('tSlider');
const lSlider = document.getElementById('lSlider');
const sSlider = document.getElementById('sSlider');
const tVal = document.getElementById('tVal');
const lVal = document.getElementById('lVal');
const sVal = document.getElementById('sVal');
const pauseBtn = document.getElementById('pauseBtn');
const resetBtn = document.getElementById('resetBtn');
const stepStat = document.getElementById('stepStat');
const mStat = document.getElementById('mStat');
const eStat = document.getElementById('eStat');
const aStat = document.getElementById('aStat');

// Simulation state
let L = 128;
let T = 2.27;
let spins = new Int8Array(L * L);
let steps = 0;
let accepted = 0;
let running = true;
let speed = 1;

// XorShift128+ PRNG
let rngS = new Uint32Array([123456789, 987654321]);
function seedRNG() {
    rngS[0] = Math.floor(Math.random() * 0xFFFFFFFF);
    rngS[1] = Math.floor(Math.random() * 0xFFFFFFFF);
}
function nextRand() {
    let s1 = rngS[0];
    const s0 = rngS[1];
    rngS[0] = s0;
    s1 ^= s1 << 23;
    rngS[1] = (s1 ^ s0 ^ (s1 >>> 17) ^ (s0 >>> 26)) >>> 0;
    return (rngS[1] + s0) >>> 0;
}

function randFloat() { return (nextRand() % 1000000) / 1000000.0; }

function initSpins() {
    for (let i = 0; i < L * L; i++) spins[i] = Math.random() > 0.5 ? 1 : -1;
    steps = 0; accepted = 0;
}

function get(i, j) {
    return spins[(((i % L) + L) % L) * L + ((j % L) + L) % L];
}

function metropolisStep() {
    const sweeps = [1, 5, 20, 100, 500][speed];
    for (let n = 0; n < sweeps; n++) {
        const i = nextRand() % L;
        const j = nextRand() % L;
        const c = get(i, j);
        const nn = get(i-1,j) + get(i+1,j) + get(i,j-1) + get(i,j+1);
        const dE = 2 * c * nn;
        if (dE <= 0 || Math.exp(-dE / T) > randFloat()) {
            spins[i * L + j] = -c;
            accepted++;
        }
        steps++;
    }
}

function magnetization() {
    let sum = 0;
    for (let i = 0; i < L * L; i++) sum += spins[i];
    return sum / (L * L);
}

function energy() {
    let e = 0;
    for (let i = 0; i < L; i++) {
        for (let j = 0; j < L; j++) {
            const c = get(i, j);
            e -= c * (get(i, j+1) + get(i+1, j));
        }
    }
    return e / (L * L);
}

// WebGL setup
const vsSource = `
attribute vec2 a_pos;
varying vec2 v_uv;
void main() {
    v_uv = a_pos * 0.5 + 0.5;
    gl_Position = vec4(a_pos, 0.0, 1.0);
}`;

const fsSource = `
precision mediump float;
varying vec2 v_uv;
uniform sampler2D u_tex;
void main() {
    vec4 c = texture2D(u_tex, v_uv);
    // c.r = 1 for spin +1, c.r = 0 for spin -1
    vec3 up = vec3(0.345, 0.655, 1.0);    // #58a6ff
    vec3 down = vec3(0.545, 0.545, 0.545); // #8b949e
    vec3 col = mix(down, up, c.r);
    gl_FragColor = vec4(col, 1.0);
}`;

function compileShader(src, type) {
    const s = gl.createShader(type);
    gl.shaderSource(s, src);
    gl.compileShader(s);
    return s;
}

const prog = gl.createProgram();
gl.attachShader(prog, compileShader(vsSource, gl.VERTEX_SHADER));
gl.attachShader(prog, compileShader(fsSource, gl.FRAGMENT_SHADER));
gl.linkProgram(prog);
gl.useProgram(prog);

// Fullscreen quad
const buf = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, buf);
gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1,-1, 1,-1, -1,1, 1,1]), gl.STATIC_DRAW);
const aPos = gl.getAttribLocation(prog, 'a_pos');
gl.enableVertexAttribArray(aPos);
gl.vertexAttribPointer(aPos, 2, gl.FLOAT, false, 0, 0);

// Texture
let texture = gl.createTexture();
let pixelData = new Uint8Array(L * L * 4);

function updateTexture() {
    for (let i = 0; i < L * L; i++) {
        const v = spins[i] > 0 ? 255 : 0;
        pixelData[i*4] = v;
        pixelData[i*4+1] = v;
        pixelData[i*4+2] = v;
        pixelData[i*4+3] = 255;
    }
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, L, L, 0, gl.RGBA, gl.UNSIGNED_BYTE, pixelData);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
}

function resizeCanvas() {
    const size = Math.min(window.innerWidth - 40, 512);
    canvas.style.width = size + 'px';
    canvas.style.height = size + 'px';
}

function recreate() {
    L = parseInt(lSlider.value);
    spins = new Int8Array(L * L);
    pixelData = new Uint8Array(L * L * 4);
    initSpins();
    updateTexture();
}

// Event listeners
tSlider.addEventListener('input', () => { T = parseFloat(tSlider.value); tVal.textContent = T.toFixed(2); });
lSlider.addEventListener('change', () => { lVal.textContent = L; recreate(); });
sSlider.addEventListener('input', () => { speed = parseInt(sSlider.value); sVal.textContent = ['медленно','норм','быстро','очень быстро','макс'][speed]; });
pauseBtn.addEventListener('click', () => { running = !running; pauseBtn.textContent = running ? '⏸ Пауза' : '▶ Продолжить'; });
resetBtn.addEventListener('click', () => { initSpins(); updateTexture(); });
window.addEventListener('resize', resizeCanvas);

// Init
seedRNG();
initSpins();
updateTexture();
resizeCanvas();
sVal.textContent = 'норм';

// Animation loop
let lastStats = 0;
function loop() {
    if (running) {
        metropolisStep();
        updateTexture();
        gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);
    }

    const now = performance.now();
    if (now - lastStats > 200) {
        stepStat.textContent = steps.toLocaleString();
        mStat.textContent = magnetization().toFixed(3);
        eStat.textContent = energy().toFixed(3);
        aStat.textContent = (steps > 0 ? (accepted * 100 / steps).toFixed(1) : 0) + '%';
        lastStats = now;
    }

    requestAnimationFrame(loop);
}
loop();
