#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    switch(ALUControl) {
        case 0: // Addition
            *ALUresult = A + B;
            break;
        case 2: // SLT (Set on Less Than)
            *ALUresult = (A < B) ? 1 : 0; // Set *ALUresult to 1 if A < B, else 0
            break;
        case 3: // SLTU (Set on Less Than, unsigned)
            *ALUresult = (A < B) ? 1 : 0;
            break;
        // Add other cases as necessary
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
    *op = (instruction >> 26) & 0x3F;
    *r1 = (instruction >> 21) & 0x1F; // rs
    *r2 = (instruction >> 16) & 0x1F; // rt
    *r3 = (instruction >> 11) & 0x1F; // rd for R-type, like 'add'
    *funct = instruction & 0x3F; // funct field for R-type
    *offset = instruction & 0xFFFF; // Immediate value, used for I-type like 'addi'
    // jsec not used here
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    memset(controls, 0, sizeof(struct_controls)); // Clear previous control signals

    switch (op) {
        case 0x00: // R-type
            controls->RegDst = 1;
            controls->RegWrite = 1;
            controls->ALUOp = 7; // Signal for R-type operation
            break;
        case 0x02: // j (Jump)
            controls->Jump = 1;
            break;
        case 0x23: // lw
            controls->ALUSrc = 1; controls->MemtoReg = 1;
            controls->RegWrite = 1; controls->MemRead = 1;
            controls->ALUOp = 0; // Addition for memory address calculation
            break;
        case 0x2B: // sw
            controls->ALUSrc = 1; controls->MemWrite = 1;
            controls->ALUOp = 0; // Addition for memory address calculation
            break;
        case 0x08: // addi
            controls->ALUSrc = 1; controls->RegWrite = 1;
            controls->ALUOp = 0; // Directly to ALU addition operation
            break;
        case 0x0F: // LUI
            controls->ALUSrc = 1;
            controls->RegWrite = 1;
            controls->ALUOp = 6; // Ensure this matches `ALU_operations`
            break;
        // Ensure there's a case for SLTU, either here with a unique ALUOp or within R-type handling based on `funct`
    }
    return 0; // Success
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
    if (ALUOp == 7) { // Convention for R-type
        switch (funct) {
            case 0x20: // ADD
                *ALUresult = data1 + data2;
                break;
            case 0x22: // SUB
                *ALUresult = data1 - data2;
                break;
            case 0x2A: // SLT
                *ALUresult = (int)data1 < (int)data2 ? 1 : 0;
                break;
            case 0x2B: // SLTU
                *ALUresult = data1 < data2 ? 1 : 0;
                break;
            // Additional R-type instructions can be handled here
            default:
                return 1; // Unrecognized funct code
        }
    } else {
        // I-type operations handling, including addi and lui, if necessary
    }
    *Zero = (*ALUresult == 0) ? 1 : 0;
    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    if (MemRead) {
        if (ALUresult % 4 != 0)  // Check for word alignment
            return 1;  // Return 1 to indicate a halt condition due to misalignment
        
        *memdata = Mem[ALUresult >> 2];  // Load the data from memory at the calculated address
    }
    if (MemWrite) {
        if (ALUresult % 4 != 0)  // Check for word alignment
            return 1;  // Return 1 to indicate a halt condition due to misalignment
        
        Mem[ALUresult >> 2] = data2;  // Store the data into memory at the calculated address
    }
    return 0;  // No halt condition; continue execution
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if (RegWrite) {
        unsigned destReg = RegDst ? r3 : r2; // Determine destination register
        unsigned writeData = MemtoReg ? memdata : ALUresult; // Select data to write
        Reg[destReg] = writeData; // Perform write operation
        printf("Writing %u to register %u\n", writeData, destReg); // Debugging statement
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    // Standard PC increment
    *PC += 4;

    // Handle jump instructions
    if (Jump) {
        *PC = (jsec << 2) | (*PC & 0xf0000000); // Assuming jsec is the lower 28 bits of the target
    }

    // Handle branch instructions
    if (Branch && Zero) {
        *PC += (extended_value << 2); // Assuming extended_value is sign-extended offset
    }
}

