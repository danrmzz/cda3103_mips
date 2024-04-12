#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{

    switch (ALUControl) {

        // 000 Addition
        case 0:
            *ALUresult = A + B;
            break;

        // 001 Subtraction
        case 1:
            *ALUresult = A - B;
            break;

        // 010 Less than
        case 2:
            if ((int)A < (int)B)
                *ALUresult = 1;
            else
                *ALUresult = 0;
            break;

        // 011 Less than (unsigned)
        case 3:
            if (A < B)
                *ALUresult = 1;
            else
                *ALUresult = 0;
            break;

        // 100 bitwise AND
        case 4: 
            *ALUresult = (A & B);
            break;

        // 101 bitwise OR
        case 5:
            *ALUresult = (A | B);
            break;

        // 110 Left shift
        case 6:
            *ALUresult = B << 16;
            break;

        // 111 bitwise NOT
        case 7:
            *ALUresult = ~A;
            break;
    }

    // "Assign Zero to 1 if the result is zero; otherwise, assign 0."
    if (*ALUresult == 0)
        *Zero = 1;

    else
        *Zero = 0;

}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{

    // Word alignment check: PC must be a multiple of 4
    if (PC % 4 != 0) {
        printf("Halt: PC is not word-aligned.\n");
        return 1; // Halt condition: PC is not word-aligned
    }

    // Project hints only mentions word alignment check
    // // Memory bounds check: PC must be within the 64kB range
    // if ((PC >> 2) >= 65536 / sizeof(unsigned)) {
    //     printf("Halt: Address is beyond the memory bounds.\n");
    //     return 1; // Halt condition: Address is beyond the memory bounds
    // }

    // If no halt conditions, fetch the instruction
    *instruction = Mem[PC >> 2];
    printf("Fetched instruction: 0x%08X\n", *instruction);
    return 0; // No halt condition

}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    // instruction [31-26]
    *op = (instruction >> 26) & 0x3F;

    // instruction [25-21]
    *r1 = (instruction >> 21) & 0x1F;

    // instruction [20-16]
    *r2 = (instruction >> 16) & 0x1F;

    // instruction [15-11]
    *r3 = (instruction >> 11) & 0x1F;

    // instruction [5-0]
    *funct = (instruction >> 0) & 0x3F; // can also be: instruction & 0x3F;

    // instruction [15-0]
    *offset = instruction & 0xFFFF;

    // instruction [25-0]
    *jsec = instruction & 0x03FFFFFF;
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{

}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{

}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{

}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{

}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{

}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{

}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    // Add 4 after each step
    *PC += 4;

    // If branching, and the Zero flag is set, branch to the address
    if (Branch && Zero)
        *PC += extended_value << 2; 

    // If jumping, jump to the address
    if (Jump)
        *PC = jsec << 2; // cwalk's: *PC = (jsec << 2) | (*PC | 0xf0000000);
}

