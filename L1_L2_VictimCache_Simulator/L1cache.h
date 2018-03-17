#include "vector"
#include "bitset"
#include "math.h"
#include "limits"
#include "bitset"
#include "string"
#include "sstream"



using namespace std;

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
		bitset <1>  bit_dirty;
		bitset <1>  bit_validity;
		string  sBlock_Data;					  //provision to have data;                   

	};

	typedef struct cache_element element;	  //element

		
		int blocksize;
		int cachesize;

		int muxselect;
		int indexselect;
		int* count_set;
		double L1_miss_penalty;
  		double L1_hit_time;

public:
	        string filename;
			int assoc;
			element **cache;
			int R_POLICY;
			int W_POLICY;


			int readmiss;
			int num_reads;
			int num_writes;
			int writemiss;

/***************************function definitions******************************/

public:

	static	void  create_get_L1cache_instance(int,int,int,int,int);
	static	L1cache  *get_L1cache_instance();


	L1cache(int , int , int ,int ,int);



	cache_Inst instruction_set;
	std::vector <cache_Inst> Instruct_array;       //stores the instruction from the processor.
	bitset<32> index_bit_mask;
	void create_bit_mask();
	void process_processor_instruction(string, string);

	int Loc_max_count(int);
	int min_count(int);
	void increment_counter_DiffTag(int, int);
	void execute_processed_instruction();
    long int hex_to_decimal(string);
	string decimal_to_hex( long int);
	string binary_to_hex(bitset<32>);



	void L1_LFU_replace(int, string, string, int,int);
	void L1_LRU_replace(int, string, string, int,int);
	bool L1_LRU_find(string, int, string);
	bool L1_LFU_find(string, int, string);

	void DisplayResults();


	void L1_formatted_Rowindex_Tagdata(string, int&, string&);

};


