#ifndef chip8_h
#define chip8_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>

typedef uint8_t u8;
typedef uint16_t u16;

#define WHITE 0xFFFFFF
#define BLACK 0x0

#define SAMPLE_RATE 44100
#define AMPLITUDE 28000

#define INIT_PC 0x200

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define ROWS 32
#define COLS 64

struct SDL_Rect_Pixel : SDL_Rect { u8 status; };

class CHIP8 {
public:
    void init();
    void loadProgram(u8* block, u16 size);
    void runProgram();
    void close();
private:
    void clearScreen();
    void updateScreen();
    bool tick();
    bool execInst(u16 op);
    
    int freq = 500; // in Hz
    
    u8 M[4096] = { 0 };
    u8 V[16] = { 0 };
    
    u16 stack[16] = { 0 };
    
    u16 I;
    
    u8 DT;
    u8 ST;
    
    u16 PC;
    
    u16 SP;
    
    const u16 pixelWidth = SCREEN_WIDTH / COLS;
    const u16 pixelHeight = SCREEN_HEIGHT / ROWS;
    
    // The window we'll be rendering to
    SDL_Window* window = NULL;
        
    // The surface contained by the window
    SDL_Surface* screenSurface = NULL;

    // Pixel rects
    SDL_Rect_Pixel* pixels = new SDL_Rect_Pixel[ROWS * COLS];
    
    u16 programSize;
    
    SDL_Scancode keymap[16] = {
        SDL_SCANCODE_0,
        SDL_SCANCODE_1,
        SDL_SCANCODE_2,
        SDL_SCANCODE_3,
        SDL_SCANCODE_4,
        SDL_SCANCODE_5,
        SDL_SCANCODE_6,
        SDL_SCANCODE_7,
        SDL_SCANCODE_8,
        SDL_SCANCODE_9,
        SDL_SCANCODE_A,
        SDL_SCANCODE_B,
        SDL_SCANCODE_C,
        SDL_SCANCODE_D,
        SDL_SCANCODE_E,
        SDL_SCANCODE_F
    };
};

#endif
