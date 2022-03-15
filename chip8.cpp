#include "chip8.h"

void audioCallback(void *user_data, Uint8 *raw_buffer, int bytes) {
    Sint16 *buffer = (Sint16*)raw_buffer;
    int length = bytes / 2; // 2 bytes per sample for AUDIO_S16SYS
    int &sample_nr(*(int*)user_data);
    
    for (int i = 0; i < length; i++, sample_nr++) {
        double time = (double)sample_nr / (double)SAMPLE_RATE;
        buffer[i] = (Sint16)(AMPLITUDE * sin(2.0f * M_PI * 659.25f * time)); // render 600 HZ sine wave
    }
}

void CHIP8::init() {
    // Initialization flag
    bool success = true;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        success = false;
    } else {
        // Create window
        window = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        
        if (window == NULL) {
            printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
            success = false;
        } else {
            // Get window surface
            screenSurface = SDL_GetWindowSurface(window);
        }
    }
    
    // Fill the surface blue (background)
    SDL_FillRect(screenSurface, NULL, 0xFF);
    
    // Initialize pixels
    for (int i = 0; i < ROWS * COLS; i++) {
        pixels[i].w = pixelWidth;
        pixels[i].h = pixelHeight;
        
        int row = i / COLS;
        pixels[i].x = (i - row * COLS) * pixelWidth;
        pixels[i].y = row * pixelHeight;
    }
    
    updateScreen();
    
    // Audio (beeper)
    int sample_nr = 0;
    SDL_AudioSpec want, have;
    want.freq = SAMPLE_RATE; // number of samples per second
    want.format = AUDIO_S16SYS; // sample type (here: signed short i.e. 16 bit)
    want.channels = 1; // only one channel
    want.samples = 2048; // buffer-size
    want.callback = audioCallback; // function SDL calls periodically to refill the buffer
    want.userdata = &sample_nr; // counter, keeping track of current sample number
    
    if (SDL_OpenAudio(&want, &have) < 0)
        SDL_LogError(SDL_LOG_CATEGORY_AUDIO, "Failed to open audio: %s", SDL_GetError());
    
    // Fonts
    u8 fonts[80] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    
    memcpy(&M[0], &fonts, 80);
}

void CHIP8::clearScreen() {
    for (int i = 0; i < ROWS * COLS; i++) {
        pixels[i].status = 0;
    }
    
    updateScreen();
}

void CHIP8::updateScreen() {
    for (int i = 0; i < ROWS * COLS; i++) {
        SDL_FillRect(screenSurface, &pixels[i], pixels[i].status ? WHITE : BLACK);
    }
}

void CHIP8::loadProgram(u8* block, u16 size) {
    programSize = size;
    
    memcpy(&M[INIT_PC], block, size);
}

void CHIP8::runProgram() {
    PC = INIT_PC;
    SP = 0;
    DT = 0;
    ST = 0;
    I = 0;
    
    // Some programs don't clear the screen!
    clearScreen();
    
    int currTick;
    int prevTick = -1;
    SDL_Event e;
    bool quit = false;
    
    while (!quit) {
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        
        currTick = SDL_GetTicks();
        if (currTick != prevTick) {
            prevTick = currTick;
            
            if (currTick % (1000 / freq) == 0) {
                if (!tick())
                    break;
            }
            
            if (currTick % 16 == 0) {
                // Must keep rendering pixels
                SDL_UpdateWindowSurface(window);
                
                // Delay and sound decrease at 60Hz
                if (DT > 0) {
                    DT--;
                }
                
                if (ST > 0) {
                    ST--;
                    if (ST == 0)
                        SDL_PauseAudio(1); // stop playing sound
                }
            }
        }
    }
}

bool CHIP8::tick() {
    if (PC > INIT_PC + programSize)
        return false; // Reached end of program
    
    u16 op = M[PC] << 8 | M[PC + 1];
    
    if (DEBUG_MSG)
        printf("inst: %x, PC: %x, time: %d\n", op, PC, SDL_GetTicks());
    
    if (!execInst(op))
        return false;
    
    PC += 2;
    return true;
}

bool CHIP8::execInst(u16 op) {
    switch (op) {
        case 0xE0: // Clear screen
            clearScreen();
            break;
        case 0xEE: // Return from a subroutine
            if (SP == 0) {
                printf("No address to return to\n");
                return false;
            }
            PC = stack[--SP];
            break;
        default:
            u8 x = op >> 8 & 0xF;
            u8 y = op >> 4 & 0xF;
            
            switch(op >> 12) {
                case 0x1: // Jump to location nnn
                    PC = (op & 0xFFF) - 2;
                    break;
                case 0x2: // Call subroutine at nnn
                    if (SP >= 0xF) {
                        printf("Stack overflow\n");
                        return false;
                    }
                    stack[SP++] = PC;
                    PC = (op & 0xFFF) - 2;
                    break;
                case 0x3: // Skip next instruction if Vx = kk
                    if (V[x] == (op & 0xFF))
                        PC += 2;
                    break;
                case 0x4: // Skip next instruction if Vx != kk
                    if (V[x] != (op & 0xFF))
                        PC += 2;
                    break;
                case 0x5: // Skip next instruction if Vx = Vy.
                    if (V[x] == V[y])
                        PC += 2;
                    break;
                case 0x6: // Set Vx = kk
                    V[x] = op & 0xFF;
                    break;
                case 0x7: // Set Vx = Vx + kk
                    V[x] += op & 0xFF;
                    break;
                case 0x8:
                    switch (op & 0xF) {
                        case 0: // Set Vx = Vy
                            V[x] = V[y];
                            break;
                        case 1: // Set Vx = Vx OR Vy
                            V[x] = V[x] | V[y];
                            break;
                        case 2: // Set Vx = Vx AND Vy
                            V[x] = V[x] & V[y];
                            break;
                        case 3: // Set Vx = Vx XOR Vy
                            V[x] = V[x] ^ V[y];
                            break;
                        case 4: // Set Vx = Vx + Vy, set VF = carry
                            V[0xF] = V[x] + V[y] > 0xFF;
                            V[x] = V[x] + V[y];
                            break;
                        case 5: // Set Vx = Vx - Vy, set VF = NOT borrow
                            V[0xF] = V[x] > V[y];
                            V[x] = V[x] - V[y];
                            break;
                        case 6: // Set Vx = Vx SHR 1
                            V[0xF] = V[x] & 0xF;
                            V[x] = V[x] >> 1;
                            break;
                        case 7: // Set Vx = Vy - Vx, set VF = NOT borrow
                            V[0xF] =  V[y] > V[x];
                            V[x] = V[y] - V[x];
                            break;
                        case 0xE: // Set Vx = Vx SHL 1
                            V[0xF] = V[x] & 0xF;
                            V[x] = V[x] << 1;
                            break;
                        default:
                            printf("Unknown OPcode: %x\n", op);
                    }
                    break;
                case 0x9: // Skip next instruction if Vx != Vy
                    if (V[x] != V[y])
                        PC += 2;
                    break;
                case 0xA: // Set I = nnn
                    I = op & 0xFFF;
                    break;
                case 0xB: // Jump to location nnn + V0
                    PC = (op & 0xFFF) + V[0];
                    break;
                case 0xC: // Set Vx = random byte AND kk
                    V[x] = (rand() % 255) & (op & 0xFF);
                    break;
                case 0xD: { // Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
                    V[0xF] = 0;
                    for (u8 i = 0; i < (op & 0xF); i++) {
                        for (int8_t j = 7; j >= 0; j--) {
                            u16 pos = (V[y] + i) % ROWS * COLS + (V[x] + 7 - j) % COLS;
                            u8 val = (M[I + i] >> j) & 1;
                            if ((pixels[pos].status ^ val) != pixels[pos].status)
                                V[0xF] = 1;
                            pixels[pos].status ^= val;
                        }
                    }
                    if (V[0xF] != 0)
                        updateScreen();
                    break;
                }
                case 0xE:
                    switch (op & 0xFF) {
                        case 0x9E: // Skip next instruction if key with the value of Vx is pressed
                            if (SDL_GetKeyboardState(NULL)[keymap[V[x]]])
                                PC += 2;
                            break;
                        case 0xA1: // Skip next instruction if key with the value of Vx is not pressed.
                            if (!SDL_GetKeyboardState(NULL)[keymap[V[x]]])
                                PC += 2;
                            break;
                        default:
                            printf("Unknown OPcode: %x\n", op);
                    }
                    break;
                case 0xF:
                    switch (op & 0xFF) {
                        case 0x7: // Set Vx = delay timer value
                            V[x] = DT;
                            break;
                        case 0xA: { // Wait for a key press, store the value of the key in Vx
                            SDL_Event e;
                            u8 found = 0;
                            while (!found) {
                                while (SDL_PollEvent(&e) != 0) {
                                    if (e.type == SDL_KEYDOWN) {
                                        for (u8 i = 0; i < 16; i++) {
                                            if (keymap[i] == e.key.keysym.scancode) {
                                                V[x] = i;
                                                found = 1;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            break;
                        }
                        case 0x15: // Set delay timer = Vx
                            DT = V[x];
                            break;
                        case 0x18: // Set sound timer = Vx
                            ST = V[x];
                            if (ST != 0)
                                SDL_PauseAudio(0); // play sound
                            else
                                SDL_PauseAudio(1); // stop sound
                            break;
                        case 0x1E: // Set I = I + Vx
                            I = I + V[x];
                            break;
                        case 0x29: // Set I = location of sprite for digit Vx
                            I = V[x] * 5;
                            break;
                        case 0x33: // Store BCD representation of Vx in memory locations I, I+1, and I+2
                            M[I] = V[x] / 100;
                            M[I + 1] = (V[x] / 10) % 10;
                            M[I + 2] = V[x] % 10;
                            break;
                        case 0x55: // Store registers V0 through Vx in memory starting at location I
                            for (u8 i = 0; i <= x; i++) {
                                M[I + i] = V[i];
                            }
                            break;
                        case 0x65: // Read registers V0 through Vx from memory starting at location I
                            for (u8 i = 0; i <= x; i++) {
                                V[i] = M[I + i];
                            }
                            break;
                        default:
                            printf("Unknown OPcode: %x\n", op);
                    }
                    break;
                default:
                    printf("Unknown OPcode: %x\n", op);
            }
    }
    return true;
}

void CHIP8::close() {
    // Destroy pixels
    delete[] pixels;
    
    // Destroy audio
    SDL_CloseAudio();
    
    // Destroy window
    SDL_DestroyWindow(window);
    window = NULL;
    
    // Quit SDL subsystems
    SDL_Quit();
}
