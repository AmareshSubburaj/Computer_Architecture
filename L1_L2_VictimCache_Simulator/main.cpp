/*
From the assumption that processor gives the instruction to the cache I have included the file read logic here.
After its read it sends to the cache*/

#include "processor.h"

#define W_POLICY 0

using namespace std;


int main(int argc, char* argv[])
{

	int BLOCKSIZE;		 
	int L1SIZE;			
	int L1ASSOC;			
	int VMSIZE;             
	int L2SIZE;		
	int L2ASSOC;		 
	int R_POLICY;
        int ReadPolicy;
	char * sfile;         


	BLOCKSIZE= stoi(argv[1]);   
	L1SIZE =  stoi(argv[2]);  
	L1ASSOC =  stoi(argv[3]);    
	VMSIZE =  stoi(argv[4]);   
	L2SIZE =  stoi(argv[5]);   
	L2ASSOC = stoi(argv[6]);        
	R_POLICY = stoi(argv[7]);
	sfile =  argv[8];


if(R_POLICY==3)
{
 ReadPolicy=1; //LFU
}
else if(R_POLICY==2)
{
 ReadPolicy=0; //LRU
}

processor::create_processor_instance();
processor *p1 = processor::get_processor_instance();                         //reading has to be done here.
L1cache::create_get_L1cache_instance(L1SIZE, BLOCKSIZE, L1ASSOC, ReadPolicy, W_POLICY);   // initialize the cache.
if(L2SIZE!=0)
L2cache::create_get_L2cache_instance(L2SIZE, BLOCKSIZE, L2ASSOC, ReadPolicy, W_POLICY);
if (VMSIZE != 0)
VMcache::create_get_VMcache_instance(VMSIZE, BLOCKSIZE);

p1->instruct_cache(sfile);               
p1->execute();
p1->displayresults();

return 0;
}
