CC = clang
CFLAGS = -O3 -march=armv8-a+simd -Wall

all: ising_app wolff_app

ising_app: ising.c
	$(CC) $(CFLAGS) -o ising_app ising.c -lm

wolff_app: wolff.c
	$(CC) $(CFLAGS) -o wolff_app wolff.c -lm

clean:
	rm -f ising_app wolff_app
