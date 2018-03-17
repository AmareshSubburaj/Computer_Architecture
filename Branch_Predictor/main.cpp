/*
From the assumption that processor gives the instruction to the cache I have included the file read logic here.
After its read it sends to the cache*/

#include "processor.h"


#ifdef CODE

#define K 0
#define M1 10
#define N 4
#define M2 0
#define SIZE 0
#define ASSOC  0
#endif

#define BLOCKSIZE 4

using namespace std;


int main(int argc, char* argv[])
{
	string MODEL;
	char * sfile="";
	int K = 0;
	int M1 = 0;
	int N = 0;
	int M2 = 0;
	int SIZE = 0;
	int ASSOC = 0;


if (argc == 6) //bimodel
{
MODEL =  argv[1];   
M2	  =  stoi(argv[2]);   
SIZE	  =  stoi(argv[3]);   
ASSOC =  stoi(argv[4]);  
sfile =  argv[5]; // trace file
}
if (argc == 7)  //gshare
{
	MODEL = argv[1];
	M1 = stoi(argv[2]);
	N  = stoi(argv[3]);
	SIZE = stoi(argv[4]);
	ASSOC = stoi(argv[5]);
	sfile = argv[6]; // trace file

}
if (argc == 9)  //gshare
{
	MODEL = argv[1];
	K = stoi(argv[2]);
	M1 = stoi(argv[3]);
	N = stoi(argv[4]);
	M2 = stoi(argv[5]);
	SIZE = stoi(argv[6]);
	ASSOC = stoi(argv[7]);
	sfile = argv[8]; // trace file

}

#ifdef CODE
	char * sfile = "gcc_trace.txt";
	string MODEL = "gshare";
#endif

processor::create_processor_instance();
processor *p1 = processor::get_processor_instance();  

//reading has to be done here.
p1->create_cache_instance(K , M1 , N, M2 , SIZE, BLOCKSIZE, ASSOC, MODEL);                   // initialize the cache.
p1->instruct_cache( K,  M1,  N,  M2, sfile , MODEL);
p1->execute(MODEL);
p1->displayresults(MODEL,K, M1, N, M2, SIZE, ASSOC,sfile);

return 0;
}
