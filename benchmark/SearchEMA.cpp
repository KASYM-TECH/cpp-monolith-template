#include <windows.h>

#include <iostream>
#include <string>

const size_t BLOCK_SIZE = 8192;

bool searchInBuffer(const int* buffer, size_t intCount, int target) {
  for (size_t i = 0; i < intCount; ++i) {
    if (buffer[i] == target) {
      return true;
    }
  }
  return false;
}

void searchIntegerInFile(const std::string& filename, int target) {
  HANDLE file =
      CreateFileA(filename.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING,
                  FILE_FLAG_WRITE_THROUGH | FILE_FLAG_NO_BUFFERING, NULL);

  if (file == INVALID_HANDLE_VALUE) {
    std::cerr << "Failed to open file. Error: " << GetLastError() << "\n";
    return;
  }

  const size_t bufferIntCount = BLOCK_SIZE / sizeof(int);
  int* buffer = (int*)_aligned_malloc(BLOCK_SIZE, 4096);
  if (!buffer) {
    std::cerr << "Failed to allocate memory for the buffer.\n";
    CloseHandle(file);
    return;
  }

  DWORD bytesRead = 0;
  bool found = false;
  int blocks_read = 0;

  while (true) {
    if (!ReadFile(file, buffer, BLOCK_SIZE, &bytesRead, NULL)) {
      std::cerr << "Failed to read data. Error: " << GetLastError() << "\n";
      break;
    }

    if (bytesRead == 0) break;

    size_t intCount = bytesRead / sizeof(int);
    if (searchInBuffer(buffer, intCount, target)) {
      found = true;
      std::cout << "Found the integer " << target << " in the file.\n";
      // std::cout << "Block index: " << blocks_read << " in the file.\n";
      break;
    }
    blocks_read++;
  }

  if (!found) {
    std::cout << "The integer " << target << " was not found in the file.\n";
  }

  _aligned_free(buffer);
  CloseHandle(file);
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <filename> <integer_to_find>\n";
    return 1;
  }

  std::string filename = argv[1];
  int target = std::stoi(argv[2]);

  searchIntegerInFile(filename, target);

  return 0;
}