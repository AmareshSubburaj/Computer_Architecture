#include "L1cache.h"
#include "L2cache.h"


L1cache* L1cache::L1_cache = NULL;                                  //  initialize the static variable


void  L1cache::create_get_L1cache_instance(int size, int blocksize, int assoc)
{
	if (L1_cache == NULL)
		L1_cache = new L1cache(size, blocksize, assoc);

}

L1cache*  L1cache::get_L1cache_instance()
{

	return L1_cache;

}

L1cache::L1cache(int size, int blocksize, int assoc)
{

	L1cache::blocksize = blocksize;
	L1cache::cachesize = size;
	L1cache::assoc = assoc;
	create_bit_mask();                 //need to initialise the cache here.
	int num_rows = (int)pow(2, indexselect);		    //create a dynamic array of L1_cell having a row of assoc size and

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
	


void L1cache::create_bit_mask()
{

	muxselect = (int)log2(blocksize);
	indexselect = (int)log2((cachesize / (blocksize*assoc)));
	double selectionbits = muxselect + indexselect;
	index_bit_mask = (bitset<32>)0;
	for (int i = muxselect; i < selectionbits; i++)
		index_bit_mask.set(i, 1);
}

void L1cache::process_processor_instruction(string address , int& Memory_latency)
{

	double selectionbits = muxselect + indexselect;
	instruction_set.orig_tagdata = address;
	long int d_tempaddress = hex_to_decimal(address);
	bitset<32> b_tempaddress(d_tempaddress);
	long int d_tagdata = (long int)(d_tempaddress / pow(2, selectionbits));   //shift by selection bits to left to get tag
	instruction_set.tagdata = decimal_to_hex(d_tagdata);

	instruction_set.index = (b_tempaddress&index_bit_mask) >> muxselect;

	instruction_set.d_index = (instruction_set.index.to_ulong()); //can only range from 0 to indexselect or num of sets

	execute_processed_instruction(instruction_set , Memory_latency);

}


void L1cache::execute_processed_instruction(cache_Inst itr_instruct , int & Memory_latency )
{
	bool tag_found = false;

	L2cache *L2 = L2cache::get_L2cache_instance();
	L1cache *L1 = L1cache::get_L1cache_instance();



			tag_found = L1_LRU_find(itr_instruct.orig_tagdata, itr_instruct.d_index, itr_instruct.tagdata);

			if (tag_found)
			{
				Memory_latency= 5;

			}
			else if (tag_found == false)
			{
				if ((L1 != NULL) && (L2 == NULL))
				{

					   L1_LRU_replace(itr_instruct.d_index, itr_instruct.tagdata, itr_instruct.orig_tagdata);

					   Memory_latency=  20;
  
				}

				if ((L1!= NULL) && (L2 != NULL))
				{
						L1_LRU_replace(itr_instruct.d_index, itr_instruct.tagdata, itr_instruct.orig_tagdata);

						Memory_latency=	L2->writerequesttoL2cache(itr_instruct.orig_tagdata);
				}

		}
}


int L1cache::Loc_max_count(int row_index)
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

void L1cache::increment_counter_DiffTag(int row_index, int ref_counter)
{
	for (int j = 0; j < assoc; j++)
	{
		if (ref_counter > cache[row_index][j].counter)
			++cache[row_index][j].counter;

	}
}

long  int  L1cache::hex_to_decimal(string str)
{
	return (long int)stoll(str, 0, 16);
}


string L1cache::decimal_to_hex(long int lli_num)
{
	std::stringstream stream;
	stream << std::hex << lli_num;
	std::string result(stream.str());
	return result;
}

string L1cache::binary_to_hex(bitset<32> bit_num)
{
	stringstream res;
	res << hex << uppercase << bit_num.to_ullong();
	return res.str();

}



void L1cache::L1_formatted_Rowindex_Tagdata(string orig_tagdata, int&row_index, string&tag_data)
{
	double selectionbits = muxselect + indexselect;
	long int d_tempaddress = hex_to_decimal(orig_tagdata);
	bitset<32> b_tempaddress(d_tempaddress);
	long int d_tagdata = (long int)(d_tempaddress / pow(2, selectionbits));   //shift by selection bits to left to get tag
	tag_data = decimal_to_hex(d_tagdata);

	b_tempaddress = ((b_tempaddress&index_bit_mask) >> muxselect);

	row_index = (b_tempaddress.to_ulong()); //can only range from 0 to indexselect or num of sets

}

bool L1cache::L1_LRU_find(string orig_tagdata, int row_index, string Tagdata)
{
	int ref_counter;
	bool tag_found = false;
	
	access++;

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

void L1cache::L1_LRU_replace(int row_index, string Tagdata, string orig_tagdata)
{
	readmiss++;

	string temp_tagdata;

	L2cache *L2 = L2cache::get_L2cache_instance();

	int	Loc_max = L1cache::Loc_max_count(row_index);


	for (int j = 0; j < assoc; j++)
	{
		cache[row_index][j].counter++;
	}

	temp_tagdata = cache[row_index][Loc_max].orig_tagdata; //previous tag

	cache[row_index][Loc_max].orig_tagdata = orig_tagdata;
	cache[row_index][Loc_max].Tag_data = Tagdata;
	cache[row_index][Loc_max].counter = 0;


}

