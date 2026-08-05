#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

// Сид из /dev/urandom (аппаратный RNG SoC)
void seed_from_urandom(uint64_t *s, int n) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) { perror("urandom"); exit(1); }
    read(fd, s, n * sizeof(uint64_t));
    close(fd);
    printf("[*] Seeded from /dev/urandom (SoC HWRNG).\n");
}

// Попытка дополнительной энтропии из termux-sensor (Accelerometer)
// Возвращает 1 если успешно, 0 если нет
int seed_from_accelerometer(uint64_t *s) {
    FILE *fp = popen("termux-sensor -s Accelerometer -d 50 -n 1 2>/dev/null", "r");
    if (!fp) return 0;
    char line[512];
    long raw_x = 0, raw_y = 0, raw_z = 0;
    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        // Парсим JSON вида {"x":0.123,"y":-0.456,"z":9.81}
        char *px = strstr(line, "\"x\"");
        char *py = strstr(line, "\"y\"");
        char *pz = strstr(line, "\"z\"");
        if (px && py && pz) {
            px = strchr(px, ':'); if (px) raw_x = (long)(strtod(px+1, NULL) * 1e6);
            py = strchr(py, ':'); if (py) raw_y = (long)(strtod(py+1, NULL) * 1e6);
            pz = strchr(pz, ':'); if (pz) raw_z = (long)(strtod(pz+1, NULL) * 1e6);
            found = 1;
        }
    }
    pclose(fp);
    if (found) {
        // XOR младших бит сырых данных в сид
        s[0] ^= (uint64_t)(raw_x & 0xFFFFFFFF);
        s[1] ^= (uint64_t)(raw_y & 0xFFFFFFFF) << 32;
        s[0] ^= (uint64_t)(raw_z & 0xFFFFFFFF);
        printf("[*] Mixed accelerometer entropy (x=%ld, y=%ld, z=%ld).\n", raw_x, raw_y, raw_z);
        return 1;
    }
    return 0;
}
