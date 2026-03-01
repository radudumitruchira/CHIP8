#define SDL_MAIN_HANDLED 1
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
#define DISPLAY_SIZE ((DISPLAY_WIDTH) * (DISPLAY_HEIGHT))

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
    KEY_01 = 1,
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
global uint16_t stack[STACK_SIZE];
global uint8_t regs[REG_COUNT];
global uint8_t display[DISPLAY_SIZE];
global uint8_t keypad[KEY_COUNT];
global uint16_t reg_pc = 0x200;
global uint16_t reg_i;
global uint16_t stack_top;

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
    if (!file)
        puts("Error!");
    fseek(file, 0L, SEEK_END);
    const long size = ftell(file) + 1;
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

internal void
instruction_exec(uint32_t size) {
    SDL_assert(reg_pc <= (0x200 + size));

    const uint8_t msb = (uint8_t)(memory[reg_pc]);
    const uint8_t lsb = (uint8_t)(memory[reg_pc + 1]);
    const uint16_t instr = (msb << 8) | lsb;

    reg_pc += 2;

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
                    memset(display, 0, DISPLAY_SIZE);
                    // TODO: Clear the display
                    puts("Clear display.");

                } break;
                case 0x00EE: {
                    puts("Return from a subroutine.");
                    reg_pc = stack[stack_top--];
                } break;
            }
        } break;
        case 0x01: {
            // printf("Jump to location: %hd.\n", nnn_addr);
            reg_pc = nnn_addr;
        } break;
        case 0x02: {
            printf("Call subroutine at: %hd.\n", nnn_addr);
            stack[++stack_top] = reg_pc;
            reg_pc = nnn_addr;
        } break;
        case 0x03: {
            printf("Skip next instruction if V%d == %hd.\n", x, kk_byte);
            if (regs[x] == kk_byte)
                reg_pc += 2;
        } break;
        case 0x04: {
            printf("Skip next instruction if V%d != %hd.\n", x, kk_byte);
            if (regs[x] != kk_byte)
                reg_pc += 2;
        } break;
        case 0x05: {
            printf("Skip next instruction if V%d == V%d.\n", x, y);
            if (regs[x] == regs[y])
                reg_pc += 2;
        } break;
        case 0x06: {
            printf("Set V%d = %hd.\n", x, kk_byte);
            regs[x] = kk_byte;
        } break;
        case 0x07: {
            printf("Set V%d = V%d + %hd.\n", x, x, kk_byte);
            regs[x] += kk_byte;
        } break;
        case 0x08: {
            switch (nibble) {
                case 0x00: {
                    printf("Set V%d = V%d.\n", x, y);
                    regs[x] = regs[y];
                } break;
                case 0x01: {
                    printf("Set V%d = V%d OR V%d.\n", x, x, y);
                    regs[x] |= regs[y];
                } break;
                case 0x02: {
                    printf("Set V%d = V%d AND V%d.\n", x, x, y);
                    regs[x] &= regs[y];
                } break;
                case 0x03: {
                    printf("Set V%d = V%d XOR V%d.\n", x, x, y);
                    regs[x] ^= regs[y];
                } break;
                case 0x04: {
                    printf("Set V%d = V%d + V%d, set VF = carry.\n", x, x, y);
                    const uint8_t v1 = regs[x];
                    const uint8_t v2 = regs[y];
                    const uint16_t result = v1 + v2;

                    if (result > 255)
                        regs[REG_VF] = 1;
                    else
                        regs[REG_VF] = 0;

                    regs[x] = (result & 0x00FF);
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
                } break;
                case 0x06: {
                    printf("Set V%d = V%d SHR 1.\n", x, x);
                    if (top_nibble & 0x08)
                        regs[REG_VF] = 1;
                    else
                        regs[REG_VF] = 0;

                    regs[x] /= 2;
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
                } break;
                case 0x0E: {
                    printf("Set V%d = V%d SHL 1.\n", x, x);
                    if (nibble & 0x01)
                        regs[REG_VF] = 1;
                    else
                        regs[REG_VF] = 0;

                    regs[x] *= 2;
                } break;
            }
        } break;
        case 0x09: {
            printf("Skip next instruction if V%d != V%d.\n", x, y);
            if (regs[x] != regs[y])
                reg_pc += 4;
        } break;
        case 0x0A: {
            printf("Set I = %hd.\n", nnn_addr);
            reg_i = nnn_addr;
        } break;
        case 0x0B: {
            printf("Jump to location %hd + V0.\n", nnn_addr);
            reg_pc = nnn_addr + regs[0];
        } break;
        case 0x0C: {
            printf("Set V%d = random byte AND %hd.\n", x, kk_byte);
            uint8_t nr = rand() % 255;
            regs[x] = nr & kk_byte;
        } break;
        case 0x0D: {
            printf("Display %d-byte sprite starting at memory location "
                   "I at (%d, "
                   "%d), set VF = collision.\n",
                   (uint32_t)nibble, regs[x], regs[y]);

            uint8_t x_coord = regs[x] % DISPLAY_WIDTH;
            uint8_t y_coord = regs[y] % DISPLAY_HEIGHT;
            const uint8_t orig_x = x_coord;

            regs[REG_VF] = 0;

            for (uint8_t i = 0; i < nibble; i++) {
                const uint8_t byte = memory[reg_i + i];
                x_coord = orig_x;

                for (int8_t j = 7; j >= 0; j--) {

                    uint8_t *pixel =
                        &display[y_coord * DISPLAY_WIDTH + x_coord];
                    const uint8_t sprite_bit = (byte & (1 << j));

                    if (sprite_bit && *pixel) {
                        regs[REG_VF] = 1;
                    }

                    *pixel ^= sprite_bit;

                    if (++x_coord >= DISPLAY_WIDTH)
                        break;
                }
                if (++y_coord >= DISPLAY_HEIGHT)
                    break;
            }
        } break;
        case 0x0E: {
            switch (kk_byte) {
                case 0x9E: {
                    printf("Skip next instruction if key with the "
                           "value of V%d is "
                           "pressed.\n",
                           x);
                    if (keypad[regs[x]])
                        reg_pc += 2;
                } break;
                case 0xA1: {
                    printf("Skip next instruction if key with the "
                           "value of V%d "
                           "is not "
                           "pressed.\n",
                           x);
                    if (!keypad[regs[x]])
                        reg_pc += 2;
                } break;
            }
        } break;
        case 0x0F: {
            switch (kk_byte) {
                case 0x07: {
                    printf("Set V%d = delay timer value.\n", x);
                    regs[x] = regs[REG_DT];
                } break;
                case 0x0A: {
                    printf("Wait for a key press, store the value "
                           "of the key "
                           "in V%d.\n",
                           x);
                    uint8_t pressed = 0;
                    for (uint8_t i = 0; i < KEY_COUNT; i++) {
                        if (keypad[i]) {
                            regs[x] = i;
                            pressed = 1;
                            break;
                        }
                    }
                    if (!pressed) {
                        reg_pc -= 2;
                    }
                } break;
                case 0x15: {
                    printf("Set delay timer = V%d.\n", x);
                    regs[REG_DT] = regs[x];
                } break;
                case 0x18: {
                    printf("Set sound timer = V%d.\n", x);
                    regs[REG_ST] = regs[x];
                } break;
                case 0x1E: {
                    printf("Set I = I + V%d.\n", x);
                    reg_i = reg_i + regs[x];
                } break;
                case 0x29: {
                    printf("Set I = location of sprite for digit "
                           "V%d.\n",
                           x);
                    // reg_i = *((uint16_t *)(&memory[x * 5]));
                    reg_i = (uint16_t)((regs[x]) * 5);
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
                        printf("%d\n", nr);
                        *(uint8_t *)(memory + reg_i + i) = nr;
                        digit /= 10;
                    }
                } break;
                case 0x55: {
                    printf("Store registers V0 through V%d in "
                           "memory starting "
                           "at location "
                           "I.\n",
                           x);
                    for (uint64_t i = 0; i <= x; i++) {
                        memory[reg_i + i] = regs[i];
                    }
                } break;
                case 0x65: {
                    printf("Read registers V0 through V%d in "
                           "memory starting "
                           "at location "
                           "I.\n",
                           x);
                    for (uint64_t i = 0; i <= x; i++) {
                        regs[i] = memory[reg_i + i];
                    }
                } break;
            }
        } break;
    }

    // printf("0x%04hX\n", instr);
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

    uint32_t size = 0;
    uint8_t *program = read_file("../assets/roms/SQRT.ch8", &size);

    const uint64_t start_location = 0x200;
    memcpy(&memory[start_location], program, size);

    init();

    const uint32_t scale_x = 20;
    const uint32_t scale_y = 20;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    SDL_Window *window = SDL_CreateWindow(
        "Chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        DISPLAY_WIDTH * scale_x, DISPLAY_HEIGHT * scale_y, SDL_WINDOW_SHOWN);
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

    SDL_Event event = {0};
    while (running) {
        // Poll Events
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT: running = 0; break;
                case SDL_KEYDOWN: {
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_1: keypad[KEY_01] = 1; break;
                        case SDL_SCANCODE_2: keypad[KEY_02] = 1; break;
                        case SDL_SCANCODE_3: keypad[KEY_03] = 1; break;
                        case SDL_SCANCODE_4: keypad[KEY_04] = 1; break;
                        case SDL_SCANCODE_5: keypad[KEY_05] = 1; break;
                        case SDL_SCANCODE_6: keypad[KEY_06] = 1; break;
                        case SDL_SCANCODE_7: keypad[KEY_07] = 1; break;
                        case SDL_SCANCODE_8: keypad[KEY_08] = 1; break;
                        case SDL_SCANCODE_9: keypad[KEY_09] = 1; break;
                        case SDL_SCANCODE_A: keypad[KEY_0A] = 1; break;
                        case SDL_SCANCODE_B: keypad[KEY_0B] = 1; break;
                        case SDL_SCANCODE_C: keypad[KEY_0C] = 1; break;
                        case SDL_SCANCODE_D: keypad[KEY_0D] = 1; break;
                        case SDL_SCANCODE_E: keypad[KEY_0E] = 1; break;
                        case SDL_SCANCODE_F: keypad[KEY_0F] = 1; break;
                        default: break;
                    }
                } break;
                case SDL_KEYUP: {
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_1: keypad[KEY_01] = 0; break;
                        case SDL_SCANCODE_2: keypad[KEY_02] = 0; break;
                        case SDL_SCANCODE_3: keypad[KEY_03] = 0; break;
                        case SDL_SCANCODE_4: keypad[KEY_04] = 0; break;
                        case SDL_SCANCODE_5: keypad[KEY_05] = 0; break;
                        case SDL_SCANCODE_6: keypad[KEY_06] = 0; break;
                        case SDL_SCANCODE_7: keypad[KEY_07] = 0; break;
                        case SDL_SCANCODE_8: keypad[KEY_08] = 0; break;
                        case SDL_SCANCODE_9: keypad[KEY_09] = 0; break;
                        case SDL_SCANCODE_A: keypad[KEY_0A] = 0; break;
                        case SDL_SCANCODE_B: keypad[KEY_0B] = 0; break;
                        case SDL_SCANCODE_C: keypad[KEY_0C] = 0; break;
                        case SDL_SCANCODE_D: keypad[KEY_0D] = 0; break;
                        case SDL_SCANCODE_E: keypad[KEY_0E] = 0; break;
                        case SDL_SCANCODE_F: keypad[KEY_0F] = 0; break;
                        default: break;
                    }
                } break;
                default: break;
            }
        }
        // Main Loop
        const uint64_t start = SDL_GetPerformanceCounter();

        for (uint16_t i = 0; i < 500 / 60; i++)
            instruction_exec(size);

        const uint64_t end = SDL_GetPerformanceCounter();

        const double elapsed =
            (double)((end - start) * 1000) / SDL_GetPerformanceFrequency();

        SDL_Delay(16.67f > elapsed ? 16.67f - elapsed : 0);
        if (regs[REG_DT] > 0)
            regs[REG_DT]--;

        if (regs[REG_ST] > 0) {
            regs[REG_ST]--;
        }

        SDL_Rect rect = {
            .x = 0,
            .y = 0,
            .w = scale_x,
            .h = scale_y,
        };

        SDL_RenderClear(renderer);
        for (uint32_t i = 0; i < sizeof(display); i++) {
            rect.x = (i % DISPLAY_WIDTH) * scale_x;
            rect.y = (i / DISPLAY_WIDTH) * scale_y;

            if (display[i]) {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderFillRect(renderer, &rect);
#if 1 // Outline pixels
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
                SDL_RenderDrawRect(renderer, &rect);
#endif
            } else {
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        SDL_RenderPresent(renderer);
    }

    // Cleanup
    {
        free(program);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }
    return 0;
}
