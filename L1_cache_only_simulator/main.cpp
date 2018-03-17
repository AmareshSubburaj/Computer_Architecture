/*
From the assumption that processor gives the instruction to the cache I have included the file read logic here.
After its read it sends to the cache*/

#include "processor.h"
//#define CODE 0


#ifdef CODE

#define SIZE 8192
#define ASSOC  2
#define BLOCKSIZE 64
#define R_POLICY 1    //0 or LRU and 1 for LFU
#define W_POLICY 0   // 0 for WBWA and 1 for WTNA

#endif

using namespace std;


int main(int argc, char* argv[])
{

int BLOCKSIZE;		  //32     blocksize
int SIZE;			//8192   size
int ASSOC ;			 //4      assoc
int R_POLICY;		 //0      replacement
int W_POLICY;		 //1      write
char * sfile;         // trace file

#ifdef CODE
	char * sfile = "gcc_trace.txt";
#endif


if (argc == 7)
{
	 BLOCKSIZE =  stoi(argv[1]);    //32     blocksize
	 SIZE	  =  stoi(argv[2]);   //8192   size
	 ASSOC	  =  stoi(argv[3]);   //4      assoc
	 R_POLICY  =  stoi(argv[4]);   //0      replacement
	 W_POLICY  =  stoi(argv[5]);   //1      write
	 sfile  =  argv[6];         // trace file
}



processor::create_processor_instance();
processor *p1 = processor::get_processor_instance();                         //reading has to be done here.
p1->create_cache_instance(SIZE, BLOCKSIZE, ASSOC, R_POLICY);                   // initialize the cache.
p1->instruct_cache(sfile);
p1->execute(R_POLICY, W_POLICY);
p1->displayresults();

return 0;
}
