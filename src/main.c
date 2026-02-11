#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define RAM_SIZE 4096
#define STACK_SIZE 16
#define DISPLAY_SIZE (64 * 32)

typedef enum
{
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

    REG_PC, // NOTE: 16 bit Register!
            // TODO: How to handle this register?
    REG_SP,
    REG_COUNT,
} reg;

typedef enum
{
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

//TODO: Create a struct
static uint8_t memory[RAM_SIZE];
static uint16_t stack[STACK_SIZE];
static uint8_t regs[REG_COUNT];
static uint8_t display[DISPLAY_SIZE];
static uint8_t keypad[KEY_COUNT];

// static uint8_t keypad[] = {
//     0x01, 0x02, 0x03, 0x0C,
//     0x04, 0x05, 0x06, 0x0D,
//     0x07, 0x08, 0x09, 0x0E,
//     0x0A, 0x00, 0x0B, 0x0F,
// };

static uint8_t sprites[80] = {
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

//TODO: Create a decent file API
static uint8_t *read_file(const char *filename, uint32_t *file_size)
{
    FILE *file = fopen(filename, "rb");
    fseek(file, 0L, SEEK_END);
    long size = ftell(file) + 1;
    *file_size = size; 
    fclose(file);

    file = fopen(filename, "r");
    uint8_t *content = malloc(size);
    memset(content, '\0', size);
    fread(content, sizeof(uint8_t), size - 1, file);
    fclose(file);

    return content;
}

//TODO: Write to file
static void debug_memory()
{
    printf("0x%04x: ", 0);
    for (uint64_t i = 0; i < RAM_SIZE; ++i)
    {
        if (i % 16 == 0 && i != 0)
        {
            printf("\n");
            printf("0x%04lx: ", i);
        }
        printf("0x%03x ", memory[i]);
    }
    printf("\n");
}

static void init()
{
    memcpy(memory, sprites, sizeof sprites);
}

int main(int argc, char const *argv[])
{
    //TODO: Add terminal support
    // if (argc < 2)
    // {
    //     printf("Usage: chip8 [chip8 program]\n");
    //     return 1;
    // }

    uint32_t size = 0;
    uint8_t *program = read_file("assets/roms/IBM Logo.ch8", &size);
    memcpy(memory + 512, program, size);
    free(program);

    init();
    debug_memory();
    return 0;
}
