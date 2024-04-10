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
    switch(op) {
        case 0x00: // This is the opcode for R-type instructions
            controls->RegDst = 1;  // The destination register is rd for R-type instructions like 'add'
            controls->ALUSrc = 0;  // The second operand is the register for 'add'
            controls->RegWrite = 1;  // Enable writing back to the register
            // ALUOp does not need to be set here for 'add'; it's determined by the funct field in ALU_operations
            return 0;
        case 0x08: // Opcode for 'addi'
            controls->RegDst = 0;  // The destination register is rt for I-type instructions like 'addi'
            controls->ALUSrc = 1;  // The second operand is the immediate value for 'addi'
            controls->RegWrite = 1;  // Enable writing back to the register
            controls->ALUOp = 0;  // Set ALUOp to indicate addition
            return 0;
        case 0x2B: // Opcode for 'sw'
            controls->ALUSrc = 1; // Use the immediate value as the second ALU operand
            controls->MemWrite = 1; // Enable writing to memory
            controls->RegWrite = 0; // Disable writing back to the register
            controls->ALUOp = 0; // Set ALUOp to indicate addition for address calculation
            return 0;
    }
    return 1; // Halt condition for unrecognized opcodes
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
    if (ALUOp == 0) { // Handle addition; works for both 'add' and 'addi'
        // Choose the second operand based on ALUSrc; for 'addi', it's the extended immediate value
        unsigned secondOperand = ALUSrc ? extended_value : data2;
        ALU(data1, secondOperand, ALUOp, ALUresult, Zero);
    }
    // Add more cases as needed for other operations
    return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    if (MemWrite) {
        if (ALUresult % 4 != 0) // Check for word alignment
            return 1; // Return 1 to indicate a halt condition due to misalignment
        
        Mem[ALUresult >> 2] = data2; // Store the data into memory at the calculated address
    }
    // Include MemRead logic here if needed for future operations like 'load word (lw)'
    return 0; // No halt condition; continue execution
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if (RegWrite) {
        unsigned destReg = RegDst ? r3 : r2; // Choose rd (r3) for R-type (e.g., 'add'), rt (r2) for I-type (e.g., 'addi')
        Reg[destReg] = ALUresult; // Write the ALU result to the chosen destination register
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    *PC += 4; // Increment PC to the next instruction
    // Additional logic for jumps and branches will be needed here
}

