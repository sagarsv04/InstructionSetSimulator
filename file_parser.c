/*
 *  file_parser.c
 *  Contains functions to parse input file and create
 *  code memory, you can edit this file to add new instructions
 *
 *  Author :
 *  Sagar Vishwakarma (svishwa2@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/*
 * This function is related to parsing input file
 *
 * Note : You are not supposed to edit this function
 */
static int get_num_from_string(char* buffer) {

  char str[16];
  int j = 0;
  for (int i = 1; buffer[i] != '\0'; ++i) {
    str[j] = buffer[i];
    j++;
  }
  str[j] = '\0';
  return atoi(str);
}

/*
 * This function is related to parsing input file
 *
 * Note : you can edit this function to add new instructions
 */
static void create_APEX_instruction(APEX_Instruction* ins, char* buffer) {

  char* token = strtok(buffer, ",");
  int token_num = 0;
  char tokens[6][128];
  while (token != NULL) {
    strcpy(tokens[token_num], token);
    token_num++;
    token = strtok(NULL, ",");
  }

  strcpy(ins->opcode, tokens[0]);
  // for MOVC instruction
  if (strcmp(ins->opcode, "MOVC") == 0) {
    ins->rd = get_num_from_string(tokens[1]);
    ins->imm = get_num_from_string(tokens[2]);
  }
  // for STORE instruction
  else if (strcmp(ins->opcode, "STORE") == 0) {
    ins->rs1 = get_num_from_string(tokens[1]); // here rs1 is source and Mem[rs2 + imm] is destination
    ins->rs2 = get_num_from_string(tokens[2]);
    ins->imm = get_num_from_string(tokens[3]);
  }

  // Below are additional code to create code memory for other instructions //

  // for MOVC instruction
  else if (strcmp(ins->opcode, "MOV") == 0) {
    ins->rd = get_num_from_string(tokens[1]);
    ins->rs1 = get_num_from_string(tokens[2]);
  }
  // for STR instruction
  else if (strcmp(ins->opcode, "STR") == 0) {
    ins->rd = get_num_from_string(tokens[1]); // here rd is source and Mem[rs1 + rs2] is destination
    ins->rs1 = get_num_from_string(tokens[2]);
    ins->rs2 = get_num_from_string(tokens[3]);
  }
  // for LDR instruction
  else if (strcmp(ins->opcode, "LDR") == 0) {
    ins->rd = get_num_from_string(tokens[1]); // here rd is destination and Mem[rs1 + rs2] is source
    ins->rs1 = get_num_from_string(tokens[2]);
    ins->rs2 = get_num_from_string(tokens[3]);
  }
  // for LOAD instruction
  else if (strcmp(ins->opcode, "LOAD") == 0) {
    ins->rd = get_num_from_string(tokens[1]); // here rd is destination and Mem[rs1 + imm] is source
    ins->rs1 = get_num_from_string(tokens[2]);
    ins->imm = get_num_from_string(tokens[3]);
  }
  // for ADD instruction
  else if (strcmp(ins->opcode, "ADD") == 0) {
    ins->rd = get_num_from_string(tokens[1]);
    ins->rs1 = get_num_from_string(tokens[2]);
    ins->rs2 = get_num_from_string(tokens[3]);
  }
  // for ADDL instruction
  else if (strcmp(ins->opcode, "ADDL") == 0) {
    ins->rd = get_num_from_string(tokens[1]);
    ins->rs1 = get_num_from_string(tokens[2]);
    ins->imm = get_num_from_string(tokens[3]);
  }
  // for SUB instruction
  else if (strcmp(ins->opcode, "SUB") == 0) {
    ins->rd = get_num_from_string(tokens[1]);
    ins->rs1 = get_num_from_string(tokens[2]);
    ins->rs2 = get_num_from_string(tokens[3]);
  }
  // for SUBL instruction
  else if (strcmp(ins->opcode, "SUBL") == 0) {
    ins->rd = get_num_from_string(tokens[1]);
    ins->rs1 = get_num_from_string(tokens[2]);
    ins->imm = get_num_from_string(tokens[3]);
  }
  // for MUL instruction
  else if (strcmp(ins->opcode, "MUL") == 0) {
    ins->rd = get_num_from_string(tokens[1]);
    ins->rs1 = get_num_from_string(tokens[2]);
    ins->rs2 = get_num_from_string(tokens[3]);
  }
  // for DIV instruction
  else if (strcmp(ins->opcode, "DIV") == 0) {
    ins->rd = get_num_from_string(tokens[1]);
    ins->rs1 = get_num_from_string(tokens[2]);
    ins->rs2 = get_num_from_string(tokens[3]);
  }
  // for BZ instruction Variation 1 only literal
  else if (strcmp(ins->opcode, "BZ") == 0) {
    ins->imm = get_num_from_string(tokens[1]); // while executing our pc starts from 4000 so keep a relative index
  }
  // for BNZ instruction Variation 1 only literal
  else if (strcmp(ins->opcode, "BNZ") == 0) {
    ins->imm = get_num_from_string(tokens[1]); // while executing our pc starts from 4000 so keep a relative index
  }
  else if (strcmp(ins->opcode, "JUMP") == 0) {
    ins->rs1 = get_num_from_string(tokens[1]); // while executing our pc starts from 4000 so keep a relative index
    ins->imm = get_num_from_string(tokens[2]); // here jump location is giving by addidng rs1 + imm
  }
  else if (strcmp(ins->opcode, "HALT") == 0) {
    ; // do nothing
  }
  else {
    fprintf(stderr, "Invalid Instruction Found :: Adding NOP to code_memory\n");
    strcpy(ins->opcode, "NOP");
  }

}

/*
 * This function is related to parsing input file
 *
 * Note : You are not supposed to edit this function
 */
APEX_Instruction* create_code_memory(const char* filename, int* size) {

  if (!filename) {
    return NULL;
  }

  FILE* fp = fopen(filename, "r");
  if (!fp) {
    return NULL;
  }

  char* line = NULL; // the address of the first character position where the input string will be stored.
  size_t len = 0; // size_t is an unsigned integral data type
  ssize_t nread; // ssize_t same as size_t but signed
  int code_memory_size = 0; // for number of lines in a input files

  while ((nread = getline(&line, &len, fp)) != -1) {
    code_memory_size++;
  }
  *size = code_memory_size;
  if (!code_memory_size) {
    fclose(fp);
    return NULL;
  }

  APEX_Instruction* code_memory = malloc(sizeof(*code_memory) * code_memory_size);  // APEX_Instruction struct pointer code_memory
  if (!code_memory) {
    fclose(fp);
    return NULL;
  }

  rewind(fp); // fb is not closed yet, rewind sets the file position to the beginning of the file
  int current_instruction = 0;
  while ((nread = getline(&line, &len, fp)) != -1) {
    create_APEX_instruction(&code_memory[current_instruction], line);
    current_instruction++;
  }

  free(line);
  fclose(fp);
  return code_memory;
}
