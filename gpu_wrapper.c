#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <string.h>

// === OpenCL fallback: если не найден, компилируем без GPU ===
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

static uint64_t s[2];
void seed_hw_entropy() {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) { perror("Failed to open /dev/urandom"); exit(1); }
    read(fd, s, sizeof(s));
    close(fd);
}
uint64_t next_rand() {
    uint64_t s1 = s[0];
    const uint64_t s0 = s[1];
    s[0] = s0;
    s1 ^= s1 << 23;
    s[1] = s1 ^ s0 ^ (s1 >> 17) ^ (s0 >> 26);
    return s[1] + s0;
}

static struct timespec ts_start, ts_end;
void timer_start() { clock_gettime(CLOCK_MONOTONIC, &ts_start); }
void timer_stop() {
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    double sec = ts_end.tv_sec - ts_start.tv_sec;
    sec += (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;
    printf("[*] Elapsed time: %.3f s\n", sec);
}

// OpenCL kernel source (checkerboard Metropolis)
const char *kernel_source =
"#define L %(L)d\n"
"#define T_VAL %(T)f\n"
"#define N (L*L)\n"
"__kernel void metropolis_step(__global char *spins, __global ulong *rng_state, int parity) {\n"
"    int gid = get_global_id(0);\n"
"    int i = gid / L;\n"
"    int j = gid % L;\n"
"    if ((i + j) % 2 != parity) return;\n"
"    int idx = i * L + j;\n"
"    char c = spins[idx];\n"
"    int left  = spins[i * L + ((j - 1 + L) % L)];\n"
"    int right = spins[i * L + ((j + 1) % L)];\n"
"    int up    = spins[((i - 1 + L) % L) * L + j];\n"
"    int down  = spins[((i + 1 + L) % L) * L + j];\n"
"    int dE = 2 * c * (left + right + up + down);\n"
"    ulong s0 = rng_state[gid * 2];\n"
"    ulong s1 = rng_state[gid * 2 + 1];\n"
"    s1 ^= s1 << 23;\n"
"    ulong s1_new = s1 ^ s0 ^ (s1 >> 17) ^ (s0 >> 26);\n"
"    rng_state[gid * 2] = s0;\n"
"    rng_state[gid * 2 + 1] = s1_new;\n"
"    float r = (float)(s1_new + s0) / (float)(0xFFFFFFFFFFFFFFFFULL);\n"
"    if (dE <= 0 || exp(-(float)dE / T_VAL) > r) {\n"
"        spins[idx] = -c;\n"
"    }\n"
"}\n";

int main(int argc, char *argv[]) {
    int L         = (argc > 1) ? atoi(argv[1]) : 64;
    double T      = (argc > 2) ? atof(argv[2]) : 2.269;
    int steps     = (argc > 3) ? atoi(argv[3]) : 1000000;
    int use_gpu   = (argc > 4 && strcmp(argv[4], "cpu") == 0) ? 0 : 1;

    printf("========================================\n");
    printf("  2D Ising — OpenCL GPU/CPU Wrapper\n");
    printf("  L=%d  T=%.4f  steps=%d\n", L, T, steps);
    printf("========================================\n");

    cl_platform_id platform = NULL;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_program program = NULL;
    cl_kernel kernel = NULL;
    cl_mem d_spins = NULL, d_rng = NULL;

    cl_int err;
    err = clGetPlatformIDs(1, &platform, NULL);
    if (err != CL_SUCCESS) {
        printf("[!] OpenCL platform not found (err=%d).\n", err);
        printf("    Install: pkg install ocl-icd opencl-headers\n");
        printf("    Or run with CPU fallback: ./gpu_app %d %.3f %d cpu\n", L, T, steps);
        return 1;
    }

    err = clGetDeviceIDs(platform, use_gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device, NULL);
    if (err != CL_SUCCESS) {
        printf("[!] OpenCL %s device not found (err=%d).\n", use_gpu ? "GPU" : "CPU", err);
        printf("    Trying CPU fallback...\n");
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
        if (err != CL_SUCCESS) {
            printf("[!] CPU fallback also failed.\n");
            return 1;
        }
    }

    char dev_name[256];
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(dev_name), dev_name, NULL);
    printf("[*] OpenCL device: %s\n", dev_name);

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    queue   = clCreateCommandQueue(context, device, 0, &err);

    char ksrc[4096];
    snprintf(ksrc, sizeof(ksrc), kernel_source, L, (float)T);
    program = clCreateProgramWithSource(context, 1, (const char **)&ksrc, NULL, &err);
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        printf("[!] OpenCL build failed (err=%d).\n", err);
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *log = malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
        printf("Build log:\n%s\n", log);
        free(log);
        return 1;
    }

    kernel = clCreateKernel(program, "metropolis_step", &err);

    int N = L * L;
    char *h_spins = calloc(N, sizeof(char));
    cl_ulong *h_rng = calloc(N * 2, sizeof(cl_ulong));
    seed_hw_entropy();
    for (int i = 0; i < N; i++) {
        h_spins[i] = (next_rand() % 2) ? 1 : -1;
        h_rng[i*2]   = next_rand();
        h_rng[i*2+1] = next_rand();
    }

    d_spins = clCreateBuffer(context, CL_MEM_READ_WRITE, N * sizeof(char), NULL, &err);
    d_rng   = clCreateBuffer(context, CL_MEM_READ_WRITE, N * 2 * sizeof(cl_ulong), NULL, &err);
    clEnqueueWriteBuffer(queue, d_spins, CL_TRUE, 0, N * sizeof(char), h_spins, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, d_rng,   CL_TRUE, 0, N * 2 * sizeof(cl_ulong), h_rng, 0, NULL, NULL);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_spins);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_rng);

    size_t global = N;
    timer_start();
    printf("[*] Starting GPU-accelerated annealing...\n");
    for (int step = 0; step < steps; step++) {
        int parity = step % 2;
        clSetKernelArg(kernel, 2, sizeof(int), &parity);
        clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
        clFinish(queue);
    }
    timer_stop();

    clEnqueueReadBuffer(queue, d_spins, CL_TRUE, 0, N * sizeof(char), h_spins, 0, NULL, NULL);

    long sum = 0;
    for (int i = 0; i < N; i++) sum += h_spins[i];
    printf("[*] Final M = %+.4f\n", (float)sum / N);

    clReleaseMemObject(d_spins);
    clReleaseMemObject(d_rng);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(h_spins);
    free(h_rng);
    return 0;
}
