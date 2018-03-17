#pragma once
#include "Base.h"
#include "vector"
#include "DynScheduler.h"
#include "DispatchQueue.h"
#include "algorithm"
#include"L1cache.h"
#include "L2cache.h"



class Issuequeue 
{

	
	int N_size;
	static Issuequeue *Issueq;
	Issuequeue();

public:
	int current_cycle;
	int Issueq_size;
	typedef Base::Queue Qu;
	std::vector <Qu> IssueQ;
	std::list <Qu>  tempIssueQ;
	Qu temp_element;
	void temp_clear();
	static void  create_IssueQueue_instance();
	static Issuequeue*  get_IssueQueue_instance();
	void set_IssueQ_size(int &num);
	void set_tempQsize(int & num);

	void Issueq_ascending_order();
	void ID2IS_Dispatch();
	void IS2Temp();

};



class Executequeue
{

	static Executequeue *Executeq;
	int Executeq_size;
	int N_size;
	Executequeue();

public:
	int current_cycle;
	typedef Base::ExQueue Qu;
	typedef Base::Queue Qu1;

private:
	std::vector <Qu> ExecuteQ;
	Qu temp_element;
	int latency;
public:
	static void  create_Executequeue_instance();
	static Executequeue*  get_Executequeue_instance();
	void set_Executeq_size(int num);
	void set_Nsize(int &num);
	void Temp2EX();
	void Execute();
	void updateRMTandIssue(int &num);
	void convert_from_Issue2Ex_Instruction(Qu1 Instruction);
	int setCounter(string &num,string mem_address);
	void temp_clear();

};