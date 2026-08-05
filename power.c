#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Чтение тока/напряжения через termux-battery-status (обход SELinux)
typedef struct {
    float voltage;   // mV
    float current;   // mA (отрицательный = разряд)
    float temp;      // °C
    float power;     // mW
} power_state_t;

power_state_t read_power() {
    power_state_t p = {0, 0, 0, 0};
    FILE *fp = popen("termux-battery-status 2>/dev/null", "r");
    if (!fp) return p;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        char *key;
        if ((key = strstr(line, "\"voltage\""))) {
            key = strchr(key, ':'); if (key) p.voltage = strtof(key + 1, NULL);
        } else if ((key = strstr(line, "\"current\""))) {
            key = strchr(key, ':'); if (key) p.current = strtof(key + 1, NULL);
        } else if ((key = strstr(line, "\"temperature\""))) {
            key = strchr(key, ':'); if (key) p.temp = strtof(key + 1, NULL);
        }
    }
    pclose(fp);
    p.power = p.voltage * p.current / 1000.0f; // mW
    return p;
}

void print_power(const char *label, const power_state_t *p) {
    printf("[*] %s: V=%.0fmV I=%.0fmA T=%.1f°C P=%.1fmW\n",
           label, p->voltage, p->current, p->temp, p->power);
}

// Joules per sample = P(W) * time(s) / samples
double joules_per_sample(double power_mw, double time_s, long samples) {
    double joules = (power_mw / 1000.0) * time_s;
    return joules / samples;
}
