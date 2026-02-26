#include <SDL.h>
#include <SDL_events.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define RAM_SIZE (4096)
#define STACK_SIZE (16)
#define DISPLAY_WIDTH (64)
#define DISPLAY_HEIGHT (32)
#define DISPLAY_SIZE ((64) * (32))

#define internal static
#define global static

typedef enum {
    // General purpose 8-bit register
    REG_V0 = 0,
    REG_V1,
    REG_V2,
    REG_V3,
    REG_V4,
    REG_V5,
    REG_V6,
    REG_V7,
    REG_V8,
    REG_V9,
    REG_VA,
    REG_VB,
    REG_VC,
    REG_VD,
    REG_VE,

    REG_VF, // NOTE: Should not be used by any program

    /* "Pseudo-registers" */
    REG_DT,
    REG_ST,

    /* Stack Pointer register */
    REG_SP,
    REG_COUNT,
} reg;

typedef enum {
    KEY_01,
    KEY_02,
    KEY_03,
    KEY_04,
    KEY_05,
    KEY_06,
    KEY_07,
    KEY_08,
    KEY_09,
    KEY_0A,
    KEY_0B,
    KEY_0C,
    KEY_0D,
    KEY_0E,
    KEY_0F,

    KEY_COUNT,
} keys;

// TODO: Create a struct
global uint8_t memory[RAM_SIZE];
global uint16_t *stack[STACK_SIZE];
global uint8_t regs[REG_COUNT];
global uint8_t display[DISPLAY_SIZE];
global uint8_t keypad[KEY_COUNT];
global uint16_t *reg_pc;
global uint16_t reg_i;
global uint8_t stack_top;

// static uint8_t keypad[] = {
//     0x01, 0x02, 0x03, 0x0C,
//     0x04, 0x05, 0x06, 0x0D,
//     0x07, 0x08, 0x09, 0x0E,
//     0x0A, 0x00, 0x0B, 0x0F,
// };

global uint8_t sprites[80] = {
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
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

// TODO: Create a decent file API
internal uint8_t *
read_file(const char *filename, uint32_t *file_size) {
    FILE *file = fopen(filename, "rb");
    fseek(file, 0L, SEEK_END);
    const long size = ftell(file) - 1;
    fseek(file, 0L, SEEK_SET);
    *file_size = size - 1;
    fclose(file);

    file = fopen(filename, "r");
    uint8_t *content = malloc(size);
    memset(content, '\0', size);
    fread(content, sizeof(uint8_t), size - 1, file);
    fclose(file);

    return content;
}

// TODO: Write to file
// internal void
// debug_memory(uint16_t n) {
//    /*
//    printf("0x%04x: ", 0);
//    for (uint64_t i = 0; i < RAM_SIZE - 1; i += 2)
//    {
//        if (i % 16 == 0 && i != 0)
//        {
//            printf("\n");
//            printf("0x%04lx: ", i);
//        }
//        printf("0x%02X%02X ", memory[i], memory[i + 1]);
//    }
//    printf("\n");
//    */
//    for (uint16_t i = 0x200; i < 0x200 + n - 1; i += 2) {
//        const uint8_t msb = memory[i];
//        const uint8_t lsb = memory[i + 1];
//        const uint16_t instr = (msb << 8) | lsb;
//        const uint16_t nnn_addr = instr & 0x0FFF;
//        const uint8_t nibble = instr & 0x000F;
//        const uint8_t x = (uint8_t)(instr & 0x0F00);
//        const uint8_t y = (uint8_t)(instr & 0x00F0);
//        const uint8_t kk_byte = (uint8_t)(instr & 0x00FF);
//
//        if ((instr & 0x0FFF) == nnn_addr)
//            printf("0x%04X: 0x%04X\n", i, instr);
//    }
//}

internal void
instruction_exec(uint32_t size) {
    for (reg_pc = (uint16_t *)(memory + 0x200);
         reg_pc < (uint16_t *)(memory + 0x200 + size);) {
        const uint8_t msb = (uint8_t)(*reg_pc & 0x00FF);
        const uint8_t lsb = (uint8_t)((*reg_pc & 0xFF00) >> 8);
        const uint16_t instr = (msb << 8) | lsb;

        const uint16_t nnn_addr = instr & 0x0FFF;
        const uint8_t nibble = (uint8_t)(instr & 0x000F);
        const uint8_t x = (instr & 0x0F00) >> 8;
        const uint8_t y = (instr & 0x00F0) >> 4;
        const uint8_t kk_byte = (uint8_t)(instr & 0x00FF);
        const uint8_t top_nibble = (instr & 0xF000) >> 12;

        switch (top_nibble) {
            case 0x00: {
                switch (instr) {
                    case 0x00E0: {
                        // TODO: Clear the display
                        puts("Clear display.");
                        reg_pc++;
                    } break;
                    case 0x00EE: {
                        puts("Return from a subroutine.");
                        reg_pc = stack[stack_top--];
                    } break;
                }
            } break;
            case 0x01: {
                printf("Jump to location: %hd.\n", nnn_addr);
                reg_pc = (uint16_t *)(&memory[nnn_addr]);
            } break;
            case 0x02: {
                printf("Call subroutine at: %hd.\n", nnn_addr);
                stack[++stack_top] = reg_pc;
                reg_pc = (uint16_t *)(&memory[nnn_addr]);
            } break;
            case 0x03: {
                printf("Skip next instruction if V%d == %hd.\n", x, kk_byte);
                if (regs[x] == kk_byte)
                    reg_pc += 2;
                else
                    ++reg_pc;
            } break;
            case 0x04: {
                printf("Skip next instruction if V%d != %hd.\n", x, kk_byte);
                if (regs[x] != kk_byte)
                    reg_pc += 2;
                else
                    ++reg_pc;
            } break;
            case 0x05: {
                printf("Skip next instruction if V%d == V%d.\n", x, y);
                if (regs[x] == regs[y])
                    reg_pc += 2;
                else
                    ++reg_pc;
            } break;
            case 0x06: {
                printf("Set V%d = %hd.\n", x, kk_byte);
                regs[x] = kk_byte;
                reg_pc++;
            } break;
            case 0x07: {
                printf("Set V%d = V%d + %hd.\n", x, x, kk_byte);
                regs[x] += kk_byte;
                reg_pc++;
            } break;
            case 0x08: {
                switch (nibble) {
                    case 0x00: {
                        printf("Set V%d = V%d.\n", x, y);
                        regs[x] = regs[y];
                        reg_pc++;
                    } break;
                    case 0x01: {
                        printf("Set V%d = V%d OR V%d.\n", x, x, y);
                        regs[x] |= regs[y];
                        reg_pc++;
                    } break;
                    case 0x02: {
                        printf("Set V%d = V%d AND V%d.\n", x, x, y);
                        regs[x] &= regs[y];
                        reg_pc++;
                    } break;
                    case 0x03: {
                        printf("Set V%d = V%d XOR V%d.\n", x, x, y);
                        regs[x] ^= regs[y];
                        reg_pc++;
                    } break;
                    case 0x04: {
                        printf("Set V%d = V%d + V%d, set VF = carry.\n", x, x,
                               y);
                        const uint8_t v1 = regs[x];
                        const uint8_t v2 = regs[y];
                        const uint16_t result = v1 + v2;

                        if (result > 255)
                            regs[REG_VF] = 1;
                        else
                            regs[REG_VF] = 0;

                        regs[x] = (result & 0x00FF);
                        reg_pc++;
                    } break;
                    case 0x05: {
                        printf("Set V%d = V%d - V%d, set VF = NOT "
                               "borrow.\n",
                               x, x, y);
                        if (regs[x] > regs[y])
                            regs[REG_VF] = 1;
                        else
                            regs[REG_VF] = 0;

                        regs[x] = regs[x] - regs[y];
                        reg_pc++;
                    } break;
                    case 0x06: {
                        printf("Set V%d = V%d SHR 1.\n", x, x);
                        if (top_nibble & 0x08)
                            regs[REG_VF] = 1;
                        else
                            regs[REG_VF] = 0;

                        regs[x] /= 2;
                        reg_pc++;
                    } break;
                    case 0x07: {
                        printf("Set V%d = V%d - V%d, set VF = NOT "
                               "borrow.\n",
                               x, y, x);
                        if (regs[x] > regs[y])
                            regs[REG_VF] = 1;
                        else
                            regs[REG_VF] = 0;

                        regs[x] = regs[y] - regs[x];
                        reg_pc++;
                    } break;
                    case 0x0E: {
                        printf("Set V%d = V%d SHL 1.\n", x, x);
                        if (nibble & 0x01)
                            regs[REG_VF] = 1;
                        else
                            regs[REG_VF] = 0;

                        regs[x] *= 2;
                        reg_pc++;
                    } break;
                }
            } break;
            case 0x09: {
                printf("Skip next instruction if V%d != V%d.\n", x, y);
                if (regs[x] != regs[y])
                    reg_pc += 2;
                else
                    ++reg_pc;
            } break;
            case 0x0A: {
                printf("Set I = %hd.\n", nnn_addr);
                reg_i = nnn_addr;
                reg_pc++;
            } break;
            case 0x0B: {
                printf("Jump to location %hd + V0.\n", nnn_addr);
                reg_pc = (uint16_t *)(memory + nnn_addr + regs[0]);
            } break;
            case 0x0C: {
                printf("Set V%d = random byte AND %hd.\n", x, kk_byte);
                uint8_t nr = rand() % 255;
                regs[x] = nr & kk_byte; // TODO: Swap bitwise and with the
                                        // CHIP8 AND instr
                reg_pc++;
            } break;
            case 0x0D: {
                printf("Display %d-byte sprite starting at memory location "
                       "I at (%d, "
                       "%d), set VF = collision.\n",
                       (uint32_t)nibble, regs[x], regs[y]);
                // NOTE: Most important instruction
                // TODO: Implement

                const uint8_t x_coord = regs[x] % 64;
                const uint8_t y_coord = regs[y] % 32;

                for (uint16_t i = 0; i < nibble; i++) {
                    const uint8_t byte = memory[reg_i + i];
                }

                regs[REG_VF] = 0;
                reg_pc++;
            } break;
            case 0x0E: {
                switch (kk_byte) {
                    case 0x9E: {
                        printf("Skip next instruction if key with the "
                               "value of V%d is "
                               "pressed.\n",
                               x);
                        if (keypad[x])
                            reg_pc += 2;
                        else
                            ++reg_pc;
                    } break;
                    case 0xA1: {
                        printf("Skip next instruction if key with the "
                               "value of V%d "
                               "is not "
                               "pressed.\n",
                               x);
                        if (!keypad[x])
                            reg_pc += 2;
                        else
                            ++reg_pc;
                    } break;
                }
            } break;
            case 0x0F: {
                switch (kk_byte) {
                    case 0x07: {
                        printf("Set V%d = delay timer value.\n", x);
                        regs[x] = regs[REG_DT];
                        reg_pc++;
                    } break;
                    case 0x0A: {
                        printf("Wait for a key press, store the value "
                               "of the key "
                               "in V%d.\n",
                               x);
                        uint8_t pressed = 0;
                        // TODO: Implement
                        reg_pc++;
                    } break;
                    case 0x15: {
                        printf("Set delay timer = V%d.\n", x);
                        regs[REG_DT] = regs[x];
                        reg_pc++;
                    } break;
                    case 0x18: {
                        printf("Set sound timer = V%d.\n", x);
                        regs[REG_ST] = regs[x];
                        reg_pc++;
                    } break;
                    case 0x1E: {
                        printf("Set I = I + V%d.\n", x);
                        reg_i = reg_i + regs[x];
                        reg_pc++;
                    } break;
                    case 0x29: {
                        printf("Set I = location of sprite for digit "
                               "V%d.\n",
                               x);
                        reg_i = *((uint16_t *)(&memory[x * 5]));
                        reg_pc++;
                    } break;
                    case 0x33: {
                        printf("Store BCD representation of V%d in "
                               "memory location "
                               "I, I + 1 "
                               "and I + 2.\n",
                               x);
                        uint8_t digit = regs[x];
                        for (int8_t i = 2; i >= 0; i--) {
                            uint8_t nr = digit % 10;
                            *(uint16_t *)(memory + reg_i + i) = nr;
                            digit /= 10;
                        }
                        reg_pc++;
                    } break;
                    case 0x55: {
                        printf("Store registers V0 through V%d in "
                               "memory starting "
                               "at location "
                               "I.\n",
                               x);
                        for (uint64_t i = 0; i < x; i++) {
                            memory[reg_i + i] = regs[i];
                        }
                        reg_pc++;
                    } break;
                    case 0x65: {
                        printf("Read registers V0 through V%d in "
                               "memory starting "
                               "at location "
                               "I.\n",
                               x);
                        for (uint64_t i = 0; i < x; i++) {
                            regs[i] = memory[reg_i + i];
                        }
                        reg_pc++;
                    } break;
                }
            } break;
        }

        // printf("0x%04hX\n", instr);
    }
}

internal void
init(void) {
    memcpy(memory, sprites, sizeof sprites);
}

int
main(int argc, const char *argv[]) {
    (void)argc;
    (void)argv;
    // TODO: Add terminal support
    //  if (argc < 2)
    //  {
    //      printf("Usage: chip8 [chip8 program]\n");
    //      return 1;
    //  }
    //
    srand((uint32_t)time(NULL));
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "Chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        DISPLAY_WIDTH * 20, DISPLAY_HEIGHT * 20, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    printf("video driver: %s\n", SDL_GetCurrentVideoDriver());
    uint8_t running = 1;

    SDL_Rect rect = {
        .x = 10,
        .y = 20,
        .w = 100,
        .h = 200,
    };

    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = 0;
        }

        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 255, 245, 120, 255);
        SDL_RenderDrawRect(renderer, &rect);
        SDL_RenderPresent(renderer);

        SDL_Delay(1);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    // uint32_t size = 0;
    // uint8_t *program = read_file("../../assets/roms/IBM Logo.ch8", &size);

    // memcpy(&memory[512], program, size);
    // free(program);

    // init();
    //  debug_memory((uint16_t)size);
    //  instruction_exec(size);
    return 0;
}
