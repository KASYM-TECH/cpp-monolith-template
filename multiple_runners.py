import os
import struct
import subprocess
import time
from pathlib import Path
import re
from random import randint
from concurrent.futures import ThreadPoolExecutor, as_completed

def compile_sources(output_filename, input_array_cpp_files):
    build_dir = Path("build")
    build_dir.mkdir(exist_ok=True)
    compile_arr = ["g++", "-o", str(build_dir / output_filename)] + input_array_cpp_files
    print(" ".join(compile_arr))
    result = subprocess.run(compile_arr, capture_output=True, text=True)
    if result.returncode != 0:
        print("Compilation error:")
        print(result.stderr)
        exit(1)
    else:
        return True

def run_benchmark(executable_path, input_value):
    try:
        process = subprocess.Popen(
            [str(executable_path), "build/nums.bin", "-1"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        process.stdin.write(input_value + '\n')
        process.stdin.close()
        stdout, stderr = process.communicate()

        if process.returncode != 0:
            return f"Error: {stderr.strip()}"
        return stdout.strip()
    except Exception as e:
        return f"Exception: {str(e)}"

def run_benchmarks_parallel(output_filename, input_data_list):
    executable_path = Path("build") / output_filename
    if not executable_path.exists():
        print(f"Executable file {output_filename} not found.")
        return

    results = []
    with ThreadPoolExecutor(max_workers=20) as executor:
        futures = {
            executor.submit(run_benchmark, executable_path, input_value): input_value
            for input_value in input_data_list
        }
        for future in as_completed(futures):
            try:
                result = future.result()
                results.append(result)
            except Exception as e:
                results.append(f"Error: {str(e)}")
    return results

def main():
    bin_bench1 = "bench_1.exe"

    cpp_bench1 = [str(file) for file in Path("./benchmark").rglob("SearchEMA.cpp")]

    int_to_search = randint(0, 100000)

    input_data_list = [
        f"{randint(0, 100)}"  # Simulate input for each benchmark
        for _ in range(25)  # Create 20 input datasets
    ]

    print(input_data_list)

    if compile_sources(bin_bench1, cpp_bench1):
        results = run_benchmarks_parallel(bin_bench1, input_data_list)
        for i, result in enumerate(results, 1):
            print(f"Result of Benchmark {i}: {result}")

if __name__ == "__main__":
    main()
