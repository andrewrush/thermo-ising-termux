#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <string.h>

#define DEFAULT_L      64
#define DEFAULT_STEPS  100000
#define DEFAULT_T      2.269

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

float get_battery_temp() {
    FILE *fp = popen("termux-battery-status 2>/dev/null", "r");
    if (!fp) return -1.0f;
    char line[256];
    float temp = -1.0f;
    while (fgets(line, sizeof(line), fp)) {
        char *p = strstr(line, "\"temperature\"");
        if (p) {
            p = strchr(p, ':');
            if (p) temp = strtof(p + 1, NULL);
        }
    }
    pclose(fp);
    return temp;
}

void print_temp() {
    float t = get_battery_temp();
    if (t >= 0) printf("[*] Battery/SoC Temp: %.1f C\n", t);
    else printf("[!] Temp: termux-battery-status unavailable\n");
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
static int *queue_i;
static int *queue_j;
static uint8_t *visited;

static inline int8_t get(int i, int j) {
    return spins[((i + L) % L) * L + ((j + L) % L)];
}
static inline void set(int i, int j, int8_t val) {
    spins[((i + L) % L) * L + ((j + L) % L)] = val;
}

float magnetization() {
    long sum = 0;
    for (int i = 0; i < L * L; i++) sum += spins[i];
    return (float)sum / (L * L);
}

float energy() {
    long e = 0;
    for (int i = 0; i < L; i++) {
        for (int j = 0; j < L; j++) {
            int8_t c = get(i, j);
            e -= c * (get(i, j+1) + get(i+1, j));
        }
    }
    return (float)e / (L * L);
}

// === Wolff cluster update ===
// Returns cluster size
int wolff_step(double p) {
    int i0 = next_rand() % L;
    int j0 = next_rand() % L;
    int8_t s0 = get(i0, j0);
    int8_t flip_to = -s0;

    int qhead = 0, qtail = 0;
    queue_i[qtail] = i0; queue_j[qtail] = j0; qtail++;
    visited[i0 * L + j0] = 1;
    set(i0, j0, flip_to);
    int cluster_size = 1;

    int di[4] = {-1, 1, 0, 0};
    int dj[4] = {0, 0, -1, 1};

    while (qhead < qtail) {
        int ci = queue_i[qhead];
        int cj = queue_j[qhead];
        qhead++;
        for (int d = 0; d < 4; d++) {
            int ni = ci + di[d];
            int nj = cj + dj[d];
            int wi = ((ni % L) + L) % L;
            int wj = ((nj % L) + L) % L;
            int idx = wi * L + wj;
            if (!visited[idx] && get(wi, wj) == s0) {
                double r = (next_rand() % 1000000) / 1000000.0;
                if (r < p) {
                    visited[idx] = 1;
                    queue_i[qtail] = wi;
                    queue_j[qtail] = wj;
                    qtail++;
                    set(wi, wj, flip_to);
                    cluster_size++;
                }
            }
        }
    }

    // clear visited
    for (int i = 0; i < qtail; i++) {
        visited[queue_i[i] * L + queue_j[i]] = 0;
    }
    return cluster_size;
}

void wolff_sim(double T, int steps, int print_interval, int csv_mode) {
    double p = 1.0 - exp(-2.0 / T);
    double sum_M = 0, sum_absM = 0, sum_E = 0, sum_M2 = 0, sum_E2 = 0;
    int sample_count = 0;
    long total_flipped = 0;

    for (int step = 0; step < steps; step++) {
        int cs = wolff_step(p);
        total_flipped += cs;

        if (step >= steps / 10 && step % 10 == 0) {
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
            printf("    step %d/%d  M=%+.4f  E=%+.4f  cs=%d\n",
                   step, steps, magnetization(), energy(), cs);
        }
    }

    if (sample_count > 0) {
        double avg_M    = sum_M    / sample_count;
        double avg_absM = sum_absM / sample_count;
        double avg_E    = sum_E    / sample_count;
        double avg_M2   = sum_M2   / sample_count;
        double avg_E2   = sum_E2   / sample_count;
        double chi      = (avg_M2 - avg_absM * avg_absM) * (L * L) / T;
        double C        = (avg_E2 - avg_E * avg_E)       * (L * L) / (T * T);

        if (csv_mode) {
            printf("%.4f,%.6f,%.6f,%.6f,%.6f,%.4f,%.4f,%.2f\n",
                   T, avg_M, avg_absM, avg_E, avg_M2, chi, C,
                   (total_flipped * 100.0) / (steps * L * L));
        } else {
            printf("[*] Averages (after thermalization):\n");
            printf("    <M>    = %+.6f\n", avg_M);
            printf("    <|M|>  = %+.6f\n", avg_absM);
            printf("    <E>    = %+.6f\n", avg_E);
            printf("    chi    = %.4f   (susceptibility)\n", chi);
            printf("    C      = %.4f   (heat capacity)\n", C);
            printf("[*] Total flipped spins: %ld (%.2f%% of lattice)\n",
                   total_flipped, (total_flipped * 100.0) / (steps * L * L));
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
        printf("  2D Ising — Wolff Cluster Algorithm\n");
        printf("  L=%d  T=%.4f  steps=%d\n", L, T, steps);
        printf("========================================\n");
    }

    spins    = calloc(L * L, sizeof(int8_t));
    queue_i  = malloc(L * L * sizeof(int));
    queue_j  = malloc(L * L * sizeof(int));
    visited  = calloc(L * L, sizeof(uint8_t));
    if (!spins || !queue_i || !queue_j || !visited) { perror("alloc"); return 1; }

    if (!csv) print_temp();
    seed_hw_entropy();
    if (!csv) timer_start();

    for (int i = 0; i < L * L; i++)
        spins[i] = (next_rand() % 2) ? 1 : -1;

    if (!csv) {
        printf("[*] Initial M = %+.4f, E = %+.4f\n", magnetization(), energy());
        printf("[*] Wolff p = %.6f\n", 1.0 - exp(-2.0 / T));
        printf("[*] Starting cluster updates on aarch64...\n");
    }

    int print_interval = csv ? 0 : (steps / 10);
    wolff_sim(T, steps, print_interval, csv);

    if (!csv) {
        timer_stop();
        printf("[*] Final   M = %+.4f, E = %+.4f\n", magnetization(), energy());
        print_temp();
    }

    free(spins); free(queue_i); free(queue_j); free(visited);
    return 0;
}
