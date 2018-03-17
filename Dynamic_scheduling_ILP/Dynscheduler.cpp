
#include "L1cache.h"
#include "L2cache.h"
#include "DynScheduler.h"
#include "DispatchQueue.h"




DynScheduler* DynScheduler::DynschedulerX = NULL;                                  //  initialize the static variable

DynScheduler::DynScheduler()
{

}
bool DynScheduler::advance_cycle()
{
	++current_cycle;
	Fake_ROB::create_FakeBuffer_instance();
	Fake_ROB *obj_FakeROB = Fake_ROB::get_FakeBuffer_instance();

	Issuequeue::create_IssueQueue_instance();
	Issuequeue *obj_Issue = Issuequeue::get_IssueQueue_instance();

	DispatchQueue::create_DispatchQueue_instance();
	DispatchQueue *DispatchQ = DispatchQueue::get_DispatchQueue_instance();

	Executequeue::create_Executequeue_instance();
	Executequeue *obj_Execute = Executequeue::get_Executequeue_instance();
	
	
	obj_Issue->current_cycle = current_cycle;
	DispatchQ->current_cycle = current_cycle;
	obj_Execute->current_cycle = current_cycle;

	if (stop)
	{
		total_cycles= current_cycle;
		return false;
	}
	else
	{
		
		return true;
	}

	
}
void  DynScheduler::create_Dynscheduler_instance()
{
	if (DynschedulerX == NULL)
		DynschedulerX = new DynScheduler();

}

DynScheduler*  DynScheduler::get_Dynscheduler_instance()
{
	return DynschedulerX;
}


void DynScheduler::set_tracefile(const char * tracefile)
{

	DynScheduler *Dynschedule = DynScheduler::get_Dynscheduler_instance();
	Dynschedule->filename = tracefile;

}

void DynScheduler::TotalInstructionFetch()
{  
	int tag_count = 0;
	DynScheduler *Dynschedule = DynScheduler::get_Dynscheduler_instance();
	Instruction Instructions;
	string sline, PC, Operator, DestReg, SReg1, SReg2, mem_address;
	ifstream  p_file(Dynschedule->filename);
	TotalInstructions.clear();

	if (!p_file.is_open())
		perror("error while opening file");

	while (!p_file.eof())
	{
		p_file >> PC;
		p_file >> Operator;
		p_file >> DestReg;
		p_file >> SReg1;
		p_file >> SReg2;
		p_file >> mem_address;
		TotalInstructions_count++;
		Instructions.Program_counter = PC;
		Instructions.operation = Operator;
		Instructions.DestReg = std::stoi(DestReg);
		Instructions.SourceReg_1 = std::stoi(SReg1);
		Instructions.SourceReg_2 = std::stoi(SReg2);
		Instructions.mem_address = mem_address;
		Instructions.tag = tag_count++;  
		TotalInstructions.push_back(Instructions);

	}

	TotalInstructions.pop_back(); //need to remove the last instruction repeated twice.
	TotalInstructions_count--;

	if (p_file.bad())
		perror("error while reading file");

}
int DynScheduler::FetchInstruction(int NumInstruct)
{	
	
	{
		for (int i = 0; i < NumInstruct; i++)
		{			
			if (TotalInstructions.size() == 0)break;
			N_Instruct_fetch_store.push_back(TotalInstructions.front());
			TotalInstructions.pop_front();
		}
	}

	if (TotalInstructions.size() == 0)return -1;

	return 0;
}


void DynScheduler::FakeRetire()
{
	bool set = false;
	Fake_ROB *obj_FakeROB = Fake_ROB::get_FakeBuffer_instance();
	Instruction_m * item = new Instruction_m();

	if(obj_FakeROB->isEmpty()==false)
	{

		for (int i = 0; i < MAX; i++)
		{
			if ((obj_FakeROB->element[i].WB_begincycle != -1)&&(obj_FakeROB->element[i].WB_endcycle == -1))
			{
	

				obj_FakeROB->modifyInstruction(obj_FakeROB->element[i].tag, WB, 1, current_cycle);


			}

		}

    	while (obj_FakeROB->element[obj_FakeROB->head].WB_endcycle != -1)
		{

			obj_FakeROB->deleteElement(item);

			if(item!=NULL)
			cout << item->tag << " fu{" << item->operation << "} src{"
				<< item->SourceReg_1_org << "," << item->SourceReg_2_org
				<< "} dst{" << item->DestReg <<
				"} IF{" << item->IF_begincycle
				<< "," << (item->IF_endcycle - item->IF_begincycle)
				<< "} ID{" << item->ID_begincycle
				<< "," << (item->ID_endcycle - item->ID_begincycle)
				<< "} IS{" << item->IS_begincycle
				<< "," << (item->IS_endcycle - item->IS_begincycle)
				<< "} EX{" << item->EX_begincycle
				<< "," << (item->EX_endcycle - item->EX_begincycle)
				<< "} WB{" << item->WB_begincycle
				<< "," << (item->WB_endcycle - item->WB_begincycle) << "}\n";
			
			if (obj_FakeROB->isEmpty())
			{
				stop = true;
				break;
			}
		}
	}	
}



void DynScheduler::DisplayResults()
{
	L1cache *L1 = L1cache::get_L1cache_instance();
	L2cache *L2 = L2cache::get_L2cache_instance();
	DynScheduler *obj_DynSchedule = DynScheduler::get_Dynscheduler_instance();
	

	if (L1 != NULL)
	{
		int num_rows = (int)pow(2, L1->indexselect);
		std::cout << "L1 CACHE CONTENTS" << "\n";
		std::cout << "a.number of accesses :" << L1->access << "\n";
		std::cout << "b.number of misses :" << L1->readmiss << "\n";
		for (int i = 0; i < num_rows; i++)
		{
			std::cout << "set" << i << ":";

			for (int j = 0; j < L1->assoc; j++)
			{
				std::cout << "\t" << L1->cache[i][j].Tag_data;
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}

	if (L2 != NULL)
	{
		L2->DisplayResults();
	}

	std::cout << "CONFIGURATION" << "\n";
	std::cout << " superscalar bandwidth (N) = " << setw(2) << N_size << "\n";
	std::cout << " dispatch queue size (2*N) = " << setw(2) << 2*N_size << "\n";
	std::cout << " schedule queue size (S)   = " <<setw(2) << S_size << "\n";
	std::cout << "RESULTS" << "\n";
	std::cout << " number of instructions = "<< setw(2) <<TotalInstructions_count<< "\n";
	std::cout << " number of cycles       = "<< setw(2) << (total_cycles-1) << "\n";
	std::cout << " IPC                    = "<< std::fixed << std::setprecision(2)<<(float)TotalInstructions_count/(float)(total_cycles-1)<< "\n";

}

//****************************************ROB**************************************/

Fake_ROB* Fake_ROB::Fake_Buffer = NULL;

void  Fake_ROB::create_FakeBuffer_instance()
{
	if (Fake_Buffer == NULL)
		Fake_Buffer = new Fake_ROB();

}

Fake_ROB*  Fake_ROB::get_FakeBuffer_instance()
{
	return Fake_Buffer;
}


Fake_ROB::Fake_ROB()
{
	head = 0;
	tail = 0;
	count = 0;
}

bool Fake_ROB::isFull()
{
	return (count == MAX);

}

bool Fake_ROB::isEmpty()
{
	return (count == 0);
}

int Fake_ROB::insertElement(Instruction item)
{
	if (isFull())
	{
		return -1;
	}
	temp_clear();
	convert_instruction(item);
	element[tail] = temp_Instruct;
	tail = (tail + 1) % MAX;

	count++;

	return 0;

}


int Fake_ROB::deleteElement(Instruction_m * item)
{
	if (isEmpty())
	{
		return -1;
	}
	*item = element[head];
	temp_clear();
	element[head]= temp_Instruct;
	head = (head + 1) % MAX;
	count--;

	return 0;
}

bool Fake_ROB::modifyInstruction(int  &tag, stages MStage, int beginrend, int value)
{
	bool set = false;
	int position = element_position(tag);
	DynScheduler *obj_DynSchedule = DynScheduler::get_Dynscheduler_instance();

	if (position != -1)
		switch (MStage)
		{
		case IF: {

			if (beginrend == 0) element[position].IF_begincycle = value;
			if (beginrend == 1) element[position].IF_endcycle = value;
			return true;
		}

		case ID: {

			if (beginrend == 0) element[position].ID_begincycle = value;
			if (beginrend == 1) element[position].ID_endcycle = value;
			return true;

		}

		case IS: {

			if (beginrend == 0) element[position].IS_begincycle = value;
			if (beginrend == 1) element[position].IS_endcycle = value;
			return true;

		}

		case EX: {


			if (beginrend == 0) element[position].EX_begincycle = value;
			if (beginrend == 1) element[position].EX_endcycle = value;
			return true;


		}

		case WB: {

			if (beginrend == 0) element[position].WB_begincycle = value;
			if (beginrend == 1) element[position].WB_endcycle = value;
			return true;
		}
		}

	return false;
}

int Fake_ROB::element_position(int & tag)
{
	for (int i = 0; i <= MAX; i++)
	{
		if (element[i].tag == tag)
			return i;
	}

	return -1;
}

void Fake_ROB::convert_instruction(Instruction item)
{
	temp_Instruct.tag = item.tag;
	temp_Instruct.Program_counter = item.Program_counter;
	temp_Instruct.operation = item.operation;
	temp_Instruct.DestReg = item.DestReg;
	temp_Instruct.SourceReg_1 = item.SourceReg_1;
	temp_Instruct.SourceReg_1_org = item.SourceReg_1;
	temp_Instruct.SourceReg_2 = item.SourceReg_2;
	temp_Instruct.SourceReg_2_org = item.SourceReg_2;
	temp_Instruct.mem_address = item.mem_address;
}

void Fake_ROB::temp_clear()
{
	temp_Instruct.tag = -1;
	temp_Instruct.Program_counter = "";
	temp_Instruct.operation = "";
	temp_Instruct.DestReg = -1;
	temp_Instruct.SourceReg_1 = -1;
	temp_Instruct.SourceReg_1_org = -1;
	temp_Instruct.SourceReg_2 = -1;
	temp_Instruct.SourceReg_2_org = -1;
	temp_Instruct.IF_begincycle = -1;
	temp_Instruct.IF_endcycle = -1;
	temp_Instruct.ID_begincycle = -1;
	temp_Instruct.ID_endcycle = -1;
	temp_Instruct.IS_begincycle = -1;
	temp_Instruct.IS_endcycle = -1;
	temp_Instruct.EX_begincycle = -1;
	temp_Instruct.EX_endcycle = -1;
	temp_Instruct.WB_begincycle = -1;
	temp_Instruct.WB_endcycle = -1;
	temp_Instruct.mem_address = "";
}


