CC = clang
CFLAGS = -O3 -march=armv8-a+simd -Wall

# OpenCL flags (Termux)
CLFLAGS = -lOpenCL

all: ising_app wolff_app ising3d_app heisenberg_app gpu_app

ising_app: ising.c
	$(CC) $(CFLAGS) -o ising_app ising.c -lm

wolff_app: wolff.c
	$(CC) $(CFLAGS) -o wolff_app wolff.c -lm

ising3d_app: ising3d.c
	$(CC) $(CFLAGS) -o ising3d_app ising3d.c -lm

heisenberg_app: heisenberg.c
	$(CC) $(CFLAGS) -o heisenberg_app heisenberg.c -lm

gpu_app: gpu_wrapper.c
	-$(CC) $(CFLAGS) -o gpu_app gpu_wrapper.c -lm $(CLFLAGS) 2>/dev/null || echo "[!] GPU build failed (OpenCL missing). Run: pkg install ocl-icd opencl-headers"

clean:
	rm -f ising_app wolff_app ising3d_app heisenberg_app gpu_app
