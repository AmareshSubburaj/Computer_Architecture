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
	   L1cache *L1 = L1cache::get_L1cache_instance();
	   L1->filename =trace_file;
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


void processor::execute()
{
	L1cache *L1 = L1cache::get_L1cache_instance();
	L1->execute_processed_instruction();

}

void processor::displayresults()
{
	L1cache *L1 = L1cache::get_L1cache_instance();
	L1->DisplayResults();

}