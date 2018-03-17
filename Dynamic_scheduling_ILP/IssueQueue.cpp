#include "IssueQueue.h"


Issuequeue* Issuequeue::Issueq = NULL;

Issuequeue::Issuequeue() {

}

void Issuequeue::create_IssueQueue_instance()
{
	if (Issueq == NULL)
		Issueq = new Issuequeue();
}

Issuequeue * Issuequeue::get_IssueQueue_instance()
{
	return Issueq;
}

void Issuequeue::set_IssueQ_size(int & num)
{
	Issueq_size = num;
}

void Issuequeue::set_tempQsize(int & num)
{
	N_size = num;  //set N which is also equal to temp size.
}



struct Registerfile
{
	bool ready = true;
	int current_tag = -1;
}Reg[127];


void Issuequeue::ID2IS_Dispatch() //Dispatch ID consumed , IS produced
{
	int pos = 0;
	int count = 1;
	bool inserted = false;
	DispatchQueue *DispatchQ = DispatchQueue::get_DispatchQueue_instance();
	Fake_ROB *obj_FakeROB = Fake_ROB::get_FakeBuffer_instance();
	std::list <Qu> temp_list , ref_list;

	if(DispatchQ->Dispatch.size()!=0)
	for (auto itr : DispatchQ->Dispatch)
	{
		pos = obj_FakeROB->element_position(itr.tag);
		
		if ((obj_FakeROB->element[pos].ID_begincycle != -1)&&(obj_FakeROB->element[pos].ID_endcycle == -1))
			temp_list.push_back(itr);
	}

	ref_list = temp_list;

	if(temp_list.size()!=0)
	while(IssueQ.size() <(unsigned int) Issueq_size)
	{
		inserted = true;
		if (temp_list.size() == 0)break;
		    IssueQ.push_back(temp_list.front()); //inserted in to issue
			temp_list.pop_front(); // removed from dispatch

			obj_FakeROB->modifyInstruction(IssueQ.back().tag, ID, 1, current_cycle); //modify the ROB cycles.
			obj_FakeROB->modifyInstruction(IssueQ.back().tag, IS, 0, current_cycle);

			pos = obj_FakeROB->element_position(IssueQ.back().tag);

			/*******************************Register rename*************************************/
			if (obj_FakeROB->element[pos].SourceReg_1 != -1)
			{

				if (Reg[obj_FakeROB->element[pos].SourceReg_1].ready == true) //compare with reg file
				{
					obj_FakeROB->element[pos].Source1_ready = true;
				}

				if (Reg[obj_FakeROB->element[pos].SourceReg_1].ready == false)
				{
					obj_FakeROB->element[pos].Source1_ready = false;
					obj_FakeROB->element[pos].SourceReg_1 = Reg[obj_FakeROB->element[pos].SourceReg_1].current_tag;
				}

			}


			if (obj_FakeROB->element[pos].SourceReg_2 != -1)
			{
				if (Reg[obj_FakeROB->element[pos].SourceReg_2].ready == true)
				{
					obj_FakeROB->element[pos].Source2_ready = true;

				}

				if (Reg[obj_FakeROB->element[pos].SourceReg_2].ready == false)
				{
					obj_FakeROB->element[pos].Source2_ready = false;
					obj_FakeROB->element[pos].SourceReg_2 = Reg[obj_FakeROB->element[pos].SourceReg_2].current_tag;

				}
			}

			if (obj_FakeROB->element[pos].DestReg != -1)
			{
				Reg[obj_FakeROB->element[pos].DestReg].current_tag = obj_FakeROB->element[pos].tag;
				Reg[obj_FakeROB->element[pos].DestReg].ready = false;
			}

		}

	if (inserted)
	{
		for (std::list<Qu>::iterator itr = temp_list.begin(); itr != temp_list.end(); itr++)
		{

			for (std::list<Qu>::iterator itr1 = ref_list.begin(); itr1 != ref_list.end(); )
			{
				if (itr->tag == itr1->tag)
				{
					itr1 = ref_list.erase(itr1);
				}
				else itr1++;
			}
		}

		for (std::list<Qu>::iterator itr = ref_list.begin(); itr != ref_list.end(); itr++)
		{

			for (std::list<Qu>::iterator itr1 = DispatchQ->Dispatch.begin(); itr1 != DispatchQ->Dispatch.end(); )
			{
				if (itr->tag == itr1->tag)
					itr1 = DispatchQ->Dispatch.erase(itr1);
				else
					itr1++;
			}
	}
  }

}

void Issuequeue::Issueq_ascending_order()
{
	for (unsigned int i = 0; i < IssueQ.size(); i++)
	{
		for (unsigned int j = i + 1; j < IssueQ.size(); j++)
		{
			if (IssueQ[i].tag > IssueQ[j].tag)
			{
				temp_element = IssueQ[i];
				IssueQ[i] = IssueQ[j];
				IssueQ[j] = temp_element;
			}
			temp_clear();
		}
	}
}


void Issuequeue::IS2Temp() //IS consumed , temp_Ex produced.
{
	Fake_ROB *obj_FakeROB = Fake_ROB::get_FakeBuffer_instance();
	int pos = -1;
	bool set = true;
	
	Issueq_ascending_order();
	tempIssueQ.clear();

if(IssueQ.size()!=0)
for(vector<Qu>::iterator itr = IssueQ.begin();itr!= IssueQ.end();)
	{
	bool deleted = false;

	if (IssueQ.size() == 0)break;
	
		if (itr->tag != -1)
		{
			if (tempIssueQ.size() == N_size)break;

			pos = obj_FakeROB->element_position(itr->tag);

				//0 0 0
				if ((obj_FakeROB->element[pos].SourceReg_1 == -1) && (obj_FakeROB->element[pos].SourceReg_2 == -1))
				{
					tempIssueQ.push_back(*itr);
					itr = IssueQ.erase(itr);
					deleted = true;
				}

				//0 0 1
				if ((obj_FakeROB->element[pos].SourceReg_1 == -1) && (obj_FakeROB->element[pos].Source2_ready == true))
				{
					tempIssueQ.push_back(*itr);
					itr = IssueQ.erase(itr);
					deleted = true;
				}

				//0 1 0
				if ((obj_FakeROB->element[pos].Source1_ready == true) && (obj_FakeROB->element[pos].SourceReg_2 == -1))
				{
					tempIssueQ.push_back(*itr);
					itr = IssueQ.erase(itr);
					deleted = true;
				}

				//0 1 1
				if ((obj_FakeROB->element[pos].Source1_ready == true) && (obj_FakeROB->element[pos].Source2_ready == true))
				{
					tempIssueQ.push_back(*itr);
					itr = IssueQ.erase(itr);
					deleted = true;
				}
		}
		if (deleted==false)itr++;
	}
}
		



void Issuequeue::temp_clear()
{
	temp_element.tag = -1;
	temp_element.invalid = true;
}
//-------------------------------------------------------Execute-------------------------------------------------
Executequeue* Executequeue::Executeq = NULL;

Executequeue::Executequeue()
{

}
void Executequeue::create_Executequeue_instance()
{
	if (Executeq == NULL)
		Executeq = new Executequeue();

}

Executequeue * Executequeue::get_Executequeue_instance()
{
	return Executeq;
}

void Executequeue::set_Executeq_size(int num)
{
	Executeq_size = num *num*num+100;
}


void Executequeue::set_Nsize(int & num)
{
	N_size = num;
}

void Executequeue::Temp2EX()
{
	Issuequeue *obj_Issue = Issuequeue::get_IssueQueue_instance();
	Fake_ROB *obj_FakeROB = Fake_ROB::get_FakeBuffer_instance();

	while (ExecuteQ.size() <(unsigned int)Executeq_size)
	{
		if (obj_Issue->tempIssueQ.size() == 0) break;

			convert_from_Issue2Ex_Instruction(obj_Issue->tempIssueQ.front());
			ExecuteQ.push_back(temp_element);
			obj_FakeROB->modifyInstruction(ExecuteQ.back().tag, IS, 1, current_cycle); //modify the ROB cycles.
			obj_FakeROB->modifyInstruction(ExecuteQ.back().tag, EX, 0, current_cycle);

			obj_Issue->tempIssueQ.pop_front();
			temp_clear();
		}

	}


void Executequeue::Execute()
{
	Fake_ROB *obj_FakeROB = Fake_ROB::get_FakeBuffer_instance();

	if (ExecuteQ.size() != 0)
	for (vector<Qu>::iterator itr = ExecuteQ.begin(); itr != ExecuteQ.end();)
	{

		   itr->counter--;

			if (itr->counter == 0)
			{

				obj_FakeROB->modifyInstruction(itr->tag, EX, 1, current_cycle);
				obj_FakeROB->modifyInstruction(itr->tag, WB, 0, current_cycle);


				updateRMTandIssue(itr->tag);
				itr = ExecuteQ.erase(itr);
			}
			else itr++;

			
		}

	}


void Executequeue::updateRMTandIssue(int &tag)
{
	int pos = 0;
	Issuequeue *obj_Issue = Issuequeue::get_IssueQueue_instance();
	Fake_ROB *obj_FakeROB = Fake_ROB::get_FakeBuffer_instance();

			for (int i = 0; i < MAX; i++)
			{
			
				if (obj_FakeROB->element[i].SourceReg_1 == tag)
					obj_FakeROB->element[i].Source1_ready = true;
				if (obj_FakeROB->element[i].SourceReg_2 == tag)
					obj_FakeROB->element[i].Source2_ready = true;
			
			}


		for (int i = 0; i < 127; i++)
		{
			if (Reg[i].current_tag != -1)
			{
				if (Reg[i].current_tag == tag)
				{
					Reg[i].ready = true;
					Reg[i].current_tag = -1;
				}
			}
		}


	return;

}

int  Executequeue::setCounter(string &num,string mem_address)
{


	if (num == "0")
		return 1;
	if (num == "1")
		return 2;
	if (num == "2")
	{
		L1cache *L1= L1cache::get_L1cache_instance();		  

		if (L1 != NULL)
		{
			L1->process_processor_instruction(mem_address, latency);
			return latency;
		}

		return 5;

	}
		

	  return 0;

}

void Executequeue::convert_from_Issue2Ex_Instruction(Qu1 Instruction) {
	temp_clear();
	Fake_ROB *obj_FakeROB = Fake_ROB::get_FakeBuffer_instance();

	int pos = obj_FakeROB->element_position(Instruction.tag);


	temp_element.tag = Instruction.tag;
	temp_element.invalid = Instruction.invalid;

	temp_element.counter = setCounter(obj_FakeROB->element[pos].operation , obj_FakeROB->element[pos].mem_address);
}

void Executequeue::temp_clear()
{
	temp_element.tag = -1;
	temp_element.counter = -1;
	temp_element.invalid = true;
}