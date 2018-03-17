/*
From the assumption that processor gives the instruction to the cache I have included the file read logic here.
After its read it sends to the cache
*/

#include "processor.h"

using namespace std;

processor::processor()          
{
}


void processor::instruct_cache(int K, int M1, int N, int M2,const char *trace_file, string MODEL)
{       

	   processor *p1 = processor::get_processor_instance();
	   p1->L1->filename =trace_file;
		string sline;
		ifstream f(trace_file);
		if (!f.is_open())
			perror("error while opening file");

		string str1 , str2;
		while (!f.eof())
		{
			f >> str1;
			f >> str2;
			L1->process_processor_instruction(K, M2, M1, N, str2 , str1, MODEL);
     	}
		if (f.bad())
			perror("error while reading file");

}


processor* processor::proX = NULL;                                  //  initialize the static variable


void  processor::create_processor_instance()
{
	if (proX == NULL)
		proX = new processor();

}

processor*  processor::get_processor_instance()
{

	return proX;

}

void processor::create_cache_instance(int K, int M1, int N, int M2, int BTBSIZE, int BLOCKSIZE,  int BTBASSOC , string MODEL)
{

	if (MODEL == "bimodal")
	{
		L1 = new cache(M2, BTBSIZE, BLOCKSIZE, BTBASSOC);
	}
	
	if (MODEL == "gshare")
	{
		L1 = new cache(M1, N, BTBSIZE , BLOCKSIZE, BTBASSOC);
	}

	if (MODEL == "hybrid")
	{
		L1 = new cache( K,  M1,  N,  M2, BTBSIZE , BLOCKSIZE, BTBASSOC);
	}

}



void processor::execute(string MODEL)
{
	L1->execute_processed_instruction(MODEL);

}

void processor::displayresults(string MODEL,int K,int M1,int N,int M2,int SIZE,int ASSOC,string sfile)
{

	L1->DisplayResults( MODEL,  K,  M1,  N,  M2,  SIZE,  ASSOC,  sfile);
}
