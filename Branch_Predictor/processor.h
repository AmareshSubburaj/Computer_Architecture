#include "iostream"
#include "fstream"
#include "string"
#include "algorithm"
#include "cache.h"
#include "stdlib.h"


using namespace std;


class processor
{

private:
processor();                                               //constructor
static processor *proX;                                   //private static member variable
cache *L1;

public:

static	void  create_processor_instance();
static	processor  *get_processor_instance();
void create_cache_instance(int K,int M1, int N, int M2,int BTBSIZE, int BLOCKSIZE, int BTBASSOC , string MODEL);
void instruct_cache(int K, int M1, int N, int M2, const char *, string MODEL);
void execute(string);
void displayresults(string MODEL, int K, int M1, int N, int M2, int SIZE, int ASSOC, string sfile);




};




