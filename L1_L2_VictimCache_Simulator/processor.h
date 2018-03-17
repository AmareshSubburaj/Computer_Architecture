#include "iostream"
#include "fstream"
#include "string"
#include "algorithm"
#include "L1cache.h"
#include "L2cache.h"
#include "victim.h"
#include "stdlib.h"


using namespace std;


class processor
{

private:
processor();                                               //constructor
static processor *proX;                                   //private static member variable

public:

static	void  create_processor_instance();
static	processor  *get_processor_instance();
void instruct_cache(const char *);
void execute();
void displayresults();




};




