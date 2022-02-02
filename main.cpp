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
    unsigned short size;
    unsigned char* hexContents = readHexFile("examples/space_invaders.ch8", size);
    
    CHIP8 emulator;
    emulator.init();
    emulator.loadProgram(hexContents, size);
    emulator.runProgram();
    
    delete hexContents;
    
    emulator.close();
    
    return 0;
}
