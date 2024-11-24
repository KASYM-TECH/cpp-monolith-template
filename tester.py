import os
import struct
import subprocess
import time
from pathlib import Path
import re
from random import randint


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


def run_and_validate(output_filename, input_data, validate_output):
    executable_path = Path("build") / output_filename
    if not executable_path.exists():
        print(f"Executable file {output_filename} not found.")
        return

    for i, input_value in enumerate(input_data):
        input_lines = input_value.splitlines()
        process = subprocess.Popen([str(executable_path)], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                                   stderr=subprocess.PIPE, text=True)
        for line in input_lines:
            process.stdin.write(line + '\n')
            process.stdin.flush()
            time.sleep(0.5)
        process.stdin.close()
        stdout, stderr = process.communicate()
        if process.returncode != 0:
            print(f"Error during test {(i + 1)} with input data: {input_value}")
            print(stderr)
            exit(1)
        validate_output(i + 1, input_value, stdout)


def search_integer_in_bin(file_path, target):
    try:
        with open(file_path, 'rb') as file:
            while chunk := file.read(4):  # Read 4 bytes (size of an integer)
                num = struct.unpack('i', chunk)[0]  # Unpack as a signed integer
                if num == target:
                    return True
        return False
    except FileNotFoundError:
        print(f"File {file_path} not found.")
        return False
    except Exception as e:
        print(f"An error occurred: {e}")
        return False

def main():
    bin_int_filler = "int_filler.exe"
    bin_shell = "shell.exe"
    bin_bench1 = "bench_1.exe"
    bin_bench2 = "bench_2.exe"

    cpp_int_file_filler = [str(file) for file in Path("./benchmark").rglob("RandomIntFiller.cpp")]
    cpp_shell = [str(file) for file in Path("shell").rglob("Shell.cpp")]
    cpp_bench1 = [str(file) for file in Path("./benchmark").rglob("SearchEMA.cpp")]
    cpp_bench2 = [str(file) for file in Path("./benchmark").rglob("DijkstraLoad.cpp")]

    int_to_search = randint(0, 100000)

    input_data = ["cd build\n./" + bin_int_filler + " nums.bin 256000\nexit\n",
                  "cd build\n./" + bin_bench1 + " nums.bin " + str(int_to_search) + "\nexit\n",
                  "cd build\n./" + bin_bench1 + " nums.bin -1\nexit\n",
                  "cd build\n./" + bin_bench2 + " 1000\nexit\n"]

    output_data = ["__PATH__$ __PATH__\\build$ Execution time of ./int_filler.exe: __NUM__ seconds\n__PATH__\\build$ ",
                   "__PATH__$ __PATH__\\build$ " + ("Found the integer " + str(int_to_search) + " in the file." if search_integer_in_bin('./build/nums.bin', int_to_search)
                                                    else "The integer " + str(int_to_search) + " was not found in the file.")
                   + "\nExecution time of ./bench_1.exe: __NUM__ seconds" + "\n__PATH__\\build$ ",
                   "__PATH__$ __PATH__\\build$ The integer -1 was not found in the file.\nExecution time of ./bench_1.exe: __NUM__ seconds\n__PATH__\\build$ ",
                   "__PATH__$ __PATH__\\build$ Execution time of ./bench_2.exe: __NUM__ seconds\n__PATH__\\build$ "]

    def validate_output(test_number, input_value, output_value):
        cout = output_value.replace(os.path.dirname(os.path.abspath(__file__)), "__PATH__")
        cout = re.sub(r"\d+.\d+ seconds", "__NUM__ seconds", cout)
        if cout == output_data[test_number - 1]:
            print(f"Test {test_number} passed")
        else:
            print(
                f"Test {test_number} failed:\nInput data: {input_value}\nResult: {cout}\nExpected: {output_data[test_number - 1]}")
            exit(1)

    if compile_sources(bin_int_filler, cpp_int_file_filler):
        if compile_sources(bin_shell, cpp_shell):
            if compile_sources(bin_bench1, cpp_bench1):
                if compile_sources(bin_bench2, cpp_bench2):
                    run_and_validate(bin_shell, input_data, validate_output)


if __name__ == "__main__":
    main()
