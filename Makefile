CC = clang
CFLAGS = -O3 -march=armv8-a+simd -Wall

all: ising_app

ising_app: ising.c
	$(CC) $(CFLAGS) -o ising_app ising.c -lm

clean:
	rm -f ising_app
