/*
From the assumption that processor gives the instruction to the cache I have included the file read logic here.
After its read it sends to the cache
*/

#include "processor.h"

using namespace std;

processor::processor()          
{
}


void processor::instruct_cache(const char *trace_file)
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
			L1->process_processor_instruction(str1 , str2);
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

void processor::create_cache_instance(int size, int blocksize, int assoc ,int R_POLICY)
{
	L1 = new cache(size, blocksize, assoc,R_POLICY);
}


void processor::execute(int LFU, int WTNA)
{
	L1->execute_processed_instruction(LFU, WTNA);

}

void processor::displayresults()
{

	L1->DisplayResults();
}