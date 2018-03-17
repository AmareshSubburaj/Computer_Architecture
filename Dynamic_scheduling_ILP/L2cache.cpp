
#include "L2cache.h"
#include "L1cache.h"
#include "iomanip"
#include "iostream"



L2cache* L2cache::L2_cache = NULL;                                  //  initialize the static variable


void  L2cache::create_get_L2cache_instance(int size, int blocksize, int assoc)
{
	if (L2_cache == NULL)
		L2_cache = new L2cache(size, blocksize, assoc);

}

L2cache*  L2cache::get_L2cache_instance()
{

	return L2_cache;

}

L2cache::L2cache(int size, int blocksize, int assoc)
{	
	L2cache::blocksize = blocksize;
	L2cache::cachesize = size;
	L2cache::assoc = assoc;
	create_bit_mask();                             //need to initialise the cache here.
	int num_rows = (int)pow(2, indexselect);	   //create a dynamic array of L1_cell having a row of assoc size and

	//column of 2^index.
	cache = new element *[num_rows];
	for (int i = 0; i < num_rows; ++i)
		cache[i] = new element[assoc];

		if (assoc != 1)
		{
			for (int i = 0; i < num_rows; i++)
			{
				int count = assoc - 1;

				for (int j = 0; j < assoc; j++)
				{
					cache[i][j].counter = count;
					cache[i][j].Tag_data = "";
					cache[i][j].sBlock_Data = "";

					--count;
				}
			}
		}

		else
		{
			//	int count = num_rows - 1;

			for (int i = 0; i < num_rows; i++)
			{
				cache[i][0].counter = 1023;

				//		--count;
			}
		}

}

void L2cache::create_bit_mask()
{

	muxselect = (int)log2(blocksize);
	indexselect = (int)log2((cachesize / (blocksize*assoc)));
	double selectionbits = muxselect + indexselect;
	index_bit_mask = (bitset<32>)0;
	for (int i = muxselect; i < selectionbits; i++)
		index_bit_mask.set(i, 1);
}


int L2cache::Loc_max_count(int row_index)
{
	int max_count = 0;
	int loc;


	for (int j = 0; j < assoc; j++)
	{
		if (cache[row_index][j].counter >= max_count)
		{
			max_count = cache[row_index][j].counter;
			loc = j;
		}

	}
	return loc;

}

void L2cache::increment_counter_DiffTag(int row_index, int ref_counter)
{
	for (int j = 0; j < assoc; j++)
	{
		if (ref_counter > cache[row_index][j].counter)
			++cache[row_index][j].counter;

	}
}


long  int  L2cache::hex_to_decimal(string str)
{
	return (long int)stoll(str, 0, 16);
}


string L2cache::decimal_to_hex(long int lli_num)
{
	std::stringstream stream;
	stream << std::hex << lli_num;
	std::string result(stream.str());
	return result;
}

string L2cache::binary_to_hex(bitset<32> bit_num)
{
	stringstream res;
	res << hex << uppercase << bit_num.to_ullong();
	return res.str();

}

void L2cache::L2_LRU_replace(int row_index, string Tagdata, string orig_tagdata) 
{


	int	Loc_max = L2cache::Loc_max_count(row_index);


	for (int j = 0; j < assoc; j++)
	{
		cache[row_index][j].counter++;
	}
	cache[row_index][Loc_max].orig_tagdata = orig_tagdata;
	cache[row_index][Loc_max].Tag_data = Tagdata;
	cache[row_index][Loc_max].counter = 0;

}


void L2cache::L2_formatted_Rowindex_Tagdata(string orig_tagdata, int&row_index, string&tag_data)
{
	double selectionbits = muxselect + indexselect;
	long int d_tempaddress = hex_to_decimal(orig_tagdata);
	bitset<32> b_tempaddress(d_tempaddress);
	long int d_tagdata = (long int)(d_tempaddress / pow(2, selectionbits));   //shift by selection bits to left to get tag
	tag_data = decimal_to_hex(d_tagdata);

	b_tempaddress = ((b_tempaddress&index_bit_mask) >> muxselect);

	row_index = (b_tempaddress.to_ulong()); //can only range from 0 to indexselect or num of sets

}

bool L2cache::L2_LRU_find(string orig_tagdata, int row_index, string Tagdata)
{
	int ref_counter;
	bool tag_found = false;

	//For read hit
	for (int j = 0; j < assoc; j++)
	{
		if (cache[row_index][j].Tag_data == Tagdata)
		{
			ref_counter = cache[row_index][j].counter;
			cache[row_index][j].orig_tagdata = orig_tagdata;
			increment_counter_DiffTag(row_index, ref_counter);
			cache[row_index][j].counter = 0;
			tag_found = true;
			break;

		}

	}

	return tag_found;
}




int L2cache::writerequesttoL2cache(string orig_tagdata)
{
	access++;
	int row_index;
	int ref_counter;
	bool tag_found = false;
	string Tagdata;

	L2cache *L2 = L2cache::get_L2cache_instance();

	L2_formatted_Rowindex_Tagdata(orig_tagdata, row_index, Tagdata); //now L2 understands.

		//For L2 write  LRU
		for (int j = 0; j < assoc; j++)
		{
			if (cache[row_index][j].Tag_data == Tagdata)                            //write 
			{
				cache[row_index][j].orig_tagdata = orig_tagdata;
				ref_counter = cache[row_index][j].counter;
				increment_counter_DiffTag(row_index, ref_counter);
				cache[row_index][j].counter = 0;
				tag_found = true;
				break;
			}

		}

	if (tag_found == false)                                       // miss
	{
		readmiss++;

		L2_LRU_replace(row_index, Tagdata, orig_tagdata); 																 //writerequesttoL1 must implement here		
	}


	return (tag_found)?10 : 20;

}


void L2cache::DisplayResults()
{
	int num_rows = (int)pow(2, indexselect);

		std::cout << "L2 CACHE CONTENTS" << "\n";
		std::cout << "a.number of accesses :" <<access << "\n";
		std::cout << "b.number of misses :" <<readmiss << "\n";

	for (int i = 0; i < num_rows; i++)
	{


		cout << "set" << i<<":";

		for (int j = 0; j < assoc; j++)
		{
			cout <<"\t"<< cache[i][j].Tag_data;

		}

		cout << "\n";
	}

}
