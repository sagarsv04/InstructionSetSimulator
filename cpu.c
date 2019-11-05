/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Sagar Vishwakarma (svishwa2@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
APEX_CPU* APEX_cpu_init(const char* filename) {

  if (!filename) {
    return NULL;
  }
  // memory allocation of struct APEX_CPU to struct pointer cpu
  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);  // fill a block of memory with a particular value here value is 0 for 32 regs with size 4 Bytes
  memset(cpu->regs_valid, 1, sizeof(int) * 32);  // all registers are valid at start, set to value 1
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES); // all values in stage struct of type CPU_Stage like pc, rs1, etc are set to 0
  memset(cpu->data_memory, 0, sizeof(int) * 4000); // from 4000 to 4095 there will be garbage values in data_memory array

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu); // If code_memory is not created free the memory for cpu struct
    return NULL;
  }
  // Below code just prints the instructions and operands before execution
  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void APEX_cpu_stop(APEX_CPU* cpu) {

  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int get_code_index(int pc) {

  return (pc - 4000) / 4;
}

static void print_instruction(CPU_Stage* stage) {

  if (strcmp(stage->opcode, "STORE") == 0) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }
}

/* Get Reg values function
 *
 */
static void get_reg_values(APEX_CPU* cpu, CPU_Stage* stage, int src_reg_pos, int src_reg) {
  if (src_reg_pos == 1) {
    stage->rs1_value = cpu->regs[src_reg];
  }
  else if (src_reg_pos == 2) {
    stage->rs2_value = cpu->regs[src_reg];
  }
  else {
    ;// Nothing
  }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void print_stage_content(char* name, CPU_Stage* stage) {

  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int fetch(APEX_CPU* cpu) {

  CPU_Stage* stage = &cpu->stage[F];
  if (!stage->busy && !stage->stalled) {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    // stage->rd = current_ins->rd; // written twice

    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    cpu->stage[DRF] = cpu->stage[F]; // this is cool

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", stage);
    }
  }
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int decode(APEX_CPU* cpu) {

  CPU_Stage* stage = &cpu->stage[DRF];
  if (!stage->busy && !stage->stalled) {

    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      // read literal and register values
      get_reg_values(cpu, stage, 1, stage->rs1);
      stage->buffer = stage->imm; // keeping literal value in buffer to calculate mem add in exe stage
    }
    else if (strcmp(stage->opcode, "STR") == 0) {
      // read only values of last two registors
      get_reg_values(cpu, stage, 1, stage->rs1); // Here rd becomes src1 and src2, src3 are rs1, rs2
      get_reg_values(cpu, stage, 2, stage->rs2);
    }
    else if (strcmp(stage->opcode, "LOAD") == 0) {
      // read literal and register values
      get_reg_values(cpu, stage, 1, stage->rs1);
      stage->buffer = stage->imm; // keeping literal value in buffer to calculate mem add in exe stage
    }
    else if (strcmp(stage->opcode, "LDR") == 0) {
      // read only values of last two registors
      get_reg_values(cpu, stage, 1, stage->rs1);
      get_reg_values(cpu, stage, 2, stage->rs2);
    }
    /* No Register file read needed for MOVC */
    else if (strcmp(stage->opcode, "MOVC") == 0) { // this is MOV Constant to Register
      // read literal values
      stage->buffer = stage->imm; // keeping literal value in buffer to load in mem stage
    }
    else if (strcmp(stage->opcode, "MOV") == 0) { // this is MOV one Reg value to another Reg
      // read register values
      get_reg_values(cpu, stage, 1, stage->rs1);
    }
    else if (strcmp(stage->opcode, "ADD") == 0) {
      // read only values of last two registors
      get_reg_values(cpu, stage, 1, stage->rs1);
      get_reg_values(cpu, stage, 2, stage->rs2);
    }
    else if (strcmp(stage->opcode, "ADDL") == 0) {
      // read only values of last two registors
      get_reg_values(cpu, stage, 1, stage->rs1);
      stage->buffer = stage->imm; // keeping literal value in buffer to add in exe stage
    }
    else if (strcmp(stage->opcode, "SUB") == 0) {
      // read only values of last two registors
      get_reg_values(cpu, stage, 1, stage->rs1);
      get_reg_values(cpu, stage, 2, stage->rs2);
    }
    else if (strcmp(stage->opcode, "SUBL") == 0) {
      // read only values of last two registors
      get_reg_values(cpu, stage, 1, stage->rs1);
      stage->buffer = stage->imm; // keeping literal value in buffer to sub in exe stage
    }
    else if (strcmp(stage->opcode, "MUL") == 0) {
      // read only values of last two registors
      get_reg_values(cpu, stage, 1, stage->rs1);
      get_reg_values(cpu, stage, 2, stage->rs2);
    }
    else if (strcmp(stage->opcode, "DIV") == 0) {
      // read only values of last two registors
      get_reg_values(cpu, stage, 1, stage->rs1);
      get_reg_values(cpu, stage, 2, stage->rs2);
    }
    else if (strcmp(stage->opcode, "BZ") == 0) {
      // read literal values
      stage->buffer = stage->imm; // keeping literal value in buffer to jump in exe stage
    }
    else if (strcmp(stage->opcode, "BNZ") == 0) {
      // read literal values
      stage->buffer = stage->imm; // keeping literal value in buffer to jump in exe stage
    }
    else if (strcmp(stage->opcode, "JUMP") == 0) {
      // read literal and register values
      get_reg_values(cpu, stage, 1, stage->rs1);
      stage->buffer = stage->imm; // keeping literal value in buffer to cal memory to jump in exe stage
    }
    else if (strcmp(stage->opcode, "HALT") == 0) {
      ; // Nothing
    }
    else if (strcmp(stage->opcode, "NOP") == 0) {
      ; // Nothing
    }
    else {
      fprintf(stderr, "Decode/RF Invalid Instruction Found :: %s\n", stage->opcode);
    }
    /* Copy data from decode latch to execute latch*/
    cpu->stage[EX] = cpu->stage[DRF];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode/RF", stage);
    }
  }
  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int execute(APEX_CPU* cpu) {

  CPU_Stage* stage = &cpu->stage[EX];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM] = cpu->stage[EX];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute", stage);
    }
  }
  return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int memory(APEX_CPU* cpu) {

  CPU_Stage* stage = &cpu->stage[MEM];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
    }

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory", stage);
    }
  }
  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int writeback(APEX_CPU* cpu) {

  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled) {

    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
    }

    cpu->ins_completed++;

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Writeback", stage);
    }
  }
  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int APEX_cpu_run(APEX_CPU* cpu) {

  while (1) {

    /* All the instructions committed, so exit */
    if (cpu->ins_completed == cpu->code_memory_size) { // check number of instruction executed to break from while loop
      printf("(apex) >> Simulation Complete");
      break;
    }

    cpu->clock++; // places here so we can see prints aligned with executions

    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }

    // why we are executing from behind ??

    writeback(cpu);
    memory(cpu);
    execute(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;

    /*
    fetch(cpu);
    decode(cpu);
    execute(cpu);
    memory(cpu);
    writeback(cpu);
    */
  }

  return 0;
}
