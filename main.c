/*
 *  main.c
 *
 *  Author :
 *  Sagar Vishwakarma (svishwa2@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"



int main(int argc, char const* argv[])
{
  int num_cycle = 0;
  char* func;
  // argc = count of arguments, executable being 1st argument in argv[0]
  if (argc != 4) {
    // stderr = Error message on stderr (using fprintf)
    fprintf(stderr, "APEX_Help : Usage %s <input_file> <func(eg: simulate Or display)> <num_cycle>\n", argv[0]);
    exit(1);
  }
  else {
    strcpy(func, argv[2]);
    num_cycle = atoi(argv[3]);
  }
  if ((strcmp(func, "display") == 0)||(strcmp(func, "simulate")==0)) {
    APEX_CPU* cpu = APEX_cpu_init(argv[1]);
    if (!cpu) {
      fprintf(stderr, "APEX_Error : Unable to initialize CPU\n");
      exit(1);
    }
    int ret = 0;
    if (strcmp(func, "display") == 0) {
      // show everything
      ret = APEX_cpu_run(cpu, num_cycle);
      if (ret == SUCCESS) {
        printf("(apex) >> Simulation Complete");
      }
      else {
        printf("Simulation Return Code %d\n",ret);
      }
      print_cpu_content(cpu);
      APEX_cpu_stop(cpu);
      printf("Press Any Key to Exit Simulation\n");
      getchar();
    }
    else {
      // show only stages
      ret = APEX_cpu_run(cpu, num_cycle);
      if (ret == SUCCESS) {
        printf("(apex) >> Simulation Complete");
      }
      else {
        printf("Simulation Return Code %d\n",ret);
      }
      APEX_cpu_stop(cpu);
      printf("Press Any Key to Exit Simulation\n");
      getchar();
    }

  }
  else {
    fprintf(stderr, "Invalid parameters passed !!!\n");
    fprintf(stderr, "APEX_Help : Usage %s <input_file> <func(eg: simulate Or display)> <num_cycle>\n", argv[0]);
  }

  return 0;
}
