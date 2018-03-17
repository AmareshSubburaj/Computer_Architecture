/***** student Id: 200207571*********Amaresh Subburaj**********asubbur@ncsu.edu********/
#include "DynScheduler.h"
#include "DispatchQueue.h"
#include "IssueQueue.h"
#define SUCCESS 0

#ifdef COMMANDLINE_ARGUMENTS

#define S_SIZE 32
#define N_SIZE 8
#define BLOCKSIZE 32
#define L1_SIZE 1024
#define L1_ASSOC 4
#define L2_SIZE 2048
#define L2_ASSOC 8

#endif

int main(int argc, char* argv[])
{

	int S_size;
	int N_size;
	int Blocksize;
	int L1_size;
	int L1_assoc;
	int L2_size;
	int L2_assoc;
	char* sfile;

#ifdef COMMANDLINE_ARGUMENTS
	 S_size = S_SIZE;
	 N_size = N_SIZE;
	 Blocksize = BLOCKSIZE;
	 L1_size = L1_SIZE;
	 L1_assoc = L1_ASSOC;
	 L2_size = L2_SIZE;
	 L2_assoc = L2_ASSOC;
	 sfile = "val_perl_trace_mem.txt";
#endif

#ifndef COMMANDLINE_ARGUMENTS
	if (argc == 9)
	{
		S_size = stoi(argv[1]);    //32     blocksize
		N_size = stoi(argv[2]);   //8192   size
		Blocksize = stoi(argv[3]);   //4      assoc
		L1_size = stoi(argv[4]);   //0      replacement
		L1_assoc = stoi(argv[5]);   //1      write
		L2_size = stoi(argv[6]);
		L2_assoc = stoi(argv[7]);
		sfile = argv[8];         // trace file
	}
#endif // !


	DynScheduler::create_Dynscheduler_instance();
	DynScheduler *obj_DynSchedule = DynScheduler::get_Dynscheduler_instance();

	Issuequeue::create_IssueQueue_instance();
	Issuequeue *obj_Issue = Issuequeue::get_IssueQueue_instance();

	DispatchQueue::create_DispatchQueue_instance();
	DispatchQueue *DispatchQ = DispatchQueue::get_DispatchQueue_instance();

	Executequeue::create_Executequeue_instance();
	Executequeue *obj_Execute = Executequeue::get_Executequeue_instance();

	Fake_ROB::create_FakeBuffer_instance();
	Fake_ROB *obj_FakeROB = Fake_ROB::get_FakeBuffer_instance();

	obj_DynSchedule->N_size = N_size;
	obj_DynSchedule->S_size = S_size;
	obj_DynSchedule->set_tracefile((const char*)sfile);
	obj_DynSchedule->TotalInstructionFetch();

	DispatchQ->set_queuesize(N_size);

	obj_Issue->set_IssueQ_size(S_size);
	obj_Issue->set_tempQsize(N_size);

	obj_Execute->set_Nsize(N_size);
	obj_Execute->set_Executeq_size(N_size);

	if (L1_size != 0)
	    L1cache::create_get_L1cache_instance(L1_size, Blocksize, L1_assoc);   // initialize the cache.
	if (L2_size != 0)
		L2cache::create_get_L2cache_instance(L2_size, Blocksize, L2_assoc);

	do {
		obj_DynSchedule->FakeRetire();
		obj_Execute->Execute();
		obj_Issue->IS2Temp();
		obj_Execute->Temp2EX();
		obj_Issue->ID2IS_Dispatch();
		DispatchQ->IF_ID_stage();

	
	  } while (obj_DynSchedule->advance_cycle());


	  obj_DynSchedule->DisplayResults();
	 
	 

	  return SUCCESS;
}
