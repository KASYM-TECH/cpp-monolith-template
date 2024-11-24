// Copyright 2024 KASYM-TECH

#include <windows.h>

#include <iostream>
#include <string>

int main() {
  HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);

  if (hStdout == INVALID_HANDLE_VALUE) {
    std::cerr << "Error: Unable to get standard handle." << std::endl;
    return 1;
  }

  const char* message = "Hello from the child process!\n";
  DWORD bytesWritten;
  if (!WriteFile(hStdout, message, strlen(message), &bytesWritten, NULL)) {
    DWORD err = GetLastError();
    fprintf(stderr, "Write operation failed with error code %lu\n", err);
    std::cerr << "Error: Unable to write to standard output." << std::endl;
    return 1;
  }

  return 0;
}