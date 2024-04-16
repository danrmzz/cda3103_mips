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
        // printf("Halt: PC is not word-aligned.\n");
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
    // printf("Fetched instruction: 0x%08X\n", *instruction);
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
    // Decode based on opcode
    switch(op)
    {
        case 0x00: // For R-type instructions (add, sub, and, or, slt, sltu)
            controls->RegDst = 1;
            controls->ALUSrc = 0;
            controls->MemtoReg = 0;
            controls->RegWrite = 1;
            controls->MemRead = 0;
            controls->MemWrite = 0;
            controls->Branch = 0;
            controls->ALUOp = 7; // R-type specific operations
            controls->Jump = 0;
            break;

        case 0x08: // addi
            controls->RegDst = 0;
            controls->ALUSrc = 1;
            controls->MemtoReg = 0;
            controls->RegWrite = 1;
            controls->MemRead = 0;
            controls->MemWrite = 0;
            controls->Branch = 0;
            controls->ALUOp = 0; // Addition
            controls->Jump = 0;
            break;

        case 0x23: // lw
            controls->RegDst = 0;
            controls->ALUSrc = 1;
            controls->MemtoReg = 1;
            controls->RegWrite = 1;
            controls->MemRead = 1;
            controls->MemWrite = 0;
            controls->Branch = 0;
            controls->ALUOp = 0; // Addition for memory read
            controls->Jump = 0;
            break;

        case 0x2B: // sw
            controls->ALUSrc = 1;
            controls->MemWrite = 1;
            controls->Branch = 0;
            controls->ALUOp = 0; // Addition for memory write
            controls->Jump = 0;
            // No need to set RegDst, MemtoReg, RegWrite as these are don't care for sw
            break;

        case 0x0F: // lui
            controls->RegDst = 0;
            controls->ALUSrc = 1;
            controls->MemtoReg = 0;
            controls->RegWrite = 1;
            controls->MemRead = 0;
            controls->MemWrite = 0;
            controls->Branch = 0;
            controls->ALUOp = 6; // Load upper immediate
            controls->Jump = 0;
            break;

        case 0x04: // beq
            controls->ALUSrc = 0;
            controls->Branch = 1;
            controls->ALUOp = 1; // Subtraction for branch comparison
            controls->Jump = 0;
            // No need to set RegDst, MemtoReg, RegWrite, MemRead, MemWrite as these are don't care for beq
            break;

        case 0x0A: // slti
            controls->RegDst = 0;
            controls->ALUSrc = 1;
            controls->MemtoReg = 0;
            controls->RegWrite = 1;
            controls->MemRead = 0;
            controls->MemWrite = 0;
            controls->Branch = 0;
            controls->ALUOp = 2; // Set less than
            controls->Jump = 0;
            break;

        case 0x0B: // sltiu
            controls->RegDst = 0;
            controls->ALUSrc = 1;
            controls->MemtoReg = 0;
            controls->RegWrite = 1;
            controls->MemRead = 0;
            controls->MemWrite = 0;
            controls->Branch = 0;
            controls->ALUOp = 3; // Set less than unsigned
            controls->Jump = 0;
            break;

        case 0x02: // j
            controls->Jump = 1;
            // No need to set other controls as they are don't care for jump
            break;

        // Add cases for any additional instructions you may have.

        default: // Illegal instruction
            return 1; // Halt condition met
    }
    
    return 0; // Continue if no halt condition met
}
    

/* Read Register */
/* 5 Points */
void read_register(unsigned r1,unsigned r2,unsigned *Reg,unsigned *data1,unsigned *data2)
{
    // Assuming Reg is an array representing the 32 MIPS registers.
    // Read the values from registers r1 and r2, and write them to data1 and data2 respectively.

    *data1 = Reg[r1]; // Read the value at register r1
    *data2 = Reg[r2]; // Read the value at register r2
}


/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset,unsigned *extended_value)
{
    // Check if the sign bit of the 16-bit offset is 1 (negative number)
    if (offset & 0x8000)
    {
        // If it is, fill the upper 16 bits with 1s to maintain the negative value
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
    unsigned B; // Define B which will be the second ALU operand

    // Decide the second operand of the ALU based on the ALUSrc signal
    if (ALUSrc == 1)
        B = extended_value; // Immediate value is used as the second operand
    else
        B = data2; // The value from the second register is used as the second operand

    // Determine the operation to perform based on ALUOp
    switch (ALUOp)
    {
        case 0: // Addition
            ALU(data1, B, 0, ALUresult, Zero);
            break;
        case 1: // Subtraction
            ALU(data1, B, 1, ALUresult, Zero);
            break;
        case 2: // Set less than (signed)
            ALU(data1, B, 2, ALUresult, Zero);
            break;
        case 3: // Set less than (unsigned)
            ALU(data1, B, 3, ALUresult, Zero);
            break;
        case 4: // AND
            ALU(data1, B, 4, ALUresult, Zero);
            break;
        case 5: // OR
            ALU(data1, B, 5, ALUresult, Zero);
            break;
        case 6: // LUI - Not typically handled here but if needed
            ALU(data1, B, 6, ALUresult, Zero);
            break;
        case 7: // Function code based operations (for R-type instructions)
            switch (funct)
            {
                case 0x20: // ADD (function code for add)
                    ALU(data1, B, 0, ALUresult, Zero);
                    break;
                case 0x22: // SUB (function code for sub)
                    ALU(data1, B, 1, ALUresult, Zero);
                    break;
                case 0x24: // AND (function code for and)
                    ALU(data1, B, 4, ALUresult, Zero);
                    break;
                case 0x25: // OR (function code for or)
                    ALU(data1, B, 5, ALUresult, Zero);
                    break;
                case 0x2A: // SLT (function code for set less than)
                    ALU(data1, B, 2, ALUresult, Zero);
                    break;
                case 0x2B: // SLTU (function code for set less than unsigned)
                    ALU(data1, B, 3, ALUresult, Zero);
                    break;
                default:
                    return 1; // Illegal function code - halt
            }
            break;
        default:
            return 1; // Illegal ALU operation code - halt
    }

    return 0; // Continue execution
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult,unsigned data2,char MemWrite,char MemRead,unsigned *memdata,unsigned *Mem)
{
    if ((MemWrite == 1 || MemRead == 1) && ALUresult % 4 != 0) // If address is bad, then return a halt condition
	{
		//Memory call out of range
		return 1;
	}
	//checks if MemWrite is 1. If it is, it sets memory of ALUresult to data2
	if (MemWrite == 1)
	{
		Mem[ALUresult >> 2] = data2;
	}
	//checks if MemRead is 1. If it is, it sets the memory data to memory of ALUresult shifted 2-bits
	if (MemRead == 1)
	{
		*memdata = Mem[ALUresult >> 2];	
	}
    
	return 0;
}


/* Write Register */
/* 10 Points */
void write_register(unsigned r2,unsigned r3,unsigned memdata,unsigned ALUresult,char RegWrite,char RegDst,char MemtoReg,unsigned *Reg)
{
    if (RegWrite == 1) // Only proceed if we are supposed to write to a register
    {
        unsigned writeData; // The data to be written into the register
        unsigned writeRegister; // The register number where data will be written

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

        // Write the data to the selected register
        Reg[writeRegister] = writeData;
    }
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

