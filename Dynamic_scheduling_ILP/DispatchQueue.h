#pragma once
#include"Base.h"
#include "DynScheduler.h"


#ifndef DISPATCH_QUEUE
#define DISPATCH_QUEUE

class DispatchQueue 
{
	int Queuesize=0;
	static DispatchQueue *DispatchQ;

public:


	typedef Base::Queue Qu;

private:

	Qu temp_q_element;

public:
	int current_cycle=0;
	std::list <Qu> Dispatch;
	static void  create_DispatchQueue_instance();
	static DispatchQueue*  get_DispatchQueue_instance();
	void set_queuesize(int &size);
	void set_currentcycle_fetch_num(int &num); //czlled to set fetch num in dynschedule.
	void IF_ID_stage();
	void temp_clear();

};
#endif