#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define DEFAULT_L      16
#define DEFAULT_STEPS  2000000
#define DEFAULT_T      4.51

static uint64_t s[2];

void seed_hw_entropy() {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) { perror("Failed to open /dev/urandom"); exit(1); }
    read(fd, s, sizeof(s));
    close(fd);
    printf("[*] Seeded with hardware entropy from SoC.\n");
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

static int L;
static int8_t *spins;

static inline int idx(int i, int j, int k) {
    i = (i + L) % L; j = (j + L) % L; k = (k + L) % L;
    return (i * L + j) * L + k;
}

static inline int8_t get(int i, int j, int k) {
    return spins[idx(i, j, k)];
}

float magnetization() {
    long sum = 0;
    int N = L * L * L;
    for (int i = 0; i < N; i++) sum += spins[i];
    return (float)sum / N;
}

float energy() {
    long e = 0;
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            for (int k = 0; k < L; k++) {
                int8_t c = get(i, j, k);
                e -= c * (get(i+1,j,k) + get(i,j+1,k) + get(i,j,k+1));
            }
        }
    }
    return (float)e / (L * L * L);
}

void metropolis3d(double T, int steps, int print_interval, int csv_mode) {
    int accepted = 0;
    double sum_M = 0, sum_absM = 0, sum_E = 0, sum_M2 = 0, sum_E2 = 0;
    int sample_count = 0;

    for (int step = 0; step < steps; step++) {
        int i = next_rand() % L;
        int j = next_rand() % L;
        int k = next_rand() % L;
        int8_t c = get(i, j, k);
        int8_t nn = get(i-1,j,k) + get(i+1,j,k)
                   + get(i,j-1,k) + get(i,j+1,k)
                   + get(i,j,k-1) + get(i,j,k+1);
        int dE = 2 * c * nn;

        if (dE <= 0 || exp(-dE / T) > (next_rand() % 1000000) / 1000000.0) {
            spins[idx(i,j,k)] = -c;
            accepted++;
        }

        if (step >= steps / 10 && step % 100 == 0) {
            float M = magnetization();
            float E = energy();
            sum_M    += M;
            sum_absM += fabsf(M);
            sum_E    += E;
            sum_M2   += M * M;
            sum_E2   += E * E;
            sample_count++;
        }

        if (!csv_mode && print_interval > 0 && step > 0 && step % print_interval == 0) {
            printf("    step %d/%d  M=%+.4f  E=%+.4f\n",
                   step, steps, magnetization(), energy());
        }
    }

    if (sample_count > 0) {
        double avg_M    = sum_M    / sample_count;
        double avg_absM = sum_absM / sample_count;
        double avg_E    = sum_E    / sample_count;
        double avg_M2   = sum_M2   / sample_count;
        double avg_E2   = sum_E2   / sample_count;
        double chi      = (avg_M2 - avg_absM * avg_absM) * (L*L*L) / T;
        double C        = (avg_E2 - avg_E * avg_E)       * (L*L*L) / (T*T);

        if (csv_mode) {
            printf("%.4f,%.6f,%.6f,%.6f,%.6f,%.4f,%.4f,%.2f\n",
                   T, avg_M, avg_absM, avg_E, avg_M2, chi, C,
                   (accepted * 100.0) / steps);
        } else {
            printf("[*] Averages (after thermalization):\n");
            printf("    <M>    = %+.6f\n", avg_M);
            printf("    <|M|>  = %+.6f\n", avg_absM);
            printf("    <E>    = %+.6f\n", avg_E);
            printf("    chi    = %.4f   (susceptibility)\n", chi);
            printf("    C      = %.4f   (heat capacity)\n", C);
            printf("[*] Acceptance rate: %.2f%%\n", (accepted * 100.0) / steps);
        }
    }
}

int main(int argc, char *argv[]) {
    L         = (argc > 1) ? atoi(argv[1]) : DEFAULT_L;
    double T  = (argc > 2) ? atof(argv[2]) : DEFAULT_T;
    int steps = (argc > 3) ? atoi(argv[3]) : DEFAULT_STEPS;
    int csv   = (argc > 4 && strcmp(argv[4], "csv") == 0) ? 1 : 0;

    if (!csv) {
        printf("========================================\n");
        printf("  3D Ising Model — Monte Carlo\n");
        printf("  L=%d  T=%.4f  steps=%d\n", L, T, steps);
        printf("  Tc(3D) ≈ 4.51 (simple cubic)\n");
        printf("========================================\n");
    }

    spins = calloc(L * L * L, sizeof(int8_t));
    if (!spins) { perror("calloc"); return 1; }

    seed_hw_entropy();
    if (!csv) timer_start();

    for (int i = 0; i < L*L*L; i++)
        spins[i] = (next_rand() % 2) ? 1 : -1;

    if (!csv) {
        printf("[*] Initial M = %+.4f, E = %+.4f\n", magnetization(), energy());
        printf("[*] Starting 3D annealing on aarch64...\n");
    }

    int print_interval = csv ? 0 : (steps / 10);
    metropolis3d(T, steps, print_interval, csv);

    if (!csv) {
        timer_stop();
        printf("[*] Final   M = %+.4f, E = %+.4f\n", magnetization(), energy());
    }

    free(spins);
    return 0;
}
