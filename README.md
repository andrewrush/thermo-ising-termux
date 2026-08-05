# thermo-ising-termux

Высокопроизводительный **2D симулятор модели Изинга** (метод Монте-Карло / Метрополис) для Termux на Android aarch64.

## Особенности

- **2D решётка** LxL с периодическими граничными условиями
- Инициализация через аппаратный RNG SoC (`/dev/urandom`)
- Быстрый XorShift128+ для шагов симуляции
- **Температура батареи** через `termux-battery-status` (обход SELinux)
- **CLI-параметры**: `./ising_app [L] [T] [steps] [csv]`
- **Промежуточный вывод** каждые 10%
- **Статистика**: `<M>`, `<|M|>`, `<E>`, восприимчивость `chi`, теплоёмкость `C`
- **CSV-режим** для серийных запусков
- **Скрипт сканирования** `scan_phase.sh`
- **Параллельное сканирование** `parallel_scan.sh` (все T одновременно)
- **Визуализация** `plot_phase.sh` через gnuplot (ASCII-графики в терминале)
- **Полный pipeline** `run_all.sh`: scan -> plot -> git commit -> push
- Оптимизация под ARM64 NEON (`-march=armv8-a+simd`)
- Учтены особенности Termux: `gcc` = `clang`, `ldd` -> `readelf`

## Сборка

```bash
make
```

## Проверка зависимостей (Termux-way)

```bash
readelf -d $PWD/ising_app | grep NEEDED
```

## Запуск

```bash
# По умолчанию: L=64, T=2.269, steps=10M
./ising_app

# Пользовательские параметры
./ising_app 128 2.5 50000000

# CSV-режим (тихий, для скриптов)
./ising_app 64 2.269 10000000 csv

# Сканирование фазового перехода
chmod +x scan_phase.sh
./scan_phase.sh 64 5000000

# Параллельное сканирование (быстрее!)
chmod +x parallel_scan.sh
./parallel_scan.sh 64 5000000

# Визуализация (установи gnuplot)
pkg install gnuplot
chmod +x plot_phase.sh
./plot_phase.sh phase_L64.csv

# Полный pipeline: scan + plot + git push
chmod +x run_all.sh
./run_all.sh 64 5000000

# С wakelock (чтобы Android не убил процесс)
termux-wake-lock
./ising_app
termux-wake-unlock
```

## Формат CSV

```
T,<M>,<|M|>,<E>,<M2>,chi,C,accept_pct
```

## Git

```bash
git add .
git commit -m "feat: your message"
git push
```

## Ограничения

- `termux-sensor` недоступен в Play Store-версии Termux
- Для доступа к сенсорам нужна F-Droid-версия Termux + Termux:API
