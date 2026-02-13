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
    REG_DT,
    REG_ST,

    /* Stack Pointer register */
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
static uint16_t *reg_pc;
static uint16_t reg_i;

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
    long size = ftell(file) - 1;
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

//TODO: Write to file
static void debug_memory(uint16_t n)
{
    /*
    printf("0x%04x: ", 0);
    for (uint64_t i = 0; i < RAM_SIZE - 1; i += 2)
    {
        if (i % 16 == 0 && i != 0)
        {
            printf("\n");
            printf("0x%04lx: ", i);
        }
        printf("0x%02X%02X ", memory[i], memory[i + 1]);
    }
    printf("\n");
    */

    for (uint16_t i = 0x200; i < 0x200 + n - 1; i += 2)
    {
        uint8_t msb = memory[i];
        uint8_t lsb = memory[i + 1];
        uint16_t instr = (msb << 8) | lsb;
        uint16_t nnn_addr = instr & 0x0FFF;
        uint8_t nibble = instr & 0x000F;
        uint8_t x = instr & 0x0F00;
        uint8_t y = instr & 0x00F0;
        uint8_t kk_byte = instr & 0x00FF;

        if ((instr & 0x0FFF) == nnn_addr)
            printf("0x%04X: 0x%04X\n", i, instr);
    }
}

static void instruction_exec(uint32_t size)
{
    for (reg_pc = (uint16_t*)(memory + 0x200); reg_pc < (uint16_t *)(memory + 0x200 + size); reg_pc++)
    {
        uint8_t msb = (uint8_t)(*reg_pc & 0x00FF);
        uint8_t lsb = (uint8_t)((*reg_pc & 0xFF00) >> 8);
        uint16_t instr = (msb << 8) | lsb;

        uint16_t nnn_addr = instr & 0x0FFF;
        uint8_t nibble = instr & 0x000F;
        uint8_t x = instr & 0x0F00;
        uint8_t y = instr & 0x00F0;
        uint8_t kk_byte = instr & 0x00FF;
        uint8_t top_nibble = instr & 0xF000;

        switch (top_nibble)
        {
            case 0x0: 
            {
                switch (instr)
                {
                    case 0x00E0:
                    {

                    } break;
                    case 0x00EE:
                    {

                    } break;
                }

            } break;
            case 0x1:
            {
                reg_pc = (uint16_t *)(memory + nnn_addr);
            } break;
            case 0x2:
            {

            } break;
            case 0x3:
            {
                if (regs[x] == kk_byte)
                    ++reg_pc;
                else
                    continue;

            } break;
            case 0x4:
            {
                if (regs[x] != kk_byte)
                    ++reg_pc;
                else
                    continue;
            } break;
            case 0x5:
            {
                if (regs[x] == regs[y])
                    ++reg_pc;
                else
                    continue;
            } break;
            case 0x6:
            {
                regs[x] = kk_byte;

            } break;
            case 0x7:
            {
                regs[x] += kk_byte;

            } break;
            case 0x8:
            {
                switch (nibble)
                {
                    case 0x0:
                    {
                        regs[x] = regs[y];

                    } break;
                    case 0x1:
                    {
                        regs[x] |= regs[y];
                    } break;
                    case 0x2:
                    {
                        regs[x] &= regs[y];
                    } break;
                    case 0x3:
                    {
                        regs[x] ^= regs[y];
                    } break;
                    case 0x4:
                    {
                        uint8_t v1 = regs[x];
                        uint8_t v2 = regs[y];
                        uint16_t result = v1 + v2;

                        if (v1 + v2 > 255)
                            regs[REG_VF] = 1;
                        else
                            regs[REG_VF] = 0;

                        regs[x] = (result & 0x00FF);
                    } break;
                    case 0x5:
                    {
                        if (regs[x] > regs[y])
                            regs[REG_VF] = 1;
                        else
                            regs[REG_VF] = 0;

                        regs[x] = regs[x] - regs[y];
                    } break;
                    case 0x6:
                    {
                        if (top_nibble & 0b1000)
                            regs[REG_VF] = 1;
                        else
                            regs[REG_VF] = 0;

                        regs[x] /= 2;
                    } break;
                    case 0x7:
                    {
                        if (regs[x] > regs[y])
                            regs[REG_VF] = 1;
                        else
                            regs[REG_VF] = 0;

                        regs[x] = regs[y] - regs[x];
                    } break;
                    case 0xE:
                    {
                        if (nibble & 0b0001)
                            regs[REG_VF] = 1;
                        else
                            regs[REG_VF] = 0;

                        regs[x] *= 2;
                    } break;
                } break;

            } break;
            case 0x9:
            {
                if (regs[x] != regs[y])
                    ++reg_pc;
                else
                    continue;
            } break;
            case 0xA:
            {
                reg_i = nnn_addr;
            } break;
            case 0xB:
            {
                reg_pc = (uint16_t *)(memory + nnn_addr + regs[0]);
            } break;
            case 0xF:
            {
                
                switch (kk_byte)
                {
                    case 0x07:
                    {
                        regs[x] = regs[REG_DT];
                    } break;
                    case 0x0A:
                    {
                    } break;
                    case 0x15:
                    {
                        regs[REG_DT] = regs[x];
                    } break;
                    case 0x18:
                    {
                        regs[REG_ST] = regs[x];
                    } break;
                    case 0x1E:
                    {
                        reg_i = reg_i + regs[x];
                    } break;
                    case 0x29:
                    {

                    } break;
                    case 0x33:
                    {
                        uint8_t digit = regs[x];
                        for (int8_t i = 2; i >= 0; i--)
                        {
                            uint8_t nr = digit % 10;
                            *(uint16_t *)(memory + reg_i + i) = nr;
                            digit /= 10;
                        }

                    } break;
                }

            } break;
        }

        printf("0x%04hX\n", instr);
    }
    
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
    uint8_t *program = read_file("../../../assets/roms/IBM Logo.ch8", &size);

    memcpy(&memory[512], program, size);
    free(program);

    init();
    //debug_memory((uint16_t)size);
    instruction_exec(size);
    return 0;
}
