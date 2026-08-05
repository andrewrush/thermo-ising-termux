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
#define DEFAULT_T      1.0
#define PI 3.141592653589793

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

static inline double randf() {
    return (next_rand() % 1000000) / 1000000.0;
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
static float *Sx, *Sy, *Sz;

static inline int idx(int i, int j, int k) {
    i = (i + L) % L; j = (j + L) % L; k = (k + L) % L;
    return (i * L + j) * L + k;
}

void random_spin(int i, float *sx, float *sy, float *sz) {
    double theta = acos(1.0 - 2.0 * randf());
    double phi   = 2.0 * PI * randf();
    *sx = sin(theta) * cos(phi);
    *sy = sin(theta) * sin(phi);
    *sz = cos(theta);
}

float magnetization() {
    double mx = 0, my = 0, mz = 0;
    int N = L * L * L;
    for (int i = 0; i < N; i++) { mx += Sx[i]; my += Sy[i]; mz += Sz[i]; }
    return sqrt(mx*mx + my*my + mz*mz) / N;
}

float energy() {
    double e = 0;
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            for (int k = 0; k < L; k++) {
                int p = idx(i,j,k);
                double dot = Sx[p]*Sx[idx(i+1,j,k)] + Sy[p]*Sy[idx(i+1,j,k)] + Sz[p]*Sz[idx(i+1,j,k)]
                           + Sx[p]*Sx[idx(i,j+1,k)] + Sy[p]*Sy[idx(i,j+1,k)] + Sz[p]*Sz[idx(i,j+1,k)]
                           + Sx[p]*Sx[idx(i,j,k+1)] + Sy[p]*Sy[idx(i,j,k+1)] + Sz[p]*Sz[idx(i,j,k+1)];
                e -= dot;
            }
        }
    }
    return (float)e / (L * L * L);
}

void metropolis_heisenberg(double T, int steps, int print_interval, int csv_mode) {
    int accepted = 0;
    double sum_M = 0, sum_E = 0, sum_M2 = 0, sum_E2 = 0;
    int sample_count = 0;

    for (int step = 0; step < steps; step++) {
        int i = next_rand() % L;
        int j = next_rand() % L;
        int k = next_rand() % L;
        int p = idx(i,j,k);

        float sx_new, sy_new, sz_new;
        random_spin(p, &sx_new, &sy_new, &sz_new);

        double dot_old = Sx[p]*Sx[idx(i-1,j,k)] + Sy[p]*Sy[idx(i-1,j,k)] + Sz[p]*Sz[idx(i-1,j,k)]
                       + Sx[p]*Sx[idx(i+1,j,k)] + Sy[p]*Sy[idx(i+1,j,k)] + Sz[p]*Sz[idx(i+1,j,k)]
                       + Sx[p]*Sx[idx(i,j-1,k)] + Sy[p]*Sy[idx(i,j-1,k)] + Sz[p]*Sz[idx(i,j-1,k)]
                       + Sx[p]*Sx[idx(i,j+1,k)] + Sy[p]*Sy[idx(i,j+1,k)] + Sz[p]*Sz[idx(i,j+1,k)]
                       + Sx[p]*Sx[idx(i,j,k-1)] + Sy[p]*Sy[idx(i,j,k-1)] + Sz[p]*Sz[idx(i,j,k-1)]
                       + Sx[p]*Sx[idx(i,j,k+1)] + Sy[p]*Sy[idx(i,j,k+1)] + Sz[p]*Sz[idx(i,j,k+1)];
        double dot_new = sx_new*Sx[idx(i-1,j,k)] + sy_new*Sy[idx(i-1,j,k)] + sz_new*Sz[idx(i-1,j,k)]
                       + sx_new*Sx[idx(i+1,j,k)] + sy_new*Sy[idx(i+1,j,k)] + sz_new*Sz[idx(i+1,j,k)]
                       + sx_new*Sx[idx(i,j-1,k)] + sy_new*Sy[idx(i,j-1,k)] + sz_new*Sz[idx(i,j-1,k)]
                       + sx_new*Sx[idx(i,j+1,k)] + sy_new*Sy[idx(i,j+1,k)] + sz_new*Sz[idx(i,j+1,k)]
                       + sx_new*Sx[idx(i,j,k-1)] + sy_new*Sy[idx(i,j,k-1)] + sz_new*Sz[idx(i,j,k-1)]
                       + sx_new*Sx[idx(i,j,k+1)] + sy_new*Sy[idx(i,j,k+1)] + sz_new*Sz[idx(i,j,k+1)];

        double dE = -(dot_new - dot_old);
        if (dE <= 0 || exp(-dE / T) > randf()) {
            Sx[p] = sx_new; Sy[p] = sy_new; Sz[p] = sz_new;
            accepted++;
        }

        if (step >= steps / 10 && step % 100 == 0) {
            float M = magnetization();
            float E = energy();
            sum_M  += M;
            sum_E  += E;
            sum_M2 += M * M;
            sum_E2 += E * E;
            sample_count++;
        }

        if (!csv_mode && print_interval > 0 && step > 0 && step % print_interval == 0) {
            printf("    step %d/%d  |M|=%.4f  E=%+.4f\n",
                   step, steps, magnetization(), energy());
        }
    }

    if (sample_count > 0) {
        double avg_M  = sum_M  / sample_count;
        double avg_E  = sum_E  / sample_count;
        double avg_M2 = sum_M2 / sample_count;
        double avg_E2 = sum_E2 / sample_count;
        double chi    = (avg_M2 - avg_M * avg_M) * (L*L*L) / T;
        double C      = (avg_E2 - avg_E * avg_E)   * (L*L*L) / (T*T);

        if (csv_mode) {
            printf("%.4f,%.6f,%.6f,%.6f,%.4f,%.4f,%.2f\n",
                   T, avg_M, avg_E, avg_M2, chi, C,
                   (accepted * 100.0) / steps);
        } else {
            printf("[*] Averages (after thermalization):\n");
            printf("    |M|    = %.6f\n", avg_M);
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
        printf("  3D Heisenberg Model — Monte Carlo\n");
        printf("  L=%d  T=%.4f  steps=%d\n", L, T, steps);
        printf("  Tc(3D) ≈ 1.44 (simple cubic)\n");
        printf("========================================\n");
    }

    int N = L * L * L;
    Sx = malloc(N * sizeof(float));
    Sy = malloc(N * sizeof(float));
    Sz = malloc(N * sizeof(float));
    if (!Sx || !Sy || !Sz) { perror("malloc"); return 1; }

    seed_hw_entropy();
    if (!csv) timer_start();

    for (int i = 0; i < N; i++)
        random_spin(i, &Sx[i], &Sy[i], &Sz[i]);

    if (!csv) {
        printf("[*] Initial |M| = %.4f, E = %+.4f\n", magnetization(), energy());
        printf("[*] Starting Heisenberg annealing on aarch64...\n");
    }

    int print_interval = csv ? 0 : (steps / 10);
    metropolis_heisenberg(T, steps, print_interval, csv);

    if (!csv) {
        timer_stop();
        printf("[*] Final   |M| = %.4f, E = %+.4f\n", magnetization(), energy());
    }

    free(Sx); free(Sy); free(Sz);
    return 0;
}
