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
		bool  TakenBranch;
		string PC;
		string tagdata;
		bitset<32> index;
		bitset<32> bimodel_index;  // I need it to calculate d_index
		bitset<32> gshare_index;
		bitset<32> hybrid_index;
		int d_index;
		int bimodel_d_index;
		int gshare_d_index;
		int hybrid_d_index;
	};

	typedef struct cache_Instruct cache_Inst;

	struct cache_element                                  // data element
	{
		int counter;
		string Tag_data;                  

	};

	typedef struct cache_element element;	  //element


		int BTB_blocksize;
		int BTB_cachesize;
		int BTB_assoc;
		int BTB_muxselect;
		int BTB_indexselect;

		int bimodel_indexselect;
		int gshare_indexselect;
		int hybrid_indexselect;

		element **L1cache;
		int * Bimodel_BranPredictTable;
		int * gshare_BranPredictTable;
		int * hybrid_BranPredictTable;
		bool *GlobalHistoryRegisters;
		int GlobalShiftRegSize;
		int gshare_indexsize;
		//int i = 0;
		unsigned long  int predictions=0;
		unsigned long  int mispredictions=0;
		unsigned long  int BP__predictions=0;
		unsigned long  int NON_BP__predictions=0;
		unsigned long  int BP__mispredictions=0;
		unsigned long  int BM__BTB_taken;		

public:
	        string filename;

/***************************function definitions******************************/

public:

	cache(int M2, int size, int blocksize, int assoc);
	cache(int M1, int N , int size, int blocksize, int assoc);
	cache(int K, int M1, int N, int M2 , int size, int blocksize, int assoc);



	cache_Inst instruction_set;
	std::vector <cache_Inst> Instruct_array;       //stores the instruction from the processor.
	bitset<32> index_bit_mask;
	void create_bit_mask();
	void process_processor_instruction(int K,int M2, int M1, int N, string NT, string address, string MODEL);

	int Loc_max_count(int);
	void increment_counter_DiffTag(int, int);
	void execute_processed_instruction(string);
    long int hex_to_decimal(string);
	string decimal_to_hex( long int);
	string binary_to_hex(bitset<32>);
	void UpdateCounter(int &,bool);
	bool TakenNotTaken(int);
	void BoolArrayShiftRight(bool GlobalShiftReg[], bool Taken);

	int BoolArrayintoint(bool GlobalShiftReg[]);



	bool LRU_read(string, int);
	void DisplayResults(string MODEL, int K, int M1, int N, int M2, int SIZE, int ASSOC, string sfile);
};


