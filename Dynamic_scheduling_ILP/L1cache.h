
#include "vector"
#include "bitset"
#include "math.h"
#include "limits"
#include "bitset"
#include "string"
#include "sstream"



using namespace std;

#ifndef L1CACHE
#define L1CACHE
class L1cache 
{

	static L1cache *L1_cache;                                   //private static member variable

	/***********************datatype declaration************************/

	struct cache_Instruct                                 //store a single instruction
	{
		bitset<1>  readorwrite;
		string orig_tagdata;
		string tagdata;
		bitset<32> index;  // I need it to calculate d_index
		int d_index;
	};

	typedef struct cache_Instruct cache_Inst;

	struct cache_element                                  // data element
	{
		int counter;
		string Tag_data;
		string orig_tagdata;
		string  sBlock_Data;					  //provision to have data;                   

	};

	typedef struct cache_element element;	  //element

		
		int blocksize;
		int cachesize;
		int muxselect;
		

public:
	        int indexselect;
	        string filename;
			int assoc;
			element **cache;
			int access=0;
			int readmiss=0;

/***************************function definitions******************************/

public:

	static	void  create_get_L1cache_instance(int,int,int);
	static	L1cache  *get_L1cache_instance();


	L1cache(int , int , int);



	cache_Inst instruction_set;
	std::vector <cache_Inst> Instruct_array;       //stores the instruction from the processor.
	bitset<32> index_bit_mask;
	void create_bit_mask();
	void process_processor_instruction(string ,int &);

	int Loc_max_count(int);
	void increment_counter_DiffTag(int, int);
	void execute_processed_instruction(cache_Inst itr_instruct, int & Memory_latency);
    long int hex_to_decimal(string);
	string decimal_to_hex( long int);
	string binary_to_hex(bitset<32>);
	void L1_LRU_replace(int, string, string);
	bool L1_LRU_find(string, int, string);

	void L1_formatted_Rowindex_Tagdata(string, int&, string&);

};

#endif
