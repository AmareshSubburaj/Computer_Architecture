#pragma once
#include "DispatchQueue.h"


DispatchQueue* DispatchQueue::DispatchQ = NULL;                                  //  initialize the static variable


void  DispatchQueue::create_DispatchQueue_instance()
{
	if (DispatchQ == NULL)
		DispatchQ = new DispatchQueue();

}

DispatchQueue*  DispatchQueue::get_DispatchQueue_instance()
{
	return DispatchQ;
}


void DispatchQueue::set_queuesize( int &size)
{
	 Queuesize = 2*size;
}

void DispatchQueue::set_currentcycle_fetch_num(int &num)
{
	if (((unsigned int)Queuesize - Dispatch.size())>((unsigned int)Queuesize/2)) num = (unsigned int)Queuesize/2;
	else num = ((unsigned int)Queuesize - Dispatch.size());

}   

void DispatchQueue::temp_clear()
{
	temp_q_element.tag = -1;
	temp_q_element.invalid = false;
}

void DispatchQueue::IF_ID_stage()
{
	int pos = -1;
	int num_fetch = 0;
	DynScheduler *Dynschedule = DynScheduler::get_Dynscheduler_instance();
	Fake_ROB *ROB = Fake_ROB::get_FakeBuffer_instance();

	if(Dispatch.size()!=0) //Dispatch ID produced ,IF consumed
	for (std::list <Qu>::iterator it = Dispatch.begin(); it != Dispatch.end(); it++)
	{
		pos = ROB->element_position(it->tag);

		if((ROB->element[pos].IF_begincycle!=-1)&&(ROB->element[pos].IF_endcycle == -1))
		{
			ROB->modifyInstruction(it->tag, IF, 1, current_cycle);
			ROB->modifyInstruction(it->tag, ID, 0, current_cycle);
		
		}
	}

	set_currentcycle_fetch_num(num_fetch); //Fetch   IF produced
	Dynschedule->FetchInstruction(num_fetch);

	while (Dynschedule->N_Instruct_fetch_store.size() != 0)
	{
		ROB->insertElement(Dynschedule->N_Instruct_fetch_store.front());

		temp_clear();
		temp_q_element.tag = Dynschedule->N_Instruct_fetch_store.front().tag;

		  //update ROB
		ROB->modifyInstruction(Dynschedule->N_Instruct_fetch_store.front().tag, IF, 0, current_cycle);
		Dispatch.push_back(temp_q_element);  //update Dispatch
		Dynschedule->N_Instruct_fetch_store.pop_front();
	}

}
