# InstructionSetSimulator

An instruction set simulator coded in C language, which mimics the behavior of a microprocessor by "reading" instructions and maintaining internal variables which represent the processor's registers.

Project 1 - APEX Pipeline Simulator
============

Part A - No Data Forwarding
============

A simple implementation of 7 Stage APEX Pipeline

Author :
============
Sagar Vishwakarma (svishwa2@binghamton.edu)

State University of New York, Binghamton

Notes:
============

1)	This code is a simple implementation of 7 Stage APEX Pipeline.
		Fetch -> Decode -> Execute One -> Execute Two -> Memory One -> Memory Two -> Writeback

		You can read, modify and build upon given codebase to add other features as
		required in project description. You are also free to write your own
		implementation from scratch.

2)	All the stages have latency of one cycle. There is a single functional unit in
		EX stage which perform all the arithmetic and logic operations.

3)	Logic to check data dependencies has been included.

File-Info
============

1)	Makefile				- You can edit as needed
2)	file_parser.c 	- Contains Functions to parse input file.
3)	cpu.c						- Contains Implementation of APEX cpu.
4)	cpu.h						- Contains various data structures declarations needed by 'cpu.c'.


How to compile and run
============

1)	go to terminal, cd into project directory and type 'make' to compile project
2)	Run using ./apex_sim <input_file> <func> <num_cycle>
		eg: ./apex_sim input.asm simulate 50


Test Run
============		

File : input_test_0.asm ---> 39 cycle (!Forwarding) till HALT instruction is processed in Writeback.

File : input_test_1.asm ---> 41 cycle (!Forwarding) till HALT instruction is processed in Writeback.

File : input_test_0.asm ---> 28 cycle (Forwarding) till HALT instruction is processed in Writeback.

File : input_test_1.asm ---> 27 cycle (Forwarding) till HALT instruction is processed in Writeback.
