#include "spimcore.h"


/* ALU */
/* 10 Points */
void ALU(unsigned A,unsigned B,char ALUControl,unsigned *ALUresult,char *Zero)
{

    if (ALUControl == 0) {
        // 000 Addition
        *ALUresult = A + B;
    }
    else if (ALUControl == 1) {
        // 001 Subtraction
        *ALUresult = A - B;
    }
    else if (ALUControl == 2) {
        // 010 Less than
        if ((int)A < (int)B)
            *ALUresult = 1;
        else
            *ALUresult = 0;
    }
    else if (ALUControl == 3) {
        // 011 Less than (unsigned)
        if (A < B)
            *ALUresult = 1;
        else
            *ALUresult = 0;
    }
    else if (ALUControl == 4) {
        // 100 bitwise AND
        *ALUresult = (A & B);
    }
    else if (ALUControl == 5) {
        // 101 bitwise OR
        *ALUresult = (A | B);
    }
    else if (ALUControl == 6) {
        // 110 Left shift
        *ALUresult = B << 16;
    }
    else if (ALUControl == 7) {
        // 111 bitwise NOT
        *ALUresult = ~A;
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

    // PC needs to be a multiple of 4
    if (PC % 4 != 0) 
        return 1;

    // If there are no halt conditions, fetch the instruction
    *instruction = Mem[PC >> 2];
    
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
    *funct = (instruction >> 0) & 0x3F;

    // instruction [15-0]
    *offset = instruction & 0xFFFF;

    // instruction [25-0]
    *jsec = instruction & 0x03FFFFFF;
}



/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op,struct_controls *controls)
{
    // Decode based on opcode
    if (op == 0x00) { // For R-type instructions (add, sub, and, or, slt, sltu)
        controls->RegDst = 1;
        controls->ALUSrc = 0;
        controls->MemtoReg = 0;
        controls->RegWrite = 1;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->Branch = 0;
        controls->ALUOp = 7;
        controls->Jump = 0;
    }
    else if (op == 0x08) { // addi
        controls->RegDst = 0;
        controls->ALUSrc = 1;
        controls->MemtoReg = 0;
        controls->RegWrite = 1;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->Branch = 0;
        controls->ALUOp = 0;
        controls->Jump = 0;
    }
    else if (op == 0x23) { // lw
        controls->RegDst = 0;
        controls->ALUSrc = 1;
        controls->MemtoReg = 1;
        controls->RegWrite = 1;
        controls->MemRead = 1;
        controls->MemWrite = 0;
        controls->Branch = 0;
        controls->ALUOp = 0;
        controls->Jump = 0;
    }
    else if (op == 0x2B) { // sw
        controls->ALUSrc = 1;
        controls->MemWrite = 1;
        controls->Branch = 0;
        controls->ALUOp = 0;
        controls->Jump = 0;
        // Don't need to set RegDst, MemtoReg, RegWrite since they are don't care for sw
    }
    else if (op == 0x0F) { // lui
        controls->RegDst = 0;
        controls->ALUSrc = 1;
        controls->MemtoReg = 0;
        controls->RegWrite = 1;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->Branch = 0;
        controls->ALUOp = 6;
        controls->Jump = 0;
    }
    else if (op == 0x04) { // beq
        controls->ALUSrc = 0;
        controls->Branch = 1;
        controls->ALUOp = 1;
        controls->Jump = 0;
        // Don't need to set RegDst, MemtoReg, RegWrite, MemRead, MemWrite since they are don't care for beq
    }
    else if (op == 0x0A) { // slti
        controls->RegDst = 0;
        controls->ALUSrc = 1;
        controls->MemtoReg = 0;
        controls->RegWrite = 1;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->Branch = 0;
        controls->ALUOp = 2;
        controls->Jump = 0;
    }
    else if (op == 0x0B) { // sltiu
        controls->RegDst = 0;
        controls->ALUSrc = 1;
        controls->MemtoReg = 0;
        controls->RegWrite = 1;
        controls->MemRead = 0;
        controls->MemWrite = 0;
        controls->Branch = 0;
        controls->ALUOp = 3;
        controls->Jump = 0;
    }
    else if (op == 0x02) { // j
        controls->Jump = 1;
        // Don't need to set other controls since they are don't care for jump
    }
    else // Illegal instruction
        return 1; // Halt condition met
    
    return 0; // Continue if no halt condition met
}
    

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    // Read value at register r1 and r2
    *data1 = Reg[r1]; 
    *data2 = Reg[r2];
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    // Check if the sign bit of the 16-bit offset is 1 (negative number)
    if (offset & 0x8000)
    {
        // If it is, fill the upper 16 bits with 1s to keep the negative value
        *extended_value = offset | 0xFFFF0000;
    }
    else
    {
        // If it isn't, fill the upper 16 bits with 0s
        *extended_value = offset & 0x0000FFFF;
    }
}


/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1,unsigned data2,unsigned extended_value,unsigned funct,char ALUOp,char ALUSrc,unsigned *ALUresult,char *Zero)
{
    unsigned B;

    
    if (ALUSrc == 1)
        B = extended_value;
    else
        B = data2;

    // Determine the operation to perform based on ALUOp
    if (ALUOp == 0) { // Addition
        ALU(data1, B, 0, ALUresult, Zero);
    }
    else if (ALUOp == 1) { // Subtraction
        ALU(data1, B, 1, ALUresult, Zero);
    }
    else if (ALUOp == 2) { // Set less than (signed)
        ALU(data1, B, 2, ALUresult, Zero);
    }
    else if (ALUOp == 3) { // Set less than (unsigned)
        ALU(data1, B, 3, ALUresult, Zero);
    }
    else if (ALUOp == 4) { // AND
        ALU(data1, B, 4, ALUresult, Zero);
    }
    else if (ALUOp == 5) { // OR
        ALU(data1, B, 5, ALUresult, Zero);
    }
    else if (ALUOp == 6) { // LUI - Not typically handled here but if needed
        ALU(data1, B, 6, ALUresult, Zero);
    }
    else if (ALUOp == 7) { // Function code based operations (for R-type instructions)
       
        if (funct == 0x20) { // ADD (function code for add)
            ALU(data1, B, 0, ALUresult, Zero);
        }
        else if (funct == 0x22) { // SUB (function code for sub)
            ALU(data1, B, 1, ALUresult, Zero);
        }
        else if (funct == 0x24) { // AND (function code for and)
            ALU(data1, B, 4, ALUresult, Zero);
        }
        else if (funct == 0x25) { // OR (function code for or)
            ALU(data1, B, 5, ALUresult, Zero);
        }
        else if (funct == 0x2A) { // SLT (function code for set less than)
            ALU(data1, B, 2, ALUresult, Zero);
        }
        else if (funct == 0x2B) { // SLTU (function code for set less than unsigned)
            ALU(data1, B, 3, ALUresult, Zero);
        }
        else 
            return 1; // Halt
    }
    else 
        return 1; // Halt
    
    return 0; // Continue
}


/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
	if (MemWrite == 1)
	
		Mem[ALUresult >> 2] = data2;

	if (MemRead == 1)

		*memdata = Mem[ALUresult >> 2];	
	
    
	return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if (RegWrite == 1) 
    {
        unsigned writeData;
        unsigned writeRegister;

        // Decide what data to write based on MemtoReg
        if (MemtoReg == 1)
            writeData = memdata; // Data from memory
        else
            writeData = ALUresult; // Data from ALU result

        // Decide which register to write to based on RegDst
        if (RegDst == 1)
            writeRegister = r3; // The destination register is r3 (for R-type instructions)
        else
            writeRegister = r2; // The destination register is r2 (for I-type instructions)

        
        Reg[writeRegister] = writeData;
    }
}


/* PC update */
/* 10 Points */
void PC_update(unsigned jsec,unsigned extended_value,char Branch,char Jump,char Zero,unsigned *PC)
{
    // Add 4 after each step
    *PC += 4;

    // Branch to the address
    if (Branch && Zero)
        *PC += extended_value << 2; 

    // Jump to the address
    if (Jump)
        *PC = jsec << 2;
}

