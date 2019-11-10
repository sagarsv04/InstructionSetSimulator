#ifndef _APEX_CPU_H_
#define _APEX_CPU_H_
/**
 *  cpu.h
 *  Contains various CPU and Pipeline Data structures
 *
 *  Author :
 *  Sagar Vishwakarma (svishwa2@binghamton.edu)
 *  State University of New York, Binghamton
 */
#define DATA_MEMORY_SIZE 4096
#define REGISTER_FILE_SIZE 32

enum {
  F,
  DRF,
  EX_ONE,
  EX_TWO,
  MEM_ONE,
  MEM_TWO,
  WB,
  NUM_STAGES
};

enum {
  SUCCESS,
  HALT,
  ERROR,
  NUM_EXIT
};

/* Index of Flags */
enum {
  ZF, // Zero Flag index
  CF, // Carry Flag index
  OF, // Overflow Flag index
  IF, // Interrupt Flag index
  NUM_FLAG
};

/* Format of an APEX instruction  */
typedef struct APEX_Instruction {
  char opcode[128];	// Operation Code
  int rd;           // Destination Register Address
  int rs1;          // Source-1 Register Address
  int rs2;          // Source-2 Register Address
  int imm;          // Literal Value
} APEX_Instruction;

/* Model of CPU stage latch */
typedef struct CPU_Stage {
  int pc;           // Program Counter
  char opcode[128]; // Operation Code
  int rs1;          // Source-1 Register Address
  int rs2;          // Source-2 Register Address
  int rd;           // Destination Register Address
  int imm;          // Literal Value
  int rs1_value;    // Source-1 Register Value
  int rs2_value;    // Source-2 Register Value
  int rd_value;     // Destination Register Value
  int buffer;       // Latch to hold some value  (currently used to hold literal value from decode)
  int mem_address;  // Computed Memory Address
  int busy;         // Flag to indicate, stage is performing some action
  int stalled;      // Flag to indicate, stage is stalled
  int executed;     // Flag to indicate, stage has executed or not
} CPU_Stage;

/* Model of APEX CPU */
typedef struct APEX_CPU {
  /* Clock cycles elasped */
  int clock;

  /* Current program counter */
  int pc;

  /* Integer register file */
  int regs[REGISTER_FILE_SIZE];
  int regs_invalid[REGISTER_FILE_SIZE];

  /* Array of 5 CPU_stage */
  CPU_Stage stage[NUM_STAGES]; // array of 5 CPU_Stage struct. Note: use . in struct with variable names, use -> when its a pointer

  /* Code Memory where instructions are stored */
  APEX_Instruction* code_memory;  // APEX_Instruction struct pointer code_memory

  int flags[NUM_FLAG];

  int code_memory_size;

  /* Data Memory */
  int data_memory[DATA_MEMORY_SIZE];

  /* Some stats */
  int ins_completed;

} APEX_CPU;

APEX_Instruction* create_code_memory(const char* filename, int* size);

APEX_CPU* APEX_cpu_init(const char* filename);

int APEX_cpu_run(APEX_CPU* cpu, int num_instruction);

void APEX_cpu_stop(APEX_CPU* cpu);

int fetch(APEX_CPU* cpu);

int decode(APEX_CPU* cpu);

int execute_one(APEX_CPU* cpu);

int execute_two(APEX_CPU* cpu);

int memory_one(APEX_CPU* cpu);

int memory_two(APEX_CPU* cpu);

int writeback(APEX_CPU* cpu);

#endif
