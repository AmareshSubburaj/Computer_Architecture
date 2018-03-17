#pragma once
#include "string"

using namespace std;

#ifndef MY_ENUMS
#define MY_ENUMS
enum stages { IF, ID, IS, EX, WB };
#endif

class Base {
public:
	struct Instruction
	{
		int tag = -1;
		string Program_counter = "";
		string operation = "";
		int DestReg;
		int SourceReg_1;
		int SourceReg_2;
		string mem_address = "";

	};

	struct Instruction_m
	{
		int tag=-1;
		string Program_counter = "";
		string operation = "";
		int Ex_count_value;
		int DestReg = -1;
		int SourceReg_1_org = -1;
		int SourceReg_2_org = -1;
		int SourceReg_1 = -1;
		int SourceReg_2 = -1;
		bool Source1_ready = false;
		bool Source2_ready = false;
		//---------------------------------------------------
		int IF_begincycle = -1;
		int IF_endcycle = -1;
		int ID_begincycle = -1;
		int ID_endcycle = -1;
		int IS_begincycle = -1;
		int IS_endcycle = -1;
		int EX_begincycle = -1;
		int EX_endcycle = -1;
		int WB_begincycle = -1;
		int WB_endcycle = -1;
		string mem_address = "";

	};


	struct Queue
	{
		int tag=-1;
		bool invalid = true;
	};


	struct ExQueue
	{
		int tag = -1;
		bool invalid = true;
		int counter = 0;
	};


};