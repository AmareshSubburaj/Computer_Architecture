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
void create_cache_instance(int,int,int,int);
void instruct_cache(const char *);
void execute(int, int);
void displayresults();




};




