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

  if (stage->busy) {
    printf(" ---> BUSY ");
  }
  else if (stage->stalled) {
    printf(" ---> STALLED");
  }
  else {
  ; // Nothing
  }
}

/*
 * Get Reg values function
 */
static int get_reg_values(APEX_CPU* cpu, CPU_Stage* stage, int src_reg_pos, int src_reg) {

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

/*
 * Get Reg Status function
 */
static int get_reg_status(APEX_CPU* cpu, int reg_number) {

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

/*
 * Set Reg Status function
 */
static void set_reg_status(APEX_CPU* cpu, int reg_number, int status) {

  if (reg_number > REGISTER_FILE_SIZE) {
    // Segmentation fault
    fprintf(stderr, "Segmentation fault for Register location :: %d\n", reg_number);
  }
  else {
    cpu->regs_invalid[reg_number] = status;
  }
}


static void add_bubble_to_stage(APEX_CPU* cpu, int stage_index) {

  if ((stage_index > F) && (stage_index < NUM_STAGES)) {
    // No adding Bubble in Fetch and WB stage
    if (cpu->stage[stage_index].executed) {
      strcpy(cpu->stage[stage_index].opcode, "NOP"); // add a Bubble
      cpu->code_memory_size = cpu->code_memory_size + 1;
    }
    else {
      ; // Nothing let it execute its current instruction
    }
  }
  else {
    fprintf(stderr, "Cannot Add Bubble at Stage %d\n", stage_index);
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
  print_stage_status(stage);
  printf("\n");
}

static void print_cpu_content(char* name, APEX_CPU* cpu) {

  printf("CPU STATUS IN %-15s :: \n", name);
  // print all Flags
  printf("Falgs::  ZeroFlag, CarryFlag, OverflowFlag, InterruptFlag\n");
  printf("Values:: %d,\t\t%d,\t\t%d,\t\t%d\n", cpu->flags[ZF],cpu->flags[CF],cpu->flags[OF],cpu->flags[IF]);

  // print all regs along with valid bits
  printf("Registers, Values, Invalid\n");
  for (int i=0;i<REGISTER_FILE_SIZE;i++) {
    printf("R%d,\t\t%d,\t\t%d\n", i, cpu->regs[i], cpu->regs_invalid[i]);
  }

  // print 100 memory location
  printf("Mem Location, Values\n");
  for (int i=0;i<100;i++) {
    printf("%d,\t\t%d\n", i, cpu->data_memory[i]);
    ;
  }
  printf("\n");
}
/*
 *  Fetch Stage of APEX Pipeline
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

    /* Copy data from Fetch latch to Decode latch*/
    cpu->stage[F].executed = 1;
    cpu->stage[DRF] = cpu->stage[F]; // this is cool I should empty the fetch stage as well to avoid repetition ?
    cpu->stage[DRF].executed = 0;
    // here may be we can use executed flag to show stage is just latched and not executed in DRF
    // this might help in forwarding as not executed stage might be forwarded to other stages
    // we can use this to stall or execcute a stage
  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Fetch", stage);
  }
  if (ENABLE_REG_MEM_STATUS_PRINT) {
    print_cpu_content("Fetch", cpu);
  }

  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 */
int decode(APEX_CPU* cpu) {

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
      ; // Nothing
    }
    else if (strcmp(stage->opcode, "NOP") == 0) {
      ; // Nothing
    }
    else {
      if (strcmp(stage->opcode, "") != 0) {
        fprintf(stderr, "Decode/RF Invalid Instruction Found :: %s\n", stage->opcode);
      }
    }
  }

  if (cpu->stage[DRF].stalled) {
    // Add NOP to to Ex One
    add_bubble_to_stage(cpu, EX_ONE); // next cycle Bubble will be executed
  }
  else {
    /* Copy data from Decode latch to Execute One latch */
    cpu->stage[DRF].executed = 1;
    cpu->stage[EX_ONE] = cpu->stage[DRF];
    cpu->stage[EX_ONE].executed = 0;
  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Decode/RF", stage);
  }
  if (ENABLE_REG_MEM_STATUS_PRINT) {
    print_cpu_content("Decode/RF", cpu);
  }
  return 0;
}

/*
 *  Execute Stage One of APEX Pipeline
 */
int execute_one(APEX_CPU* cpu) {

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
    else if (strcmp(stage->opcode, "BZ") == 0) {
      // load buffer value to mem_address
      stage->mem_address = stage->buffer;
      // TODO:
      // flush all the previous stages and start fetching instruction from mem_address
    }
    else if (strcmp(stage->opcode, "BNZ") == 0) {
      // load buffer value to mem_address
      stage->mem_address = stage->buffer;
      // TODO:
      // flush all the previous stages and start fetching instruction from mem_address
    }
    else if (strcmp(stage->opcode, "JUMP") == 0) {
      // load buffer value to mem_address
      stage->mem_address = stage->rs1_value + stage->buffer;
      // TODO:
      // flush all the previous stages and start fetching instruction from mem_address
    }
    else if (strcmp(stage->opcode, "HALT") == 0) {
      // TODO:
      // treat Halt as an interrupt and set interrupt flag
      // stop executing any new instructions comming to exe stage
      // complete the instructions in exe, mem. writeback stages
    }
    else if (strcmp(stage->opcode, "NOP") == 0) {
      ; // Do nothing its just a bubble
    }
    else {
      ; // Do nothing
    }

    /* Copy data from Execute One latch to Execute Two latch*/
    cpu->stage[EX_ONE].executed = 1;
    cpu->stage[EX_TWO] = cpu->stage[EX_ONE];
    cpu->stage[EX_TWO].executed = 0;
  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Execute One", stage);
  }
  if (ENABLE_REG_MEM_STATUS_PRINT) {
    print_cpu_content("Execute One", cpu);
  }
  return 0;
}

/*
 *  Execute Stage Two of APEX Pipeline
 */
int execute_two(APEX_CPU* cpu) {

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
      ; // Nothing for now do operation in mem or writeback stage
    }
    else if (strcmp(stage->opcode, "MOV") == 0) {
      ; // Nothing for now do operation in mem or writeback stage
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
      if (stage->rd_value == 0) {
        cpu->flags[ZF] = 1; // computation resulted value zero
      }
      else {
        cpu->flags[ZF] = 0; // computation did not resulted value zero
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
      if (stage->rd_value == 0) {
        cpu->flags[ZF] = 1; // computation resulted value zero
      }
      else {
        cpu->flags[ZF] = 0; // computation did not resulted value zero
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
      if (stage->rd_value == 0) {
        cpu->flags[ZF] = 1; // computation resulted value zero
      }
      else {
        cpu->flags[ZF] = 0; // computation did not resulted value zero
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
      if (stage->rd_value == 0) {
        cpu->flags[ZF] = 1; // computation resulted value zero
      }
      else {
        cpu->flags[ZF] = 0; // computation did not resulted value zero
      }
    }
    else if (strcmp(stage->opcode, "MUL") == 0) {
      // mul registers value and keep in rd_value for mem / writeback stage
      stage->rd_value = stage->rs1_value * stage->rs2_value;
      if (stage->rd_value == 0) {
        cpu->flags[ZF] = 1; // computation resulted value zero
      }
      else {
        cpu->flags[ZF] = 0; // computation did not resulted value zero
      }
    }
    else if (strcmp(stage->opcode, "DIV") == 0) {
      // div registers value and keep in rd_value for mem / writeback stage
      if (stage->rs2_value != 0) {
        stage->rd_value = stage->rs1_value / stage->rs2_value;
        if (stage->rs1_value % stage->rs2_value != 0) {
          cpu->flags[ZF] = 1; // remainder / operation result is zero
        }
        else {
          cpu->flags[ZF] = 0; // remainder / operation result is not zero
        }
      }
      else {
        fprintf(stderr, "Division By Zero Returning Value Zero\n");
        stage->rd_value = 0;
      }
    }
    else if (strcmp(stage->opcode, "BZ") == 0) {
      // load buffer value to mem_address
      stage->mem_address = stage->buffer;
      // TODO:
      // flush all the previous stages and start fetching instruction from mem_address
    }
    else if (strcmp(stage->opcode, "BNZ") == 0) {
      // load buffer value to mem_address
      stage->mem_address = stage->buffer;
      // TODO:
      // flush all the previous stages and start fetching instruction from mem_address
    }
    else if (strcmp(stage->opcode, "JUMP") == 0) {
      // load buffer value to mem_address
      stage->mem_address = stage->rs1_value + stage->buffer;
      // TODO:
      // flush all the previous stages and start fetching instruction from mem_address
    }
    else if (strcmp(stage->opcode, "HALT") == 0) {
      // TODO:
      // treat Halt as an interrupt and set interrupt flag
      // stop executing any new instructions comming to exe stage
      // complete the instructions in exe, mem. writeback stages
    }
    else if (strcmp(stage->opcode, "NOP") == 0) {
      ; // Do nothing its just a bubble
    }
    else {
      ; // Do nothing
    }

    /* Copy data from Execute Two latch to Memory One latch*/
    cpu->stage[EX_TWO].executed = 1;
    cpu->stage[MEM_ONE] = cpu->stage[EX_TWO];
    cpu->stage[MEM_ONE].executed = 0;

  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Execute Two", stage);
  }
  if (ENABLE_REG_MEM_STATUS_PRINT) {
    print_cpu_content("Execute Two", cpu);
  }
  return 0;
}

/*
 *  Memory One Stage of APEX Pipeline
 */
int memory_one(APEX_CPU* cpu) {

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
    /* Copy data from Memory One latch to Memory Two latch*/
    cpu->stage[MEM_ONE].executed = 1;
    cpu->stage[MEM_TWO] = cpu->stage[MEM_ONE];
    cpu->stage[MEM_TWO].executed = 0;
  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Memory One", stage);
  }
  if (ENABLE_REG_MEM_STATUS_PRINT) {
    print_cpu_content("Memory One", cpu);
  }
  return 0;
}

/*
 *  Memory Two Stage of APEX Pipeline
 */
int memory_two(APEX_CPU* cpu) {

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
    /* Copy data from Memory Two latch to Writeback latch*/
    cpu->stage[MEM_TWO].executed = 1;
    cpu->stage[WB] = cpu->stage[MEM_TWO];
    cpu->stage[WB].executed = 0;

  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Memory Two", stage);
  }
  if (ENABLE_REG_MEM_STATUS_PRINT) {
    print_cpu_content("Memory Two", cpu);
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

  int ret = 0;

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
        cpu->regs[stage->rd] = stage->buffer;
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
        cpu->regs[stage->rd] = stage->rs1_value;
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
      fprintf(stderr, "Simulation Paussed ....\n");
      ret = HALT;
    }
    else if (strcmp(stage->opcode, "NOP") == 0) {
      ; // Nothing for now
    }
    else {
      ; // Nothing
    }

    cpu->stage[WB].executed = 1;
    cpu->ins_completed++;

  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Writeback", stage);
  }
  if (ENABLE_REG_MEM_STATUS_PRINT) {
    print_cpu_content("Writeback", cpu);
  }
  return ret;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int APEX_cpu_run(APEX_CPU* cpu, int num_instruction) {

  int ret = 0;

  while (ret==0) {

    /* Requested number of instructions committed, so pause and exit */
    if ((num_instruction>0)&&(cpu->ins_completed == num_instruction)) {
      printf("Requsted %d Instructions Completed\n", num_instruction);
      printf("Press Any Key to Exit Simulation\n");
      getchar();
      break;
    }
    /* All the instructions committed, so exit */
    if (cpu->ins_completed == cpu->code_memory_size) { // check number of instruction executed to break from while loop
      // printf("(apex) >> Simulation Complete");
      break;
    }
    else {
      cpu->clock++; // places here so we can see prints aligned with executions

      if (ENABLE_DEBUG_MESSAGES) {
        printf("--------------------------------\n");
        printf("Clock Cycle #: %d\n", cpu->clock);
        printf("--------------------------------\n");
      }

      // why we are executing from behind ??
      int stage_ret = 0;
      stage_ret = writeback(cpu);
      stage_ret = memory_two(cpu);
      stage_ret = memory_one(cpu);
      stage_ret = execute_two(cpu);
      stage_ret = execute_one(cpu);
      stage_ret = decode(cpu);
      stage_ret = fetch(cpu);

      if (stage_ret == HALT) {
        printf("Instruction HALT Encountered\n");
        printf("Press Any Key to Continue Simulation\n");
        getchar();
      }
      else {
        ret = stage_ret;
      }
    }
  }

  if (ret == SUCCESS) {
    printf("(apex) >> Simulation Complete");
  }
  else {
    printf("Simulation Return Code %d\n",ret);
  }
  return 0;
}
