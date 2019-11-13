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
#include <limits.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1

/* Set this flag to 1 to enable print of Regs, Flags, Memory */
#define ENABLE_REG_MEM_STATUS_PRINT 1
#define ENABLE_PUSH_STAGE_PRINT 1

/*
 * ########################################## Initialize CPU ##########################################
 */

APEX_CPU* APEX_cpu_init(const char* filename) {
  // This function creates and initializes APEX cpu.
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
  memset(cpu->regs, 0, sizeof(int) * REGISTER_FILE_SIZE);  // fill a block of memory with a particular value here value is 0 for 32 regs with size 4 Bytes
  memset(cpu->regs_invalid, 0, sizeof(int) * REGISTER_FILE_SIZE);  // all registers are valid at start, set to value 1
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES); // all values in stage struct of type CPU_Stage like pc, rs1, etc are set to 0
  memset(cpu->data_memory, 0, sizeof(int) * 4000); // from 4000 to 4095 there will be garbage values in data_memory array
  memset(cpu->flags, 0, sizeof(int) * NUM_FLAG); // all flag values in cpu are set to 0

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
    cpu->stage[i].empty = 1;
  }

  return cpu;
}

void APEX_cpu_stop(APEX_CPU* cpu) {
  // This function de-allocates APEX cpu.
  free(cpu->code_memory);
  free(cpu);
}

/*
 * ########################################## Initialize CPU End ##########################################
 */

int get_code_index(int pc) {
  // Converts the PC(4000 series) into array index for code memory
  // First instruction index is 0
  return (pc - 4000) / 4;
}

static void print_instruction(CPU_Stage* stage) {
  // This function prints operands of instructions in stages.
  if (strcmp(stage->opcode, "STORE") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }
  else if (strcmp(stage->opcode, "STR") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  else if (strcmp(stage->opcode, "LOAD") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }
  else if (strcmp(stage->opcode, "LDR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  else if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }
  else if (strcmp(stage->opcode, "MOV") == 0) {
    printf("%s,R%d,R%d ", stage->opcode, stage->rd, stage->rs1);
  }
  else if (strcmp(stage->opcode, "ADD") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  else if (strcmp(stage->opcode, "ADDL") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }
  else if (strcmp(stage->opcode, "SUB") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  else if (strcmp(stage->opcode, "SUBL") == 0) {
    printf("%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }
  else if (strcmp(stage->opcode, "MUL") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  else if (strcmp(stage->opcode, "DIV") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  else if (strcmp(stage->opcode, "AND") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  else if (strcmp(stage->opcode, "OR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  else if (strcmp(stage->opcode, "EX-OR") == 0) {
    printf("%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
  }
  else if (strcmp(stage->opcode, "BZ") == 0) {
    printf("%s,#%d ", stage->opcode, stage->imm);
  }
  else if (strcmp(stage->opcode, "BNZ") == 0) {
    printf("%s,#%d ", stage->opcode, stage->imm);
  }
  else if (strcmp(stage->opcode, "JUMP") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rs1, stage->imm);
  }
  else if (strcmp(stage->opcode, "HALT") == 0) {
    printf("%s ", stage->opcode);
  }
  else if (strcmp(stage->opcode, "NOP") == 0) {
    printf("%s ", stage->opcode);
  }
}

static void print_stage_status(CPU_Stage* stage) {
  // This function prints status of stages.
  if (stage->empty) {
    printf(" ---> EMPTY ");
  }
  else if (stage->stalled) {
    printf(" ---> STALLED ");
  }
  else if (stage->busy){
    printf(" ---> BUSY ");
  }
}

static void print_stage_content(char* name, CPU_Stage* stage) {
  // Print function which prints contents of stage
  printf("%-15s: %d: pc(%d) ", name, stage->executed, stage->pc);
  print_instruction(stage);
  print_stage_status(stage);
  printf("\n");
}

void print_cpu_content(APEX_CPU* cpu) {
  // Print function which prints contents of cpu memory
  if (ENABLE_REG_MEM_STATUS_PRINT) {
    printf("============ STATE OF CPU FLAGS ============\n");
    // print all Flags
    printf("Falgs::  ZeroFlag, CarryFlag, OverflowFlag, InterruptFlag\n");
    printf("Values:: %d,\t|\t%d,\t|\t%d,\t|\t%d\n", cpu->flags[ZF],cpu->flags[CF],cpu->flags[OF],cpu->flags[IF]);

    // print all regs along with valid bits
    printf("============ STATE OF ARCHITECTURAL REGISTER FILE ============\n");
    printf("NOTE :: 0 Means Valid & 1 Means Invalid\n");
    printf("Registers, Values, Invalid\n");
    for (int i=0;i<REGISTER_FILE_SIZE;i++) {
      printf("R%02d,\t|\t%02d,\t|\t%d\n", i, cpu->regs[i], cpu->regs_invalid[i]);
    }

    // print 100 memory location
    printf("============ STATE OF DATA MEMORY ============\n");
    printf("Mem Location, Values\n");
    for (int i=0;i<100;i++) {
      printf("M%02d,\t|\t%02d\n", i, cpu->data_memory[i]);
    }
    printf("\n");
  }
}

static int get_reg_values(APEX_CPU* cpu, CPU_Stage* stage, int src_reg_pos, int src_reg) {
  // Get Reg values function
  int value = 0;
  if (src_reg_pos == 0) {
    value = cpu->regs[src_reg];
  }
  else if (src_reg_pos == 1) {
    value = cpu->regs[src_reg];
  }
  else if (src_reg_pos == 2) {
    value = cpu->regs[src_reg];
  }
  else {
    ;// Nothing
  }
  return value;
}

static int get_reg_status(APEX_CPU* cpu, int reg_number) {
  // Get Reg Status function
  int status = 1; // 1 is invalid
  if (reg_number > REGISTER_FILE_SIZE) {
    // Segmentation fault
    fprintf(stderr, "Segmentation fault for Register location :: %d\n", reg_number);
  }
  else {
    status = cpu->regs_invalid[reg_number];
  }
  return status;
}

static void set_reg_status(APEX_CPU* cpu, int reg_number, int status) {
  // Set Reg Status function
  if (reg_number > REGISTER_FILE_SIZE) {
    // Segmentation fault
    fprintf(stderr, "Segmentation fault for Register location :: %d\n", reg_number);
  }
  else {
    cpu->regs_invalid[reg_number] = status;
  }
}

static void add_bubble_to_stage(APEX_CPU* cpu, int stage_index, int flushed) {
  // Add bubble to cpu stage
   if (flushed){
       strcpy(cpu->stage[stage_index].opcode, "NOP"); // add a Bubble
       cpu->code_memory_size = cpu->code_memory_size + 1;
       cpu->stage[stage_index].empty = 1;
       // this is because while checking for forwarding we dont look if EX_TWO or MEM_TWO has NOP
       // we simply compare rd value to know if this register is wat we are looking for
       // assuming no source register will be negative
       cpu->stage[stage_index].rd = -99;
   }
  if ((stage_index > F) && (stage_index < NUM_STAGES) && !(flushed)) {
    // No adding Bubble in Fetch and WB stage
    if (cpu->stage[stage_index].executed) {
      strcpy(cpu->stage[stage_index].opcode, "NOP"); // add a Bubble
      cpu->code_memory_size = cpu->code_memory_size + 1;
      // this is because while checking for forwarding we dont look if EX_TWO or MEM_TWO has NOP
      // we simply compare rd value to know if this register is wat we are looking for
      // assuming no source register will be negative
      cpu->stage[stage_index].rd = -99;
    }
    else {
      ; // Nothing let it execute its current instruction
    }
  }
  else {
    ;
  }
}

int previous_arithmetic_check(APEX_CPU* cpu) {

  int status = 0;
  int a = 0;
  for (int i=EX_ONE;i<WB; i++) {
    if (strcmp(cpu->stage[i].opcode, "NOP") != 0) {
      a = i;
      break;
    }
  }

  if (a!=0){
    if ((strcmp(cpu->stage[a].opcode, "ADD") == 0) ||
      (strcmp(cpu->stage[a].opcode, "ADDL") == 0) ||
      (strcmp(cpu->stage[a].opcode, "SUB") == 0) ||
      (strcmp(cpu->stage[a].opcode, "SUBL") == 0) ||
      (strcmp(cpu->stage[a].opcode, "MUL") == 0) || (strcmp(cpu->stage[EX_ONE].opcode, "DIV") == 0)) {

      status = 1;
    }
  }

  return status;
}

/*
 * ########################################## Fetch Stage ##########################################
 */
int fetch(APEX_CPU* cpu) {

  cpu->stage[F].executed = 0;
  CPU_Stage* stage = &cpu->stage[F];
  if (!stage->busy && !stage->stalled) {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into fetch latch */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;

    /* Copy data from Fetch latch to Decode latch*/
    cpu->stage[F].executed = 1;
    // cpu->stage[DRF] = cpu->stage[F]; // this is cool I should empty the fetch stage as well to avoid repetition ?
    // cpu->stage[DRF].executed = 0;
    if (strcmp(cpu->stage[F].opcode, "") == 0) {
      // stop fetching Instructions, exit from writeback stage
      cpu->stage[F].stalled = 0;
      cpu->stage[F].empty = 1;
    }
    else {
      /* Update PC for next instruction */
      cpu->pc += 4;
      cpu->stage[F].empty = 0;
    }
  }
  if (cpu->stage[F].stalled) {
    //If Fetch has HALT and Decode has HALT fetch only one Inst
    if (strcmp(cpu->stage[DRF].opcode, "HALT") == 0){
      // just fetch the next instruction
      stage->pc = cpu->pc;
      APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
      strcpy(stage->opcode, current_ins->opcode);
      stage->rd = current_ins->rd;
      stage->rs1 = current_ins->rs1;
      stage->rs2 = current_ins->rs2;
      stage->imm = current_ins->imm;
    }
  }

  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Fetch", stage);
  }

  return 0;
}

/*
 * ########################################## Decode Stage ##########################################
 */
int decode(APEX_CPU* cpu) {

  cpu->stage[DRF].executed = 0;
  CPU_Stage* stage = &cpu->stage[DRF];
  // decode stage only has power to stall itself and Fetch stage
  // unstalling will happen in Mem_two or Writeback stage
  if (!stage->busy && !stage->stalled) {
    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {

      if (!get_reg_status(cpu, stage->rd) && !get_reg_status(cpu, stage->rs1)) {
        // read literal and register values
        stage->rd_value = get_reg_values(cpu, stage, 0, stage->rd);
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->buffer = stage->imm; // keeping literal value in buffer to calculate mem add in exe stage
      }
      else {
      // keep DF and Fetch Stage in stall if regs_invalid is set
      cpu->stage[DRF].stalled = 1;
      cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "STR") == 0) {
      // read only values of last two registers
      if (!get_reg_status(cpu, stage->rd) && !get_reg_status(cpu, stage->rs1) && !get_reg_status(cpu, stage->rs2)) {
        stage->rd_value = get_reg_values(cpu, stage, 0, stage->rd);
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1); // Here rd becomes src1 and src2, src3 are rs1, rs2
        stage->rs2_value = get_reg_values(cpu, stage, 2, stage->rs2);
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "LOAD") == 0) {
      // read literal and register values
      if (!get_reg_status(cpu, stage->rs1)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->buffer = stage->imm; // keeping literal value in buffer to calculate mem add in exe stage
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "LDR") == 0) {
      // read only values of last two registers
      if (!get_reg_status(cpu, stage->rs1) && !get_reg_status(cpu, stage->rs2)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->rs2_value = get_reg_values(cpu, stage, 2, stage->rs2);
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    /* No Register file read needed for MOVC */
    else if (strcmp(stage->opcode, "MOVC") == 0) { // this is MOV Constant to Register
      // read literal values
      stage->buffer = stage->imm; // keeping literal value in buffer to load in mem stage
    }
    else if (strcmp(stage->opcode, "MOV") == 0) { // this is MOV one Reg value to another Reg
      // read register values
      if (!get_reg_status(cpu, stage->rs1)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "ADD") == 0) {
      // read only values of last two registers
      if (!get_reg_status(cpu, stage->rs1) && !get_reg_status(cpu, stage->rs2)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->rs2_value = get_reg_values(cpu, stage, 2, stage->rs2);
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "ADDL") == 0) {
      // read only values of last two registers
      if (!get_reg_status(cpu, stage->rs1)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->buffer = stage->imm; // keeping literal value in buffer to add in exe stage
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "SUB") == 0) {
      // read only values of last two registers
      if (!get_reg_status(cpu, stage->rs1) && !get_reg_status(cpu, stage->rs2)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->rs2_value = get_reg_values(cpu, stage, 2, stage->rs2);
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "SUBL") == 0) {
      // read only values of last two registers
      if (!get_reg_status(cpu, stage->rs1)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->buffer = stage->imm; // keeping literal value in buffer to sub in exe stage
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "MUL") == 0) {
      // read only values of last two registers
      if (!get_reg_status(cpu, stage->rs1) && !get_reg_status(cpu, stage->rs2)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->rs2_value = get_reg_values(cpu, stage, 2, stage->rs2);
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "DIV") == 0) {
      // read only values of last two registers
      if (!get_reg_status(cpu, stage->rs1) && !get_reg_status(cpu, stage->rs2)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->rs2_value = get_reg_values(cpu, stage, 2, stage->rs2);
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "AND") == 0) {
      // read only values of last two registers
      if (!get_reg_status(cpu, stage->rs1) && !get_reg_status(cpu, stage->rs2)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->rs2_value = get_reg_values(cpu, stage, 2, stage->rs2);
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "OR") == 0) {
      // read only values of last two registers
      if (!get_reg_status(cpu, stage->rs1) && !get_reg_status(cpu, stage->rs2)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->rs2_value = get_reg_values(cpu, stage, 2, stage->rs2);
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "EX-OR") == 0) {
      // read only values of last two registers
      if (!get_reg_status(cpu, stage->rs1) && !get_reg_status(cpu, stage->rs2)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->rs2_value = get_reg_values(cpu, stage, 2, stage->rs2);
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "BZ") == 0) {
      // read literal values
      stage->buffer = stage->imm; // keeping literal value in buffer to jump in exe stage
      if (previous_arithmetic_check(cpu)) {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "BNZ") == 0) {
      // read literal values
      stage->buffer = stage->imm; // keeping literal value in buffer to jump in exe stage
      if (previous_arithmetic_check(cpu)) {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "JUMP") == 0) {
      // read literal and register values
      if (!get_reg_status(cpu, stage->rs1) && !get_reg_status(cpu, stage->rs2)) {
        stage->rs1_value = get_reg_values(cpu, stage, 1, stage->rs1);
        stage->buffer = stage->imm; // keeping literal value in buffer to cal memory to jump in exe stage
      }
      else {
        // keep DF and Fetch Stage in stall if regs_invalid is set
        cpu->stage[DRF].stalled = 1;
        cpu->stage[F].stalled = 1;
      }
    }
    else if (strcmp(stage->opcode, "HALT") == 0) {
      // Halt causes a type of Intrupt where Fetch is stalled and cpu intrupt Bit is Set
      // Stop fetching new instruction but allow all the instruction to go from Decode Writeback
      cpu->stage[F].stalled = 1; // add NOP from fetch stage
      cpu->flags[IF] = 1; // Halt as Interrupt
    }
    else if (strcmp(stage->opcode, "NOP") == 0) {
      ; // Nothing
    }
    else {
      if (strcmp(stage->opcode, "") != 0) {
        fprintf(stderr, "Decode/RF Invalid Instruction Found :: %s\n", stage->opcode);
      }
    }
    cpu->stage[DRF].executed = 1;
  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Decode/RF", stage);
  }

  return 0;
}

/*
 * ########################################## EX One Stage ##########################################
 */
int execute_one(APEX_CPU* cpu) {

  cpu->stage[EX_ONE].executed = 0;
  CPU_Stage* stage = &cpu->stage[EX_ONE];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      // create memory address using literal and register values
      stage->mem_address = stage->rs1_value + stage->buffer;
    }
    else if (strcmp(stage->opcode, "STR") == 0) {
      // create memory address using register values
      stage->mem_address = stage->rs1_value + stage->rs2_value;
    }
    else if (strcmp(stage->opcode, "LOAD") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "LDR") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    /* MOVC */
    else if (strcmp(stage->opcode, "MOVC") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "MOV") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "ADD") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "ADDL") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "SUB") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "SUBL") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "MUL") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "DIV") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "AND") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "OR") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "EX-OR") == 0) {
      set_reg_status(cpu, stage->rd, 1); // make desitination regs invalid so following instructions stall
    }
    else if (strcmp(stage->opcode, "BZ") == 0) {
      ; // flush all the previous stages and start fetching instruction from mem_address in execute_two
    }
    else if (strcmp(stage->opcode, "BNZ") == 0) {
      ; // flush all the previous stages and start fetching instruction from mem_address in execute_two
    }
    else if (strcmp(stage->opcode, "JUMP") == 0) {
      ; // flush all the previous stages and start fetching instruction from mem_address in execute_two
    }
    else if (strcmp(stage->opcode, "HALT") == 0) {
      ; // treat Halt as an interrupt stoped fetching instructions
    }
    else if (strcmp(stage->opcode, "NOP") == 0) {
      ; // Do nothing its just a bubble
    }
    else {
      ; // Do nothing
    }
    cpu->stage[EX_ONE].executed = 1;
  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Execute One", stage);
  }

  return 0;
}

/*
 * ########################################## EX Two Stage ##########################################
 */
int execute_two(APEX_CPU* cpu) {

  cpu->stage[EX_TWO].executed = 0;
  CPU_Stage* stage = &cpu->stage[EX_TWO];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      // create memory address using literal and register values
      stage->mem_address = stage->rs1_value + stage->buffer;
    }
    else if (strcmp(stage->opcode, "STR") == 0) {
      // create memory address using register values
      stage->mem_address = stage->rs1_value + stage->rs2_value;
    }
    else if (strcmp(stage->opcode, "LOAD") == 0) {
      // create memory address using literal and register values
      stage->mem_address = stage->rs1_value + stage->buffer;
    }
    else if (strcmp(stage->opcode, "LDR") == 0) {
      // create memory address using register values
      stage->mem_address = stage->rs1_value + stage->rs2_value;
    }
    /* MOVC */
    else if (strcmp(stage->opcode, "MOVC") == 0) {
      stage->rd_value = stage->buffer; // move buffer value to rd_value so it can be forwarded
    }
    else if (strcmp(stage->opcode, "MOV") == 0) {
      stage->rd_value = stage->rs1_value; // move rs1_value value to rd_value so it can be forwarded
    }
    else if (strcmp(stage->opcode, "ADD") == 0) {
      // add registers value and keep in rd_value for mem / writeback stage
      if ((stage->rs2_value > 0 && stage->rs1_value > INT_MAX - stage->rs2_value) ||
        (stage->rs2_value < 0 && stage->rs1_value < INT_MIN - stage->rs2_value)) {
        cpu->flags[OF] = 1; // there is an overflow
      }
      else {
        stage->rd_value = stage->rs1_value + stage->rs2_value;
        cpu->flags[OF] = 0; // there is no overflow
      }
    }
    else if (strcmp(stage->opcode, "ADDL") == 0) {
      // add literal and register value and keep in rd_value for mem / writeback stage
      if ((stage->buffer > 0 && stage->rs1_value > INT_MAX - stage->buffer) ||
        (stage->buffer < 0 && stage->rs1_value < INT_MIN - stage->buffer)) {
        cpu->flags[OF] = 1; // there is an overflow
      }
      else {
        stage->rd_value = stage->rs1_value + stage->buffer;
        cpu->flags[OF] = 0; // there is no overflow
      }
    }
    else if (strcmp(stage->opcode, "SUB") == 0) {
      // sub registers value and keep in rd_value for mem / writeback stage
      if (stage->rs2_value > stage->rs1_value) {
        stage->rd_value = stage->rs1_value - stage->rs2_value;
        cpu->flags[CF] = 1; // there is an carry
      }
      else {
        stage->rd_value = stage->rs1_value - stage->rs2_value;
        cpu->flags[CF] = 0; // there is no carry
      }
    }
    else if (strcmp(stage->opcode, "SUBL") == 0) {
      // sub literal and register value and keep in rd_value for mem / writeback stage
      if (stage->buffer > stage->rs1_value) {
        stage->rd_value = stage->rs1_value - stage->buffer;
        cpu->flags[CF] = 1; // there is an carry
      }
      else {
        stage->rd_value = stage->rs1_value - stage->buffer;
        cpu->flags[CF] = 0; // there is no carry
      }
    }
    else if (strcmp(stage->opcode, "MUL") == 0) {
      // mul registers value and keep in rd_value for mem / writeback stage
      stage->rd_value = stage->rs1_value * stage->rs2_value;
    }
    else if (strcmp(stage->opcode, "DIV") == 0) {
      // div registers value and keep in rd_value for mem / writeback stage
      if (stage->rs2_value != 0) {
        stage->rd_value = stage->rs1_value / stage->rs2_value;
      }
      else {
        fprintf(stderr, "Division By Zero Returning Value Zero\n");
        stage->rd_value = 0;
      }
    }
    else if (strcmp(stage->opcode, "AND") == 0) {
      // mul registers value and keep in rd_value for mem / writeback stage
      stage->rd_value = stage->rs1_value & stage->rs2_value;
    }
    else if (strcmp(stage->opcode, "OR") == 0) {
      // or registers value and keep in rd_value for mem / writeback stage
      stage->rd_value = stage->rs1_value | stage->rs2_value;
    }
    else if (strcmp(stage->opcode, "EX-OR") == 0) {
      // ex-or registers value and keep in rd_value for mem / writeback stage
      stage->rd_value = stage->rs1_value ^ stage->rs2_value;
    }
    else if (strcmp(stage->opcode, "BZ") == 0) {
      // load buffer value to mem_address
      stage->mem_address = stage->buffer;
      if (cpu->flags[ZF]) {
        // check address validity, pc-add % 4 should be 0
        if (((stage->pc + stage->mem_address)%4 == 0)&&!((stage->pc + stage->mem_address) < 4000)) {
          // reset status of rd in exe_one stage
          set_reg_status(cpu, cpu->stage[EX_ONE].rd, 0); // make desitination regs valid so following instructions won't stall
          // flush previous instructions add NOP
          add_bubble_to_stage(cpu, EX_ONE, 1); // next cycle Bubble will be executed
          add_bubble_to_stage(cpu, DRF, 1); // next cycle Bubble will be executed
          add_bubble_to_stage(cpu, F, 1); // next cycle Bubble will be executed
          // change pc value
          cpu->pc = stage->pc + stage->mem_address;
          // un stall Fetch and Decode stage if they are stalled
          cpu->stage[DRF].stalled = 0;
          cpu->stage[F].stalled = 0;
        }
        else {
          fprintf(stderr, "Invalid Branch Loction for %s\n", stage->opcode);
          fprintf(stderr, "Instruction %s Relative Address %d\n", stage->opcode, cpu->pc + stage->mem_address);
        }
      }
    }
    else if (strcmp(stage->opcode, "BNZ") == 0) {
      // load buffer value to mem_address
      stage->mem_address = stage->buffer;
      if (!cpu->flags[ZF]) {
        // check address validity, pc-add % 4 should be 0
        if (((stage->pc + stage->mem_address)%4 == 0)&&!((stage->pc + stage->mem_address) < 4000)) {
          // reset status of rd in exe_one stage
          set_reg_status(cpu, cpu->stage[EX_ONE].rd, 0); // make desitination regs valid so following instructions won't stall
          // flush previous instructions add NOP
          add_bubble_to_stage(cpu, EX_ONE, 1); // next cycle Bubble will be executed
          add_bubble_to_stage(cpu, DRF, 1); // next cycle Bubble will be executed
          add_bubble_to_stage(cpu, F, 1); // next cycle Bubble will be executed
          // change pc value
          cpu->pc = stage->pc + stage->mem_address;
          // un stall Fetch and Decode stage if they are stalled
          cpu->stage[DRF].stalled = 0;
          cpu->stage[F].stalled = 0;
        }
        else {
          fprintf(stderr, "Invalid Branch Loction for %s\n", stage->opcode);
          fprintf(stderr, "Instruction %s Relative Address %d\n", stage->opcode, cpu->pc + stage->mem_address);
        }
      }
    }
    else if (strcmp(stage->opcode, "JUMP") == 0) {
      // load buffer value to mem_address
      stage->mem_address = stage->rs1_value + stage->buffer;
      // check address validity, pc-add % 4 should be 0
      if (((stage->pc + stage->mem_address)%4 == 0)&&!((stage->pc + stage->mem_address) < 4000)) {
        // reset status of rd in exe_one stage
        set_reg_status(cpu, cpu->stage[EX_ONE].rd, 0); // make desitination regs valid so following instructions won't stall
        // flush previous instructions add NOP
        add_bubble_to_stage(cpu, EX_ONE, 1); // next cycle Bubble will be executed
        add_bubble_to_stage(cpu, DRF, 1); // next cycle Bubble will be executed
        add_bubble_to_stage(cpu, F, 1); // next cycle Bubble will be executed
        // change pc value
        cpu->pc = stage->pc + stage->mem_address;
        // un stall Fetch and Decode stage if they are stalled
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
      else {
        fprintf(stderr, "Invalid Branch Loction for %s\n", stage->opcode);
        fprintf(stderr, "Instruction %s Relative Address %d\n", stage->opcode, cpu->pc + stage->mem_address);
      }
    }
    else if (strcmp(stage->opcode, "HALT") == 0) {
      ; // treat Halt as an interrupt stoped fetching instructions
    }
    else if (strcmp(stage->opcode, "NOP") == 0) {
      ; // Do nothing its just a bubble
    }
    else {
      ; // Do nothing
    }
    cpu->stage[EX_TWO].executed = 1;
  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Execute Two", stage);
  }

  return 0;
}

/*
 * ########################################## Mem One Stage ##########################################
 */
int memory_one(APEX_CPU* cpu) {

  cpu->stage[MEM_ONE].executed = 0;
  CPU_Stage* stage = &cpu->stage[MEM_ONE];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      // use memory address and write value in data_memory
      if (stage->mem_address > DATA_MEMORY_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for writing memory location :: %d\n", stage->mem_address);
      }
      else {
        cpu->data_memory[stage->mem_address] = stage->rd_value;
      }
    }
    else if (strcmp(stage->opcode, "STR") == 0) {
      // use memory address and write value in data_memory
      if (stage->mem_address > DATA_MEMORY_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for writing memory location :: %d\n", stage->mem_address);
      }
      else {
        cpu->data_memory[stage->mem_address] = stage->rd_value;
      }
    }
    else if (strcmp(stage->opcode, "LOAD") == 0) {
      // use memory address and write value in data_memory
      if (stage->mem_address > DATA_MEMORY_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing memory location :: %d\n", stage->mem_address);
      }
      else {
        stage->rd_value = cpu->data_memory[stage->mem_address];
      }
    }
    else if (strcmp(stage->opcode, "LDR") == 0) {
      // use memory address and write value in data_memory
      if (stage->mem_address > DATA_MEMORY_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing memory location :: %d\n", stage->mem_address);
      }
      else {
        stage->rd_value = cpu->data_memory[stage->mem_address];
      }
    }
    /* MOVC */
    else if (strcmp(stage->opcode, "MOVC") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now do operation in writeback stage
    }
    else if (strcmp(stage->opcode, "MOV") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now do operation in writeback stage
    }
    else if (strcmp(stage->opcode, "ADD") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "ADDL") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "SUB") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "SUBL") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "MUL") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "DIV") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "AND") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "OR") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "EX-OR") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "BZ") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "BNZ") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "JUMP") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "HALT") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "NOP") == 0) {
      ; // Nothing for now
    }
    else {
      ; // Nothing
    }
    cpu->stage[MEM_ONE].executed = 1;
  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Memory One", stage);
  }

  return 0;
}

/*
 * ########################################## Mem Two Stage ##########################################
 */
int memory_two(APEX_CPU* cpu) {

  cpu->stage[MEM_TWO].executed = 0;
  CPU_Stage* stage = &cpu->stage[MEM_TWO];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      // use memory address and write value in data_memory
      if (stage->mem_address > DATA_MEMORY_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for writing memory location :: %d\n", stage->mem_address);
      }
      else {
        cpu->data_memory[stage->mem_address] = stage->rd_value;
      }
    }
    else if (strcmp(stage->opcode, "STR") == 0) {
      // use memory address and write value in data_memory
      if (stage->mem_address > DATA_MEMORY_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for writing memory location :: %d\n", stage->mem_address);
      }
      else {
        cpu->data_memory[stage->mem_address] = stage->rd_value;
      }
    }
    else if (strcmp(stage->opcode, "LOAD") == 0) {
      // use memory address and write value in data_memory
      if (stage->mem_address > DATA_MEMORY_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing memory location :: %d\n", stage->mem_address);
      }
      else {
        stage->rd_value = cpu->data_memory[stage->mem_address];
      }
    }
    else if (strcmp(stage->opcode, "LDR") == 0) {
      // use memory address and write value in data_memory
      if (stage->mem_address > DATA_MEMORY_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing memory location :: %d\n", stage->mem_address);
      }
      else {
        stage->rd_value = cpu->data_memory[stage->mem_address];
      }
    }
    /* MOVC */
    else if (strcmp(stage->opcode, "MOVC") == 0) {
      ; // stage->rd_value = stage->buffer; // move buffer value to rd_value so it can be forwarded
    }
    else if (strcmp(stage->opcode, "MOV") == 0) {
      ; // stage->rd_value = stage->rs1_value; // move rs1_value value to rd_value so it can be forwarded
    }
    else if (strcmp(stage->opcode, "ADD") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "ADDL") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "SUB") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "SUBL") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "MUL") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "DIV") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "AND") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "OR") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "EX-OR") == 0) {
      // can flags be used to make better decision
      ; // Nothing for now holding rd_value from exe stage
    }
    else if (strcmp(stage->opcode, "BZ") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "BNZ") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "JUMP") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "HALT") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "NOP") == 0) {
      ; // Nothing for now
    }
    else {
      ; // Nothing
    }
    cpu->stage[MEM_TWO].executed = 1;
  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Memory Two", stage);
  }

  return 0;
}

/*
 * ########################################## Writeback Stage ##########################################
 */
int writeback(APEX_CPU* cpu) {

  int ret = 0;
  cpu->stage[WB].executed = 0;
  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "STR") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "LOAD") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "LDR") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    /* MOVC */
    else if (strcmp(stage->opcode, "MOVC") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "MOV") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "ADD") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        if (stage->rd_value == 0) {
          cpu->flags[ZF] = 1; // computation resulted value zero
        }
        else {
          cpu->flags[ZF] = 0; // computation did not resulted value zero
        }
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "ADDL") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        if (stage->rd_value == 0) {
          cpu->flags[ZF] = 1; // computation resulted value zero
        }
        else {
          cpu->flags[ZF] = 0; // computation did not resulted value zero
        }
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "SUB") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        if (stage->rd_value == 0) {
          cpu->flags[ZF] = 1; // computation resulted value zero
        }
        else {
          cpu->flags[ZF] = 0; // computation did not resulted value zero
        }
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "SUBL") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        if (stage->rd_value == 0) {
          cpu->flags[ZF] = 1; // computation resulted value zero
        }
        else {
          cpu->flags[ZF] = 0; // computation did not resulted value zero
        }
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "MUL") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        if (stage->rd_value == 0) {
          cpu->flags[ZF] = 1; // computation resulted value zero
        }
        else {
          cpu->flags[ZF] = 0; // computation did not resulted value zero
        }
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "DIV") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        if (stage->rs1_value % stage->rs2_value != 0) {
          cpu->flags[ZF] = 1; // remainder / operation result is zero
        }
        else {
          cpu->flags[ZF] = 0; // remainder / operation result is not zero
        }
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also un-stall instruction which were dependent on rd reg
        // values are valid un-stall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "AND") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "OR") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "EX-OR") == 0) {
      // use rd address and write value in register
      if (stage->rd > REGISTER_FILE_SIZE) {
        // Segmentation fault
        fprintf(stderr, "Segmentation fault for accessing register location :: %d\n", stage->rd);
      }
      else {
        cpu->regs[stage->rd] = stage->rd_value;
        set_reg_status(cpu, stage->rd, 0); // make desitination regs valid so following instructions won't stall
        // also unstall instruction which were dependent on rd reg
        // values are valid unstall DF and Fetch Stage
        cpu->stage[DRF].stalled = 0;
        cpu->stage[F].stalled = 0;
      }
    }
    else if (strcmp(stage->opcode, "BZ") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "BNZ") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "JUMP") == 0) {
      ; // Nothing for now
    }
    else if (strcmp(stage->opcode, "HALT") == 0) {
      ret = HALT; // return exit code halt to stop simulation
    }
    else if (strcmp(stage->opcode, "NOP") == 0) {
      ; // Nothing for now
    }
    else {
      if (strcmp(stage->opcode, "") == 0) {
        ret = EMPTY; // return exit code empty to stop simulation
      }
    }
    cpu->stage[WB].executed = 1;
    cpu->ins_completed++;
  }
  // But If Fetch has Something and Decode Has NOP Do Not Un Stall Fetch
  // Intrupt Flag is set
  if ((cpu->flags[IF])&&(strcmp(cpu->stage[DRF].opcode, "NOP") == 0)){
    cpu->stage[F].stalled = 1;
  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Writeback", stage);
  }

  return ret;
}

static void push_stages(APEX_CPU* cpu) {

  cpu->stage[WB] = cpu->stage[MEM_TWO];
  cpu->stage[WB].executed = 0;
  cpu->stage[MEM_TWO] = cpu->stage[MEM_ONE];
  cpu->stage[MEM_TWO].executed = 0;
  cpu->stage[MEM_ONE] = cpu->stage[EX_TWO];
  cpu->stage[MEM_ONE].executed = 0;
  cpu->stage[EX_TWO] = cpu->stage[EX_ONE];
  cpu->stage[EX_TWO].executed = 0;
  if (!cpu->stage[DRF].stalled) {
    cpu->stage[EX_ONE] = cpu->stage[DRF];
    cpu->stage[EX_ONE].executed = 0;
  }
  else {
    add_bubble_to_stage(cpu, EX_ONE, 0); // next cycle Bubble will be executed
    cpu->stage[EX_ONE].executed = 0;
  }
  if (!cpu->stage[F].stalled) {
    cpu->stage[DRF] = cpu->stage[F];
    cpu->stage[DRF].executed = 0;
  }
  else if (!cpu->stage[DRF].stalled) {
    add_bubble_to_stage(cpu, DRF, 0); // next cycle Bubble will be executed
    cpu->stage[DRF].executed = 0;
  }
  if (ENABLE_PUSH_STAGE_PRINT) {
    printf("\n--------------------------------\n");
    printf("Clock Cycle #: %d Completed\n", cpu->clock);
    printf("%-15s: Executed: Instruction\n", "Stage");
    printf("--------------------------------\n");
    print_stage_content("Writeback", &cpu->stage[WB]);
    print_stage_content("Memory Two", &cpu->stage[MEM_TWO]);
    print_stage_content("Memory One", &cpu->stage[MEM_ONE]);
    print_stage_content("Execute Two", &cpu->stage[EX_TWO]);
    print_stage_content("Execute One", &cpu->stage[EX_ONE]);
    print_stage_content("Decode/RF", &cpu->stage[DRF]);
    print_stage_content("Fetch", &cpu->stage[F]);
  }
}
/*
 * ########################################## CPU Run ##########################################
 */
int APEX_cpu_run(APEX_CPU* cpu, int num_cycle) {

  int ret = 0;

  while (ret==0) {

    /* Requested number of cycle committed, so pause and exit */
    if ((num_cycle>0)&&(cpu->clock == num_cycle)) {
      printf("Requested %d Cycle Completed\n", num_cycle);
      break;
    }
    // /* All the instructions committed, so exit */
    // if (cpu->ins_completed == cpu->code_memory_size) { // check number of instruction executed to break from while loop
    //   // also check if no brach is taken
    //   printf("All Instruction are Completed\n");
    //   break;
    // }
    else {
      cpu->clock++; // places here so we can see prints aligned with executions

      if (ENABLE_DEBUG_MESSAGES) {
        printf("\n--------------------------------\n");
        printf("Clock Cycle #: %d\n", cpu->clock);
        printf("%-15s: Executed: Instruction\n", "Stage");
        printf("--------------------------------\n");
      }

      // why we are executing from behind ??
      int stage_ret = 0;
      stage_ret = writeback(cpu);
      if ((stage_ret == HALT) || (stage_ret == EMPTY)) {
        if (ENABLE_DEBUG_MESSAGES) {
          print_stage_content("Memory Two", &cpu->stage[MEM_TWO]);
          print_stage_content("Memory One", &cpu->stage[MEM_ONE]);
          print_stage_content("Execute Two", &cpu->stage[EX_TWO]);
          print_stage_content("Execute One", &cpu->stage[EX_ONE]);
          print_stage_content("Decode/RF", &cpu->stage[DRF]);
          print_stage_content("Fetch", &cpu->stage[F]);
        }
        if (stage_ret == HALT) {
          fprintf(stderr, "Simulation Stoped ....\n");
          printf("Instruction HALT Encountered\n");
        }
        else if (stage_ret == EMPTY) {
          fprintf(stderr, "Simulation Stoped ....\n");
          printf("No More Instructions Encountered\n");
        }
        ret = stage_ret;
        break; // break when halt is encountered or empty instruction goes to writeback
      }
      stage_ret = memory_two(cpu);
      stage_ret = memory_one(cpu);
      stage_ret = execute_two(cpu);
      stage_ret = execute_one(cpu);
      stage_ret = decode(cpu);
      stage_ret = fetch(cpu);
      if ((stage_ret!=HALT)&&(stage_ret!=SUCCESS)) {
        ret = stage_ret;
      }
      push_stages(cpu);
    }
  }

  return ret;
}
