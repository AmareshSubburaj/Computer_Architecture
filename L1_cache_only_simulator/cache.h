#include "vector"
#include "bitset"
#include "math.h"
#include "limits"
#include "bitset"
#include "string"
#include "sstream"


using namespace std;

class cache 
{

	/***********************datatype declaration************************/

	struct cache_Instruct                                 //store a single instruction
	{
		bitset<1>  readorwrite;
		string tagdata;
		bitset<32> index;  // I need it to calculate d_index
		int d_index;
	};

	typedef struct cache_Instruct cache_Inst;

	struct cache_element                                  // data element
	{
		int counter;
		string Tag_data;
		bitset <1>  bit_dirty;
		bitset <1>  bit_validity;
		string  sBlock_Data;					  //provision to have data;                   

	};

	typedef struct cache_element element;	  //element

	

		int readmiss;
		int num_reads;
		int num_writes;
		int writemiss;
		int writebacks;

		int R_POLICY;
		int W_POLICY;
		
		int blocksize;
		int cachesize;
		int assoc;
		int muxselect;
		int indexselect;
		int* count_set;
		double L1_miss_penalty;
  		double L1_hit_time;
		element **L1cache;
public:
	        string filename;

/***************************function definitions******************************/

public:

	cache(int , int , int ,int );



	cache_Inst instruction_set;
	std::vector <cache_Inst> Instruct_array;       //stores the instruction from the processor.
	bitset<32> index_bit_mask;
	void create_bit_mask();
	void process_processor_instruction(string, string);

	int Loc_max_count(int);
	int min_count(int);
	void increment_counter_DiffTag(int, int);
	void execute_processed_instruction(int,int);
        long int hex_to_decimal(string);
	string decimal_to_hex( long int);
	string binary_to_hex(bitset<32>);




	void LRU_read(string, int,int);
	void LRU_write(string, int,int);
        void LFU_read(string, int,int);
	void LFU_write(string, int, int);
	void DisplayResults();
};


