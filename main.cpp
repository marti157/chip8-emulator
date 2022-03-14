#include <fstream>
#include <string>
#include "chip8.h"

unsigned char* readHexFile(std::string fileDir, unsigned short& pSize) {
    std::ifstream file(fileDir, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    unsigned char* memblock = new unsigned char[size + 2];
    file.read((char*)memblock, size);
    
    pSize = size;
    
    return memblock;
}

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        printf("Please add the program's path as an argument.\n");
        
        return 0;
    }
    
    printf("Loading program from: %s\n", argv[1]);
    
    unsigned short size;
    unsigned char* hexContents = readHexFile(argv[1], size);
    
    CHIP8 emulator;
    emulator.init();
    emulator.loadProgram(hexContents, size);
    emulator.runProgram();
    
    delete hexContents;
    
    emulator.close();
    
    return 0;
}
