// Copyright 2024 KASYM-TECH

#include <iostream>
#include <string>

const size_t BLOCK_SIZE = 8192;

__declspec(dllimport) ssize_t __cdecl lab2_open(const char *path);
__declspec(dllimport) ssize_t __cdecl lab2_close(int fd);
__declspec(dllimport) ssize_t __cdecl lab2_read(int fd, unsigned char *buf, size_t count);
__declspec(dllimport) ssize_t __cdecl lab2_write(int fd, unsigned char *buf, size_t count);
__declspec(dllimport) off_t __cdecl lab2_lseek(int fd, off_t offset);
__declspec(dllimport) ssize_t __cdecl lab2_fsync(int fd);
__declspec(dllimport) void __cdecl reportLeaks();

bool searchInBuffer(const int* buffer, size_t intCount, int target) {
    for (size_t i = 0; i < intCount; ++i) {
        if (buffer[i] == target) {
            return true;
        }
    }
    return false;
}

void searchIntegerInFile(const std::string& filename, int target) {
    int fd = lab2_open(filename.c_str());
    if (fd < 0) {
        return;
    }

    const size_t bufferIntCount = BLOCK_SIZE / sizeof(int);
    int* buffer = reinterpret_cast<int*>(_aligned_malloc(BLOCK_SIZE, 4096));
    if (!buffer) {
        lab2_close(fd);
        return;
    }

    bool found = false;
    ssize_t bytesRead = 0;
    off_t offset = 0;

    while (true) {
        bytesRead = lab2_read(fd, reinterpret_cast<unsigned char*>(buffer), BLOCK_SIZE);

        if (bytesRead != 0) {
            break;
        }

        size_t intCount = BLOCK_SIZE / sizeof(int);
        if (searchInBuffer(buffer, intCount, target)) {
            found = true;
            break;
        }

        offset += BLOCK_SIZE;
    }

    if (!found) {
        std::cout << "The integer " << target << " was not found in the file.\n";
    } else {
        std::cout << "The integer " << target << " was found in the file.\n";
    }

    _aligned_free(buffer);
    lab2_close(fd);
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
