/*
 *  main.c
 *
 *  Author :
 *  Sagar Vishwakarma (svishwa2@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"

int main(int argc, char const* argv[])
{
  int num_instruction = 0;
  // argc = count of arguments, executable being 1st argument in argv[0]
  if (argc < 2 || argc > 3) {
    // stderr = Error message on stderr (using fprintf)
    fprintf(stderr, "APEX_Help : Usage %s <input_file> <num_instruction(optional)>\n", argv[0]);
    exit(1);
  }
  else if (argc == 3) {
    num_instruction = atoi(argv[2]);
  }

  APEX_CPU* cpu = APEX_cpu_init(argv[1]);
  if (!cpu) {
    fprintf(stderr, "APEX_Error : Unable to initialize CPU\n");
    exit(1);
  }

  APEX_cpu_run(cpu, num_instruction);
  APEX_cpu_stop(cpu);
  return 0;
}
