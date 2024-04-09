#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    switch(ALUControl) {
        case 0: // Assuming 0 is the control code for addition
            *ALUresult = A + B;
            break;
        // Implement other cases as needed
    }
    *Zero = (*ALUresult == 0) ? 1 : 0;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    if(PC % 4 != 0) // Check for word alignment
        return 1; // Return 1 to indicate a halt condition

    *instruction = Mem[PC >> 2]; // Fetch the instruction; PC >> 2 converts byte address to word address

    return 0; // No halt condition; continue execution
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    *op = (instruction >> 26) & 0x3F; // Extracts the opcode
    *r1 = (instruction >> 21) & 0x1F; // Source register (rs)
    *r2 = (instruction >> 16) & 0x1F; // Target register (rt)
    *offset = instruction & 0xFFFF; // Immediate value
    // Note: For 'addi', r3, funct, and jsec are not used
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    switch(op) {
        case 0x08: // Assuming opcode for 'addi' is 0x08
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            controls->ALUOp = 0; // Assuming '0' is the control code for addition
            return 0;
    }
    return 1; // Return 1 for unrecognized opcodes (halt condition)
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    // Reg is the register file array, r1 and r2 are the register numbers to read
    *data1 = Reg[r1]; // Read the first register value into data1
    *data2 = Reg[r2]; // Read the second register value into data2
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    // Cast the 16-bit offset to a signed 16-bit value, then assign it to a signed 32-bit variable.
    // Casting the 16-bit value to a 32-bit signed int automatically sign-extends it.
    int temp = (int)(short)offset;  // First cast to short (signed 16-bit), then to int (signed 32-bit).
    
    // Finally, cast back to unsigned, as the extended_value expects an unsigned.
    *extended_value = (unsigned)temp;
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    // For `addi`, ALUSrc should be 1, indicating the second operand is the immediate value (sign-extended)
    if (ALUSrc == 1) {
        ALU(data1, extended_value, ALUOp, ALUresult, Zero); // Assuming ALUOp for addition is correctly set
    }
    return 0; // Assuming no halt conditions for now
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
    if (RegWrite) {
        if (MemtoReg == 0) // For `addi`, the result does not come from memory
            Reg[r2] = ALUresult; // Assuming r2 is the destination register for `addi`
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    *PC += 4; // Increment PC to the next instruction
    // Additional logic for jumps and branches will be needed here
}

