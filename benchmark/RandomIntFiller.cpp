// Copyright 2024 KASYM-TECH

#include <windows.h>

#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#define BLOCK_SIZE 8192

void writeRandomIntDataWithFlags(const std::string& filename,
                                 size_t totalInts) {
  HANDLE file = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, NULL,
                            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

  if (file == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to create file. Error: " << GetLastError() << "\n";
    return;
  }

  DWORD bytesWritten = 0;
  constexpr size_t INT_PER_BLOCK = BLOCK_SIZE / sizeof(int);

  std::srand(static_cast<unsigned>(std::time(nullptr)));

  for (size_t i = 0; i < totalInts; i += INT_PER_BLOCK) {
    int buffer[INT_PER_BLOCK] = {0};
    size_t intsToGenerate = std::min(INT_PER_BLOCK, totalInts - i);

    for (size_t j = 0; j < intsToGenerate; ++j) {
      buffer[j] = std::rand();  // Generate a random integer
    }

    if (!WriteFile(file, buffer, BLOCK_SIZE, &bytesWritten, NULL)) {
      std::cerr << "Failed to write data. Error: " << GetLastError() << "\n";
      CloseHandle(file);
      return;
    }
  }

  CloseHandle(file);
  // std::cout << "Random integer data written to file successfully.\n";
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <file_name> <number_of_ints>\n";
    return 1;
  }

  std::string filename = argv[1];
  size_t totalInts = std::stoi(argv[2]);
  writeRandomIntDataWithFlags(filename, totalInts);
  return 0;
}
