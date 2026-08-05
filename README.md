# thermo-ising-termux

Симулятор спиновых моделей (Изинг, Гейзенберг) для Termux на Android aarch64. Оптимизирован под ARM64 NEON, с поддержкой OpenCL GPU (экспериментально).

## What works now

### Модели и алгоритмы

| Модель | Размерность | Алгоритм | Файл | Статус |
|---|---|---|---|---|
| 2D Ising | 2D | Metropolis | `ising.c` | ✅ Работает |
| 2D Ising | 2D | Wolff cluster | `wolff.c` | ✅ Работает |
| 3D Ising | 3D | Metropolis | `ising3d.c` | ✅ Работает |
| 3D Heisenberg | 3D | Metropolis (continuous spins) | `heisenberg.c` | ✅ Работает |
| 2D Ising | 2D | OpenCL checkerboard | `gpu_wrapper.c` | ⚠️ Экспериментально (требует OpenCL-драйвер) |

### Бенчмарки (Snapdragon 8 Gen 2, Termux)

| Модель | L | Steps | Время | Acceptance | <|M|> | <E> |
|---|---|---|---|---|---|---|
| 2D Ising | 64 | 5M | 2.5s | 24% | 0.41 | -1.43 |
| 2D Ising (Wolff) | 64 | 100k кластеров | 12.5s | — | 0.60 | -1.40 |
| 3D Ising | 16 | 2M | 0.66s | 48% | 0.27 | -1.35 |
| 3D Heisenberg | 16 | 2M | 0.86s | 43% | 0.32 | -1.03 |

### Энергометрика

| Модель | Steps | Joules/sample |
|---|---|---|
| 2D Ising | 1M | 3.68 µJ |
| 3D Ising | 2M | 0.65 µJ |
| Heisenberg | 2M | 0.83 µJ |

Измерено через `termux-battery-status` (voltage × current × time / steps).

## Quick start

```bash
# Сборка
make

# 2D Ising (по умолчанию: L=64, T=2.269, 10M шагов)
./ising_app
# Ожидаемый вывод: <|M|> ≈ 0.4–0.6, <E> ≈ -1.4, acceptance ≈ 20–25%

# ASCII-визуализация доменов (█ = +1, ░ = -1)
./ascii_viz.sh 32 2.0 500000
# При T=2.0 (< Tc=2.269) увидите рост магнитных доменов
# При T=3.0 (> Tc) — хаос

# Энергометрика
./benchmark_power.sh 64 1000000

# Сканирование фазового перехода
./scan_phase.sh 64 5000000
# Результат: phase_L64.csv
```

## For ordinary people

**Почему магнит теряет силу при нагревании?**

Магнит состоит из миллиардов микроскопических стрелочек (спинов). При низкой температуре они смотрят в одну сторону. При нагревании начинают трястись и разбегаться. Наш симулятор показывает этот момент: чёрные и серые пятна (магнитные домены) сливаются или распадаются на хаос при определённой температуре.

**Почему вода закипает ровно при 100°C, а не постепенно?**

Фазовые переходы — резкие скачки, а не плавные изменения. Вода не становится «чуть-чуть газом» — она внезапно закипает. Точно так же магнит внезапно теряет силу. Код вычисляет эту «точку невозврата» для магнитной решётки.

**Может ли телефон считать серьёзную науку?**

Да. Android — это 8-ядерный ARM64-компьютер с аппаратным генератором случайных чисел. Мы используем `/dev/urandom` (SoC HWRNG) для инициализации симуляций — телефон буквально использует свой тепловой шум для научного эксперимента.

## Research context: thermodynamic computing

Этот репозиторий — **software reference implementation** для понимания процессов, которые термодинамические процессоры (например, [Extropic Z1](https://extropic.ai/writing/from-one-to-one-billion)) выполняют аппаратно.

| | Software (this repo) | Hardware (Extropic Z1) |
|---|---|---|
| Базовый элемент | Классический бит в RAM | `pbit` (вероятностный бит) |
| Энтропия | HWRNG `/dev/urandom` + PRNG | Физический тепловой шум транзисторов |
| Сэмплирование | Metropolis / Wolff (циклы на CPU) | Физическая релаксация в кремнии |
| Узкое место | Memory Wall (данные в/из RAM) | Вычисление и память совмещены |
| Энергия | ~2 Вт (CPU NEON) | < 1 Вт (TSU) |

> **Примечание:** Это сравнение архитектур, не коммерческая оценка. Мы не тестировали реальное железо Extropic — у нас нет доступа к Z1. Репозиторий служит математическим базлайном для понимания физики, которую такие устройства реализуют в кремнии.

## Architecture

### 1. Энергометрика — Joules per sample
`benchmark_power.sh` измеряет потребление батареи через `termux-battery-status` до/во время/после симуляции. Это позволяет оценить "расход топлива" алгоритмов.

### 2. Физическая энтропия
`hotstart.sh` использует `/dev/urandom` (SoC HWRNG) для сидирования. При наличии `termux-sensor` — добавляет джиттер акселерометра.

### 3. ASCII-визуализация
`ising_viz.c` + `ascii_viz.sh` — Unicode-блоки █/░ показывают рост магнитных доменов в реальном времени.

### 4. Finite-Size Scaling
`multi_L_scan.sh` + `fss_analysis.sh` — сравнение фазовых переходов для L=32, 64, 128.

## Roadmap

- [x] 2D/3D Ising, Heisenberg (CPU / ARM64 NEON)
- [x] Wolff cluster algorithm
- [x] OpenCL GPU wrapper (checkerboard)
- [x] Joules per sample benchmark
- [x] Hardware entropy seeding
- [x] ASCII domain visualization
- [x] Finite-Size Scaling analysis
- [ ] PPM image export для анимации доменов
- [ ] Hybrid CPU-GPU Wolff
- [ ] Energy-Based Model (EBM) API compatibility

## Сборка

```bash
make
```

Если OpenCL не установлен, `gpu_app` пропустится:
```bash
pkg install ocl-icd opencl-headers
```

## Запуск

```bash
# 2D Ising
./ising_app 64 2.269 10000000

# Wolff (кластерный алгоритм, быстрее около Tc)
./wolff_app 64 2.269 5000

# 3D Ising
./ising3d_app 16 4.51 2000000

# Heisenberg (непрерывные спины на сфере S^2)
./heisenberg_app 16 1.44 2000000

# GPU (экспериментально)
./gpu_app 64 2.269 1000000

# Сравнение алгоритмов
./compare_algos.sh 64 2.269

# Полный бенчмарк
./benchmark.sh
```

## Git

```bash
git add .
git commit -m "feat: your message"
git push
```

## Limitations

- `termux-sensor` недоступен в Play Store-версии Termux (см. [termux-play-store#29](https://github.com/termux-play-store/termux-apps/issues/29))
- GPU OpenCL зависит от vendor-драйверов; на многих устройствах недоступен из Termux
- Для L=128 и выше рекомендуется Wolff-алгоритм (Metropolis страдает от critical slowing down)
