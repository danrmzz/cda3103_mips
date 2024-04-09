#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{
    if (ALUControl == 0) {
        // Z = A + B
        *ALUresult = A + B;
    } else if (ALUControl == 1) {
        // Z = A - B
        *ALUresult = A - B;
    } else if (ALUControl == 2) {
        // if A < B, Z = 1; otherwise, Z = 0
        *ALUresult = A < B ? 1 : 0;
    } else if (ALUControl == 3) {
        // if A < B (unsigned), Z = 1; otherwise, Z = 0
        *ALUresult = (unsigned)A < (unsigned)B ? 1 : 0;
    } else if (ALUControl == 4) {
        // Z = A AND B
        *ALUresult = A & B;
    } else if (ALUControl == 5) {
        // Z = A OR B
        *ALUresult = A | B;
    } else if (ALUControl == 6) {
        // Z = Shift B left by 16 bits
        *ALUresult = B << 16;
    } else if (ALUControl == 7) {
        // Z = NOT A
        *ALUresult = ~A;
    }

    // Set Zero to 1 if the result is zero; otherwise, set to 0.
    if (*ALUresult == 0) {
        *Zero = 1;
    } else {
        *Zero = 0;
    }

}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC,unsigned *Mem,unsigned *instruction)
{
    // Check if PC is word-aligned by ensuring the last two bits are 0
    // Since our memory array is word-addressable, we divide the byte-addressable PC by 4.
    if ((PC % 4) != 0)
        return 1; // Halt condition if the instruction is misaligned

    // Check for memory bounds
    // The memory size is 64KB; since it's word-addressable, we divide by 4 to get the number of entries.
    if (PC >= 0x10000)
        return 1; // Halt condition if PC is beyond the memory size of 64KB

    // Fetch the instruction and write it to the instruction pointer
    // Shift right by 2 to convert from byte address to word address
    *instruction = Mem[PC >> 2];
    
    return 0; // Return 0 to indicate that no halt condition occurre
}


/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1,unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec)
{
    // Extract the opcode: bits 31-26
    *op = (instruction >> 26) & 0x3F; 
    // Extract the source register (r1): bits 25-21
    *r1 = (instruction >> 21) & 0x1F; 
    // Extract the target register or the second source register (r2): bits 20-16
    *r2 = (instruction >> 16) & 0x1F;
    // Extract the destination register (r3): bits 15-11 for R-type instructions
    *r3 = (instruction >> 11) & 0x1F; 
    // Extract the function field: bits 5-0 for R-type instructions
    *funct = instruction & 0x3F;
    // Extract the immediate value/offset field: bits 15-0 for I-type instructions
    *offset = instruction & 0xFFFF;
    // Extract the jump target address: bits 25-0 for J-type instructions
    *jsec = instruction & 0x3FFFFFF;
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    // Reset all control signals
    controls->RegDst = controls->Jump = controls->Branch = controls->MemRead = 0;
    controls->MemtoReg = controls->ALUOp = controls->MemWrite = controls->ALUSrc = controls->RegWrite = 0;

    switch(op) {
        case 0x00: // R-type instructions
            controls->RegDst = 1;
            controls->RegWrite = 1;
            controls->ALUOp = 7; // ALUOp for R-type could be different based on the funct field, but here we assume it's '7'
            break;

        case 0x02: // j (jump)
            controls->Jump = 1;
            break;

        case 0x04: // beq (branch if equal)
            controls->Branch = 1;
            controls->ALUOp = 1; // ALU operation for subtraction to compare registers
            break;

        case 0x23: // lw (load word)
            controls->MemRead = 1;
            controls->MemtoReg = 1;
            controls->RegWrite = 1;
            controls->ALUSrc = 1;
            controls->ALUOp = 0; // ALU operation for addition (base + offset)
            break;

        case 0x2B: // sw (store word)
            controls->MemWrite = 1;
            controls->ALUSrc = 1;
            controls->ALUOp = 0; // ALU operation for addition (base + offset)
            break;

        case 0x08: // addi (add immediate)
            controls->RegWrite = 1;
            controls->ALUSrc = 1;
            controls->ALUOp = 0; // ALU operation for addition
            break;

        // ... Include other cases for the instructions you need to support

        default: // Illegal opcode
            return 1; // Halt the simulation on illegal instruction
    }

    return 0; // Return 0 to indicate successful decoding
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    // Read the value from the register file at the index specified by r1 and r2
    // and store these values into data1 and data2 respectively
    *data1 = Reg[r1];
    *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    // Check if the 16th bit of offset is 1 (indicating a negative number in two's complement)
    if (offset & 0x8000) {
        // If it's negative, fill the upper 16 bits of extended_value with 1s to preserve the sign
        *extended_value = offset | 0xFFFF0000;
    } else {
        // If it's positive, simply copy offset into extended_value, upper 16 bits will be 0
        *extended_value = offset;
    }
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    // Determine the second operand for the ALU operation
    unsigned ALU_operand2 = ALUSrc ? extended_value : data2;

    // Decide on the operation based on ALUOp
    switch (ALUOp) {
        case 0: // For addition (used by lw, sw, addi)
            ALU(data1, ALU_operand2, 0, ALUresult, Zero);
            break;

        case 1: // For subtraction (used by beq)
            ALU(data1, ALU_operand2, 1, ALUresult, Zero);
            break;

        case 2: // For set less than (slt)
            ALU(data1, ALU_operand2, 2, ALUresult, Zero);
            break;

        case 3: // For set less than unsigned (sltu)
            ALU(data1, ALU_operand2, 3, ALUresult, Zero);
            break;

        // Additional cases for other operations based on ALUOp

        case 7: // For R-type instructions, decision based on funct
            switch (funct) {
                // Assuming you have a mapping of funct to operations
                case 0x20: // add
                    ALU(data1, ALU_operand2, 0, ALUresult, Zero);
                    break;
                case 0x22: // subtract
                    ALU(data1, ALU_operand2, 1, ALUresult, Zero);
                    break;
                case 0x24: // AND
                    ALU(data1, ALU_operand2, 4, ALUresult, Zero);
                    break;
                case 0x25: // OR
                    ALU(data1, ALU_operand2, 5, ALUresult, Zero);
                    break;
                // Add additional funct codes as necessary
                default:
                    return 1; // Unsupported funct, potentially halt
            }
            break;

        default:
            return 1; // Unsupported ALUOp, potentially halt
    }

    return 0; // Successful operation
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    // Check for word alignment
    if (ALUresult % 4 != 0) return 1; // Halt if the address is not word-aligned

    // Convert the byte address to a word address by dividing by 4
    unsigned address = ALUresult >> 2;

    // Check memory access bounds
    if (address >= (65536 / 4)) return 1; // Halt if the address is out of bounds, assuming 64KB memory

    if (MemRead) {
        // If MemRead is set, read from memory
        *memdata = Mem[address];
    }

    if (MemWrite) {
        // If MemWrite is set, write to memory
        Mem[address] = data2;
    }

    return 0; // Return 0 to indicate no halt condition occurred
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    // First, check if we should write to a register at all
    if (RegWrite) {
        unsigned writeData = 0; // Data to be written to the register
        unsigned writeReg = 0;  // Register to write to

        // Decide what data to write: result of ALU operation or memory data
        if (MemtoReg)
            writeData = memdata; // Data from memory
        else
            writeData = ALUresult; // Result from ALU operation

        // Decide which register to write to: r2 or r3 based on instruction type
        if (RegDst)
            writeReg = r3; // R-type instruction, write to rd (r3)
        else
            writeReg = r2; // I-type instruction, write to rt (r2)

        // Write the data to the selected register
        Reg[writeReg] = writeData;
    }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    // Regular PC update: Move to the next instruction
    *PC += 4;

    // Handle branching: If the branch condition is met, add the offset to the PC
    if (Branch && Zero) {
        *PC += (extended_value << 2); // Note: The offset needs to be shifted left by 2 to get the byte address
    }

    // Handle jumps: If a jump is instructed, update the PC to the jump address
    if (Jump) {
        *PC = (*PC & 0xF0000000) | (jsec << 2); // Preserve the upper 4 bits of the current PC, replace the lower 28 bits with the jump target
    }
}

