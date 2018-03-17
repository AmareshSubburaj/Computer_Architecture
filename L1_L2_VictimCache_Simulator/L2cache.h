#include "vector"
#include "bitset"
#include "math.h"
#include "limits"
#include "bitset"
#include "string"
#include "sstream"


using namespace std;

class L2cache
{

	static L2cache *L2_cache;                                   //private static member variable

/***********************datatype declaration************************/


	struct cache_element                                  // data element
	{
		int counter;
		string Tag_data;
		bitset <1>  bit_dirty;
		bitset <1>  bit_validity;
		string orig_tagdata;
		string  sBlock_Data;					  //provision to have data;                   

	};

	typedef struct cache_element element;	  //element





	int R_POLICY;
	int W_POLICY;
	int blocksize;
	int muxselect;
	int indexselect;
	element **cache;

public:

	int cachesize;
	int readmiss;
	int num_reads;
	int num_writes;
	int writemiss;
	int writebacks;
	int assoc;
	double L2_miss_penalty;
	double L2_hit_time;

	/***************************function definitions******************************/

public:

	static	void  create_get_L2cache_instance(int, int, int, int,int);
	static	L2cache  *get_L2cache_instance();


	L2cache(int, int, int, int,int);

	bitset<32> index_bit_mask;
	void create_bit_mask();

	int Loc_max_count(int);
	int min_count(int);
	void increment_counter_DiffTag(int, int);
	long int hex_to_decimal(string);
	string decimal_to_hex(long int);
	string binary_to_hex(bitset<32>);




	void L2_LFU_replace(int, string,string, int);
	void L2_LRU_replace(int, string, string, int);
	bool L2_LRU_find(string, int, string);
	bool L2_LFU_find(string, int, string);

	void L2_formatted_Rowindex_Tagdata(string, int&, string&);
	void DisplayResults();

	void writerequesttoL2cache(string,int,int);
};


