#!/usr/bin/env python3
"""
benchmark.py

Скрипт компилирует и запускает C++ программу с разными флагами компиляции,
собирает время выполнения и сохраняет метрики в CSV.
"""
import subprocess
import argparse
import csv
import itertools
from typing import List, Tuple


def compile_program(source: str, output: str,
                    matrix_size: int, threads: int, container: int) -> bool:
    """Компилирует программу с заданными флагами."""
    compile_cmd = [
        "g++", "-std=c++20", f"-DMATRIX_SIZE={matrix_size}",
        f"-DNTHREADS={threads}", f"-DTHREAD_CONTAINER={container}",
        "-O2",  # Оптимизация
        "-o", output, source
    ]

    print(f"Compiling: {' '.join(compile_cmd)}")
    result = subprocess.run(compile_cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print("Compilation failed:")
        print(result.stderr)
        return False
    return True


def run_program(executable: str) -> Tuple[bool, float]:
    """Запускает программу и парсит её вывод, возвращая время."""
    try:
        result = subprocess.run([f"./{executable}"], capture_output=True, text=True, timeout=600)
        if result.returncode != 0:
            print("Execution failed:")
            print(result.stderr)
            return False, 0.0

        for line in result.stdout.splitlines():
            if "Best calculations took" in line:
                time_str = line.strip().split()[-2]
                return True, float(time_str)
        return False, 0.0
    except subprocess.TimeoutExpired:
        print("Timeout.")
        return False, 0.0


def main() -> None:
    parser = argparse.ArgumentParser(description="Benchmark C++ program with various flags.")
    parser.add_argument("--source", type=str, default="task1.cpp", help="C++ исходник")
    parser.add_argument("--output", type=str, default="task1", help="Имя исполняемого файла")
    parser.add_argument("--trials", type=int, default=5, help="Количество повторений на конфигурацию")
    parser.add_argument("--csv", type=str, default="results.csv", help="Выходной CSV файл")

    args = parser.parse_args()

    # Жёстко заданные значения
    matrix_sizes = [20000, 40000]
    threads_list = [1, 2, 4, 7, 8, 16, 20, 40]
    containers = [1, 2, 3, 4, 5]

    with open(args.csv, mode="w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(["MatrixSize", "Threads", "Container", "AvgTime_ms"])

        for matrix_size, threads, container in itertools.product(matrix_sizes, threads_list, containers):
            print(f"\nTesting: MATRIX_SIZE={matrix_size}, NTHREADS={threads}, CONTAINER={container}")

            if not compile_program(args.source, args.output, matrix_size, threads, container):
                continue

            times: List[float] = []
            for trial in range(args.trials):
                print(f"Trial {trial + 1}...", end=' ')
                success, elapsed = run_program(args.output)
                if success:
                    print(f"{elapsed:.4f} ms")
                    times.append(elapsed)
                else:
                    print("Failed.")
                    break

            if times:
                avg_time = sum(times) / len(times)
                writer.writerow([matrix_size, threads, container, round(avg_time, 4)])
            else:
                print("Не удалось получить результаты для этой конфигурации.")


if __name__ == "__main__":
    main()
