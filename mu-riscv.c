#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include "mu-riscv.h"

/***************************************************************/
/* Print out a list of commands available                                                                  */
/***************************************************************/
void help() {        
	printf("------------------------------------------------------------------\n\n");
	printf("\t**********MU-RISCV Help MENU**********\n\n");
	printf("sim\t-- simulate program to completion \n");
	printf("run <n>\t-- simulate program for <n> instructions\n");
	printf("rdump\t-- dump register values\n");
	printf("reset\t-- clears all registers/memory and re-loads the program\n");
	printf("input <reg> <val>\t-- set GPR <reg> to <val>\n");
	printf("mdump <start> <stop>\t-- dump memory from <start> to <stop> address\n");
	printf("high <val>\t-- set the HI register to <val>\n");
	printf("low <val>\t-- set the LO register to <val>\n");
	printf("print\t-- print the program loaded into memory\n");
	printf("?\t-- display help menu\n");
	printf("quit\t-- exit the simulator\n\n");
	printf("------------------------------------------------------------------\n\n");
}

/***************************************************************/
/* Turn a byte to a word                                                                          */
/***************************************************************/
uint32_t byte_to_word(uint8_t byte)
{
	//handles twos-complement signed numbers; sign-extends the byte
    return (byte & 0x80) ? (byte | 0xffffff80) : byte;
}

/***************************************************************/
/* Turn a halfword to a word                                                                          */
/***************************************************************/
uint32_t half_to_word(uint16_t half)
{
	//handles twos-complement signed numbers; sign-extends the halfword
    return (half & 0x8000) ? (half | 0xffff8000) : half;
}

/***************************************************************/
/* Read a 32-bit word from memory                                                                            */
/***************************************************************/
uint32_t mem_read_32(uint32_t address)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return (MEM_REGIONS[i].mem[offset+3] << 24) |
					(MEM_REGIONS[i].mem[offset+2] << 16) |
					(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

uint32_t mem_read_16(uint32_t address, uint32_t value)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return	(MEM_REGIONS[i].mem[offset+1] <<  8) |
					(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
	
}

uint32_t mem_read_8(uint32_t address, uint32_t value)
{
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) &&  ( address <= MEM_REGIONS[i].end) ) {
			uint32_t offset = address - MEM_REGIONS[i].begin;
			return	(MEM_REGIONS[i].mem[offset+0] <<  0);
		}
	}
	return 0;
}

/***************************************************************/
/* Write a 32-bit word to memory                                                                                */
/***************************************************************/
void mem_write_32(uint32_t address, uint32_t value)
{
	int i;
	uint32_t offset;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		if ( (address >= MEM_REGIONS[i].begin) && (address <= MEM_REGIONS[i].end) ) {
			offset = address - MEM_REGIONS[i].begin;

			MEM_REGIONS[i].mem[offset+3] = (value >> 24) & 0xFF;
			MEM_REGIONS[i].mem[offset+2] = (value >> 16) & 0xFF;
			MEM_REGIONS[i].mem[offset+1] = (value >>  8) & 0xFF;
			MEM_REGIONS[i].mem[offset+0] = (value >>  0) & 0xFF;
		}
	}
}

void SYSCALL(CPU_State given_state)
{
	uint32_t code = given_state.REGS[2];
	//TODO : implement the rest of the syscall codes....
	switch(code)
	{
		case(1):
			//print int
			printf("%d\n",given_state.REGS[4]);
			break;
		case(2):
			//print float in f12
			break;
		case(3):
			//print double in f12
			break;
		case(4):
			//print string in memory address $a0
			break;
		case(5):
			//read int to $v0
			(void) scanf("%d", &NEXT_STATE.REGS[2]);
			break;
		case(6):
			//read float
			scanf("%f", (float*) &NEXT_STATE.REGS[2]);
			break;
		case(7):
			//read double
			scanf("%lf", (double*) &NEXT_STATE.REGS[2]);
			break;
		case(8):
			//read string
			//$a0 = memory address of string input buffer
			// $a1 = length of string buffer (n)
			break;
		case(9):
			//sbrk
			break;
		case(10):
			RUN_FLAG = FALSE;
			break;
		default:
			break;
	}

}

/***************************************************************/
/* Execute one cycle                                                                                                              */
/***************************************************************/
void cycle() {                                                
	handle_instruction();
	CURRENT_STATE = NEXT_STATE;
	SYSCALL(CURRENT_STATE);
	INSTRUCTION_COUNT++;
	if(PROGRAM_SIZE == INSTRUCTION_COUNT) RUN_FLAG = false; //end program after handling last instruction
}

/***************************************************************/
/* Simulate RISCV for n cycles                                                                                       */
/***************************************************************/
void run(int num_cycles) {                                      
	
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped\n\n");
		return;
	}

	printf("Running simulator for %d cycles...\n\n", num_cycles);
	int i;
	for (i = 0; i < num_cycles; i++) {
		if (RUN_FLAG == FALSE) {
			printf("Simulation Stopped.\n\n");
			break;
		}
		cycle();
	}
}

/**************************************************************rdump*/
/* simulate to completion                                                                                               */
/***************************************************************/
void runAll() {                                                     
	if (RUN_FLAG == FALSE) {
		printf("Simulation Stopped.\n\n");
		return;
	}

	printf("Simulation Started...\n\n");
	while (RUN_FLAG){
		cycle();
	}
	printf("Simulation Finished.\n\n");
}

/***************************************************************/ 
/* Dump a word-aligned region of memory to the terminal                              */
/***************************************************************/
void mdump(uint32_t start, uint32_t stop) {          
	uint32_t address;

	printf("-------------------------------------------------------------\n");
	printf("Memory content [0x%08x..0x%08x] :\n", start, stop);
	printf("-------------------------------------------------------------\n");
	printf("\t[Address in Hex (Dec) ]\t[Value]\n");
	for (address = start; address <= stop; address += 4){
		printf("\t0x%08x (%d) :\t0x%08x\n", address, address, mem_read_32(address));
	}
	printf("\n");
}

/***************************************************************/
/* Dump current values of registers to the teminal                                              */   
/***************************************************************/
void rdump() {                               
	int i; 
	printf("-------------------------------------\n");
	printf("Dumping Register Content\n");
	printf("-------------------------------------\n");
	printf("# Instructions Executed\t: %u\n", INSTRUCTION_COUNT);
	printf("PC\t: 0x%08x\n", CURRENT_STATE.PC);
	printf("-------------------------------------\n");
	printf("[Register]\t[Value]\n");
	printf("-------------------------------------\n");
	for (i = 0; i < RISCV_REGS; i++){
		printf("[R%d]\t: 0x%08x\n", i, CURRENT_STATE.REGS[i]);
	}
	printf("-------------------------------------\n");
	printf("[HI]\t: 0x%08x\n", CURRENT_STATE.HI);
	printf("[LO]\t: 0x%08x\n", CURRENT_STATE.LO);
	printf("-------------------------------------\n");
}

/***************************************************************/
/* Read a command from standard input.                                                               */  
/***************************************************************/
void handle_command() {                         
	char buffer[20];
	uint32_t start, stop, cycles;
	uint32_t register_no;
	int register_value;
	int hi_reg_value, lo_reg_value;

	printf("MU-RISCV SIM:> ");

	if (scanf("%s", buffer) == EOF){
		exit(0);
	}

	switch(buffer[0]) {
		case 'S':
		case 's':
			runAll(); 
			break;
		case 'M':
		case 'm':
			if (scanf("%x %x", &start, &stop) != 2){
				break;
			}
			mdump(start, stop);
			break;
		case '?':
			help();
			break;
		case 'Q':
		case 'q':
			printf("**************************\n");
			printf("Exiting MU-RISCV! Good Bye...\n");
			printf("**************************\n");
			exit(0);
		case 'R':
		case 'r':
			if (buffer[1] == 'd' || buffer[1] == 'D'){
				rdump();
			}else if(buffer[1] == 'e' || buffer[1] == 'E'){
				reset();
			}
			else {
				if (scanf("%d", &cycles) != 1) {
					break;
				}
				run(cycles);
			}
			break;
		case 'I':
		case 'i':
			if (scanf("%u %i", &register_no, &register_value) != 2){
				break;
			}
			CURRENT_STATE.REGS[register_no] = register_value;
			NEXT_STATE.REGS[register_no] = register_value;
			break;
		case 'H':
		case 'h':
			if (scanf("%i", &hi_reg_value) != 1){
				break;
			}
			CURRENT_STATE.HI = hi_reg_value; 
			NEXT_STATE.HI = hi_reg_value; 
			break;
		case 'L':
		case 'l':
			if (scanf("%i", &lo_reg_value) != 1){
				break;
			}
			CURRENT_STATE.LO = lo_reg_value;
			NEXT_STATE.LO = lo_reg_value;
			break;
		case 'P':
		case 'p':
			print_program(); 
			break;
		default:
			printf("Invalid Command.\n");
			break;
	}
}

/***************************************************************/
/* reset registers/memory and reload program                                                    */
/***************************************************************/
void reset() {   
	int i;
	/*reset registers*/
	for (i = 0; i < RISCV_REGS; i++){
		CURRENT_STATE.REGS[i] = 0;
	}
	CURRENT_STATE.HI = 0;
	CURRENT_STATE.LO = 0;
	
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
	
	/*load program*/
	load_program();
	
	/*reset PC*/
	INSTRUCTION_COUNT = 0;
	CURRENT_STATE.PC =  MEM_TEXT_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/***************************************************************/
/* Allocate and set memory to zero                                                                            */
/***************************************************************/
void init_memory() {                                           
	int i;
	for (i = 0; i < NUM_MEM_REGION; i++) {
		uint32_t region_size = MEM_REGIONS[i].end - MEM_REGIONS[i].begin + 1;
		MEM_REGIONS[i].mem = malloc(region_size);
		memset(MEM_REGIONS[i].mem, 0, region_size);
	}
}

/**************************************************************/
/* load program into memory                                                                                      */
/**************************************************************/
void load_program() {                   
	FILE * fp;
	int i, word;
	uint32_t address;
	/* Open program file. */
	fp = fopen(prog_file, "r");
	if (fp == NULL) {
		printf("Error: Can't open program file %s\n", prog_file);
		exit(-1);
	}

	/* Read in the program. */

	i = 0;
	while( fscanf(fp, "%x\n", &word) != EOF ) {
		address = MEM_TEXT_BEGIN + i;
		mem_write_32(address, word);
		printf("writing 0x%08x into address 0x%08x (%d)\n", word, address, address);
		i += 4;
	}
	PROGRAM_SIZE = i/4;
	printf("Program loaded into memory.\n%d words written into memory.\n\n", PROGRAM_SIZE);
	fclose(fp);
}

void R_Processing(uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t rs2, uint32_t f7) {
	//printf("internal debugging: rd = %x , f3 = %x , rs1 = %x , rs2 = %x , f7 = %x\n" ,rd,f3,rs1,rs2,f7 );
	switch(f3){
		case 0:
			switch(f7){
				case 0:		//add
					NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] + NEXT_STATE.REGS[rs2];
					break;
				case 32:	//sub
					NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] - NEXT_STATE.REGS[rs2];
					break;
				default:
					RUN_FLAG = FALSE;
					break;
				}	
			break;
		case 1:				//left shift logical
			NEXT_STATE.REGS[rd] = (NEXT_STATE.REGS[rs1] << NEXT_STATE.REGS[rs2]);
			break;
		case 2:				//set less than
			NEXT_STATE.REGS[rd] = (rs1 < rs2)?1:0;
			break;
		case 3:				//set less than unsigned
			NEXT_STATE.REGS[rd] = (rs1 < rs2)?1:0;
			break;
		case 4:				//xor
			NEXT_STATE.REGS[rd] = (NEXT_STATE.REGS[rs1] ^ NEXT_STATE.REGS[rs2]);
			break;
		case 5:
			switch(f7){
				case 0:		//right shift logical
					NEXT_STATE.REGS[rd] = (NEXT_STATE.REGS[rs1] >> rs2);
					break;
				case 32:	//right shift arithmetic
					NEXT_STATE.REGS[rd] = (NEXT_STATE.REGS[rs1] >> rs2);
					break;
			}


		case 6: 			//or
			NEXT_STATE.REGS[rd] = (NEXT_STATE.REGS[rs1] | NEXT_STATE.REGS[rs2]);
			break;
		case 7:				//and
			NEXT_STATE.REGS[rd] = (NEXT_STATE.REGS[rs1] & NEXT_STATE.REGS[rs2]);
			break;
		default:
			RUN_FLAG = FALSE;
			break;
	} 			
}

void ILoad_Processing(uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t imm) {
	// I noticed that this function reads the memory address from NEXT_STATE rather than CURRENT_STATE.
	// That's probably fine since the two should be the same at this point,
	// but it might cause problems in the future if we need to implement pipelining.
	switch (f3)
	{
	case 0: //lb
		NEXT_STATE.REGS[rd] = byte_to_word((mem_read_32(NEXT_STATE.REGS[rs1] + imm)) & 0xFF);
		break;

	case 1: //lh
		NEXT_STATE.REGS[rd] = half_to_word((mem_read_32(NEXT_STATE.REGS[rs1] + imm)) & 0xFFFF);
		break;

	case 2: //lw
		NEXT_STATE.REGS[rd] = mem_read_32(NEXT_STATE.REGS[rs1] + imm);
		break;
	case 4:
		// lbu load byte unsigned
		NEXT_STATE.REGS[rd] = (mem_read_32(NEXT_STATE.REGS[rs1] + imm)) & 0xFF;
		break;
	case 5:
		// lhu load half unsigned
		NEXT_STATE.REGS[rd] = (mem_read_32(NEXT_STATE.REGS[rs1] + imm)) & 0xFFFF;
		break;
	
	default:
		printf("Invalid instruction");
		RUN_FLAG = FALSE;
		break;
	}
}

void Iimm_Processing(uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t imm) {
	uint32_t imm0_4 = (imm << 7) >> 7;
	uint32_t imm5_11 = imm >> 5;
	switch (f3)
	{
	case 0: //addi
		NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] + imm;
		break;

	case 4: //xori
		NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] ^ imm;
		break;
	
	case 6: //ori
		NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] | imm;
		break;
	
	case 7: //andi
		NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] & imm;
		break;
	
	case 1: //slli
		NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] << imm0_4;
		break;
	
	case 5: //srli and srai
		switch (imm5_11)
		{
		case 0: //srli
			NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] >> imm0_4;
			break;

		case 32: //srai
			//NEXT_STATE.REGS[rd] = NEXT_STATE.REGS[rs1] >> imm0_4;
			break;
		
		default:
			RUN_FLAG = FALSE;
			break;
		}
		break;
	
	case 2:		//slti
		NEXT_STATE.REGS[rd] = (rs1 < imm)?1:0;
		break;

	case 3:
		NEXT_STATE.REGS[rd] = (rs1 < imm)?1:0;
		break;

	default:
		printf("Invalid instruction");
		RUN_FLAG = FALSE;
		break;
	}
}

void S_Processing(uint32_t imm4, uint32_t f3, uint32_t rs1, uint32_t rs2, uint32_t imm11) {
	// I noticed that this function reads the memory address from NEXT_STATE rather than CURRENT_STATE.
	// That's probably fine since the two should be the same at this point,
	// but it might cause problems in the future if we need to implement pipelining.
	// Recombine immediate
	uint32_t imm = (imm11 << 5) + imm4;

	switch (f3)
	{
	case 0: //sb
		mem_write_32((NEXT_STATE.REGS[rs1] + imm), NEXT_STATE.REGS[rs2]);
		break;
	
	case 1: //sh
		mem_write_32((NEXT_STATE.REGS[rs1] + imm), NEXT_STATE.REGS[rs2]);
		break;

	case 2: //sw
		mem_write_32((NEXT_STATE.REGS[rs1] + imm), NEXT_STATE.REGS[rs2]);
		break;

	default:
		printf("Invalid instruction");
		RUN_FLAG = FALSE;
		break;
	}
}

void B_Processing() {
	// hi
}

void J_Processing() {
	// hi
}

void U_Processing() {
	// hi
}

void R_print(uint32_t rd, uint32_t f3, uint32_t rs1,uint32_t rs2,uint32_t f7)
{
	char * arg_string;
	switch(f3){
		case 0:
			switch(f7){
				case 0:		//add
					arg_string = "add";
					break;
				case 32:	//sub
					arg_string = "sub";
					break;
				default:
					RUN_FLAG = FALSE;
					break;
				}	
			break;
		case 1:				//left shift logical
			arg_string = "sll";
			break;
		case 2:				//set less than
			arg_string = "slt";
			break;
		case 3:				//set less than unsigned
			arg_string = "sltu";
			break;
		case 4:				//xor
			arg_string = "xor";
			break;
		case 5:
			switch(f7){
				case 0:		//right shift logical
					arg_string = "srl";
					break;
				case 32:	//right shift arithmetic
					arg_string = "sra";
					break;
			}


		case 6:				//or
			arg_string = "or";
			break;
		case 7:				//and
			arg_string = "and";
			break;
		default:
			RUN_FLAG = FALSE;
			break;
	}
	printf("%s x%u x%u x%u\n",arg_string,rd,rs1,rs2);
	
}
void ILoad_print(uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t imm) {
	char * arg_string;
	switch (f3)
	{
	case 0: //lb
		arg_string = "lb";
		break;

	case 1: //lh
		arg_string = "lh";
		break;

	case 2: //lw
		arg_string = "lw";
		break;
	case 4:
		break;
	case 5:
		break;
	
	default:
		printf("Invalid instruction");
		RUN_FLAG = FALSE;
		break;
	}
	printf("%s x%u %u(x%u)\n",arg_string,rd,imm,rs1);
}

void Iimm_print(uint32_t rd, uint32_t f3, uint32_t rs1, uint32_t imm)
{
	uint32_t imm0_4 = (imm << 7) >> 7;
	uint32_t imm5_11 = imm >> 5;

	char * arg_string;
	switch (f3)
	{
	case 0: //addi
		arg_string = "addi";
		break;

	case 4: //xori
		arg_string = "xori";
		break;
	
	case 6: //ori
		arg_string = "ori";
		break;
	
	case 7: //andi
		arg_string = "andi";
		break;
	
	case 1: //slli
		arg_string = "slli";
		break;
	
	case 5: //srli and srai
		switch (imm5_11)
		{
		case 0: //srli
			arg_string = "srli";
			break;

		case 32: //srai
			arg_string = "srai";
			break;
		
		default:
			RUN_FLAG = FALSE;
			break;
		}
		break;
	
	case 2:		//slti
		arg_string = "slti";
		break;

	case 3:		//sktiu
		arg_string = "sltiu";
		break;

	default:
		printf("Invalid instruction");
		RUN_FLAG = FALSE;
		break;
	}
	printf("%s x%u x%u x%u\n",arg_string,rd,rs1,imm0_4);
}

void S_print(uint32_t imm4, uint32_t f3, uint32_t rs1, uint32_t rs2, uint32_t imm11) {
	// Recombine immediate
	uint32_t imm = (imm11 << 5) + imm4;
	char * arg_string;

	switch (f3)
	{
	case 0: //sb
		arg_string = "sb";
		break;
	
	case 1: //sh
		arg_string = "sh";
		break;

	case 2: //sw
		arg_string = "sw";
		break;

	default:
		printf("Invalid instruction");
		RUN_FLAG = FALSE;
		break;
	}
	printf("%s x%u %u(x%u)\n",arg_string,rs2,imm,rs1);
}



static inline uint32_t rd_get(uint32_t instruction)
{
	return (instruction & 0xF80) >> 7;
}

static inline uint32_t funct3_get(uint32_t instruction)
{
	return (instruction & 0x7000) >> 12;
}

static inline uint32_t rs1_get(uint32_t instruction)
{
	return (instruction & 0xf8000) >> 15;
}

static inline uint32_t rs2_get(uint32_t instruction)
{
	return (instruction & 0x1f00000) >> 20;
}

static inline uint32_t funct7_get(uint32_t instruction)
{
	return (instruction & 0xfe000000) >> 25;
}

static inline uint32_t bigImm_get(uint32_t instruction)
{
	return (instruction & 0xfff00000) >> 20;
}

void instruction_map(uint32_t args, bool PRINT_FLAG)
{
	uint8_t type = (uint8_t)(args & 0x3f);
	switch(type)
	{
		case(0x03): //IL
		{
			if(PRINT_FLAG) {ILoad_print(rd_get(args), funct3_get(args), rs1_get(args), bigImm_get(args)); break;}
			ILoad_Processing(rd_get(args) , funct3_get(args) , rs1_get(args) , bigImm_get(args));
			break;
		}
		case(0x13): //Iimm
		{
			if(PRINT_FLAG) { Iimm_print(rd_get(args) , funct3_get(args) , rs1_get(args) , bigImm_get(args)); break;}
			Iimm_Processing(rd_get(args) , funct3_get(args) , rs1_get(args) , bigImm_get(args));
			break;
		}
		case(0x23): //S 
		{
			if(PRINT_FLAG) { S_print(rd_get(args) , funct3_get(args), rs1_get(args) , rs2_get(args), funct7_get(args)); break;}
			S_Processing(rd_get(args) , funct3_get(args), rs1_get(args) , rs2_get(args), funct7_get(args));
			break;
		}
		case(0x33): //R
		{
			if(PRINT_FLAG) { R_print(rd_get(args) , funct3_get(args), rs1_get(args) , rs2_get(args), funct7_get(args)); break;}
			R_Processing(rd_get(args) , funct3_get(args), rs1_get(args) , rs2_get(args), funct7_get(args));
			break;
		}
		default:
			break;

	}
}

/************************************************************/
/* decode and execute instruction                                                                     */ 
/************************************************************/
void handle_instruction()
{
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/
	uint32_t PC = CURRENT_STATE.PC;
	uint32_t instruction = mem_read_32(PC);
	instruction_map(instruction,false);
	NEXT_STATE.PC = PC + 4;
}


/************************************************************/
/* Initialize Memory                                                                                                    */ 
/************************************************************/
void initialize() { 
	init_memory();
	CURRENT_STATE.PC = MEM_TEXT_BEGIN;
	CURRENT_STATE.REGS[2] = MEM_STACK_BEGIN;
	NEXT_STATE = CURRENT_STATE;
	RUN_FLAG = TRUE;
}

/************************************************************/
/* Print the program loaded into memory (in RISCV assembly format)    */ 
/************************************************************/
void print_program(){
	/*IMPLEMENT THIS*/
	/* execute one instruction at a time. Use/update CURRENT_STATE and and NEXT_STATE, as necessary.*/


	uint32_t temp_pc = MEM_TEXT_BEGIN, i = 0;

	while(i < PROGRAM_SIZE){
		uint32_t instruction = mem_read_32(temp_pc);
		instruction_map(instruction,true);
		temp_pc += 4;
		i++;
		//exit loop at some point
	}
}




/************************************************************/
/* Print the instruction at given memory address (in RISCV assembly format)    */
/************************************************************/
void print_instruction(uint32_t addr){

	uint32_t instruction = mem_read_32(addr);
		uint32_t maskopcode = 0x7F;
		uint32_t opcode = instruction & maskopcode;
		if(opcode == 51) { //R-type
			uint32_t maskrd = 0xF80;
			uint32_t rd = instruction & maskrd;
			rd = rd >> 7;
			uint32_t maskf3 = 0x7000;
			uint32_t f3 = instruction & maskf3;
			f3 = f3 >> 12;
			uint32_t maskrs1 = 0xF8000;
			uint32_t rs1 = instruction & maskrs1;
			rs1 = rs1 >> 15;
			uint32_t maskrs2 = 0x1F00000;
			uint32_t rs2 = instruction & maskrs2;
			rs2 = rs2 >> 20;
			uint32_t maskf7 = 0xFE000000;
			uint32_t f7 = instruction & maskf7;
			f7 = f7 >> 25;
			R_print(rd,f3,rs1,rs2,f7);
		} else {
			printf("instruction print not yet created\n");
		}
		CURRENT_STATE = NEXT_STATE;
	return;
}

/***************************************************************/
/* main                                                                                                                                   */
/***************************************************************/
int main(int argc, char *argv[]) {                              
	printf("\n**************************\n");
	printf("Welcome to MU-RISCV SIM...\n");
	printf("**************************\n\n");
	
	if (argc < 2) {
		printf("Error: You should provide input file.\nUsage: %s <input program> \n\n",  argv[0]);
		exit(1);
	}

	strcpy(prog_file, argv[1]);
	initialize();
	load_program();
	help();
	while (1){
		handle_command();
	}
	return 0;
}
