#include "Base.h"
#include "iostream"
#include "fstream"
#include "string"
#include "list"
#include "algorithm"
#include "iomanip"
#include "iostream"
#include "IssueQueue.h"

#define MAX 1024
using namespace std;

#ifndef DYNSCHEDULER
#define DYNSCHEDULER
class DynScheduler 
{

public:

	typedef Base::Instruction Instruction;
	typedef Base::Instruction_m Instruction_m;

private:
	int total_cycles;
	DynScheduler();
	static DynScheduler *DynschedulerX;
	string filename;


public:	
	int N_size, S_size;
	int TotalInstructions_count = 0;
	int current_cycle = 0;
	bool stop = false;
	bool advance_cycle();
	std::list <Instruction> N_Instruct_fetch_store;
	std::list <Instruction> TotalInstructions;
	void TotalInstructionFetch();
	static	void 	create_Dynscheduler_instance();
	static DynScheduler* get_Dynscheduler_instance();
	void set_tracefile(const char *);
	int FetchInstruction(int NumInstruct);
	void FakeRetire();
	void DisplayResults();

};

#endif


#ifndef FAKE
#define FAKE
class Fake_ROB
{

	typedef Base::Instruction Instruction;
	typedef Base::Instruction_m Instruction_m;

private:

	Fake_ROB();
	static Fake_ROB *Fake_Buffer;

	int count;
	Instruction_m temp_Instruct;

public:
	int head = 0;
	int tail = -1;

	Instruction_m  element[MAX];
	bool  isFull();
	bool  isEmpty();
	static void  create_FakeBuffer_instance();
	static Fake_ROB*  get_FakeBuffer_instance();
	int  insertElement(Instruction item);
	int  deleteElement(Instruction_m * item);
	bool modifyInstruction(int &tag, stages MStage, int beginrend, int value);
	int  element_position(int &tag);
	void convert_instruction(Instruction item);
	void temp_clear();
};
#endif