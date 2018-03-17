#pragma once
#include "vector"
#include "bitset"
#include "math.h"
#include "limits"
#include "bitset"
#include "string"
#include "sstream"


using namespace std;

class VMcache
{

	static VMcache *VM_cache;                                   //private static member variable

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
	int assoc;
	element *cache;

public:
	int blocksize;
	int cachesize;
	int num_swaps;
	int writebacks;
	double VM_miss_penalty;
	double VM_hit_time;

	/***************************function definitions******************************/

public:

	static	void  create_get_VMcache_instance(int, int);
	static	VMcache  *get_VMcache_instance();


	VMcache(int, int);

	int Loc_max_count();
	void increment_counter_DiffTag(int);

	long int hex_to_decimal(string);
	string decimal_to_hex(long int);
	string binary_to_hex(bitset<32>);

	void VM_LRU_replace(string, bitset<1>);
	bool VM_LRU_find(string orig_tagdata,int readorwrite);

	void VM_formatted_Tagdata(string , string& );

	void DisplayResults();
};



