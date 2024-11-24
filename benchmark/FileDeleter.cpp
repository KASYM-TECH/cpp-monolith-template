#include <iostream>
#include <windows.h>

void deleteFile(const std::string& filename) {
    if (DeleteFileA(filename.c_str())) {
        //std::cout << "File '" << filename.c_str() << "' deleted successfully.\n";
    } else {
        std::cerr << "Failed to delete file '" << filename.c_str() << "'. Error: " << GetLastError() << "\n";
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <file_name>\n";
        return 1;
    }

    std::string filename = argv[1];
    deleteFile(filename);
    return 0;
}