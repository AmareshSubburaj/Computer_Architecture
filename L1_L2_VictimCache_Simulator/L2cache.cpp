
#include "L2cache.h"
#include "L1cache.h"
#include "iomanip"
#include "iostream"

L2cache* L2cache::L2_cache = NULL;                                  //  initialize the static variable


void  L2cache::create_get_L2cache_instance(int size, int blocksize, int assoc, int R_POLICY,int W_POLICY)
{
	if (L2_cache == NULL)
		L2_cache = new L2cache(size, blocksize, assoc, R_POLICY, W_POLICY);

}

L2cache*  L2cache::get_L2cache_instance()
{

	return L2_cache;

}

L2cache::L2cache(int size, int blocksize, int assoc, int R_POLICY,int W_POLICY)
{	
	L2cache::W_POLICY = W_POLICY;
	L2cache::R_POLICY = R_POLICY;
	L2cache::blocksize = blocksize;
	L2cache::cachesize = size;
	L2cache::assoc = assoc;
	create_bit_mask();                 //need to initialise the cache here.
	int num_rows = (int)pow(2, indexselect);		    //create a dynamic array of L1_cell having a row of assoc size and

	//column of 2^index.
	cache = new element *[num_rows];
	for (int i = 0; i < num_rows; ++i)
		cache[i] = new element[assoc];

	if (R_POLICY == 0)
	{
		if (assoc != 1)
		{
			for (int i = 0; i < num_rows; i++)
			{
				int count = assoc - 1;

				for (int j = 0; j < assoc; j++)
				{

					cache[i][j].bit_dirty.reset();           //resets a cell
					cache[i][j].bit_validity.reset();
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
	else
	{

		for (int i = 0; i < num_rows; i++)
		{

			for (int j = 0; j < assoc; j++)
			{
				cache[i][j].bit_dirty.reset();           //resets a cell
				cache[i][j].bit_validity.reset();
				cache[i][j].counter = 0;
				cache[i][j].Tag_data = "";
				cache[i][j].sBlock_Data = "";

			}

		}

	}

	readmiss = 0;
	writemiss = 0;
	num_reads = 0;
	num_writes = 0;
	writebacks = 0;
	L2_miss_penalty = 20.000 + 0.5*(blocksize / 16.000);
	L2_hit_time = 2.5 + 2.500*((cachesize) / (512.000 * 1024)) + 0.025*((blocksize / 16.000)) + 0.025*assoc;

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




int L2cache::min_count(int row_index)
{


	int min_count = std::numeric_limits<int>::max();


	for (int j = 0; j < assoc; j++)
	{
		if (min_count >= cache[row_index][j].counter)
		{
			min_count = cache[row_index][j].counter;
		}
	}

	return min_count;

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

void L2cache::DisplayResults()
{
	int num_rows = (int)pow(2, indexselect);

	for (int i = 0; i < num_rows; i++)
	{


		cout << "set" << i;

		for (int j = 0; j < assoc; j++)
		{
			cout << "\t" << cache[i][j].Tag_data << setw(5) << (((cache[i][j].bit_dirty == 1) ? "D" : ""));

		}

		cout << "\n";
	}
	double missrate = (double)((((double)(readmiss + writemiss) / (double)(num_reads + num_writes))));


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

void L2cache::L2_LRU_replace(int row_index, string Tagdata, string orig_tagdata, int dirty_bit) 
{


	int	Loc_max = L2cache::Loc_max_count(row_index);


	for (int j = 0; j < assoc; j++)
	{
		cache[row_index][j].counter++;
	}
	cache[row_index][Loc_max].orig_tagdata = orig_tagdata;
	cache[row_index][Loc_max].Tag_data = Tagdata;
	cache[row_index][Loc_max].bit_validity = 1;
	cache[row_index][Loc_max].counter = 0;


	if (cache[row_index][Loc_max].bit_dirty == 1)
		writebacks++;   //to main memory

	cache[row_index][Loc_max].bit_dirty = dirty_bit;


}

void L2cache::L2_LFU_replace(int row_index, string Tagdata, string orig_tagdata, int dirty_bit)
{
	int min_counter = min_count(row_index);

	for (int j = 0; j < assoc; j++)
	{
		if ((cache[row_index][j].bit_validity == 0)||( cache[row_index][j].counter == min_counter))
		{
			cache[row_index][j].Tag_data = Tagdata;
			cache[row_index][j].orig_tagdata = orig_tagdata;
			cache[row_index][j].counter++;  //set counter to min count (set_count+1)
			cache[row_index][j].bit_validity = 1; //set bit validity
			if (cache[row_index][j].bit_dirty == 1)
				writebacks++;  //write back to main memory

			cache[row_index][j].bit_dirty = dirty_bit;
			break;
		}

			
	}

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


bool L2cache::L2_LFU_find(string orig_tagdata, int row_index, string Tagdata)
{
	bool tag_found = false;

	//For read hit
	for (int j = 0; j < assoc; j++)
	{

		if (cache[row_index][j].bit_validity == 1)
		{
			if (cache[row_index][j].Tag_data == Tagdata)
			{	
				cache[row_index][j].orig_tagdata = orig_tagdata;
				//if referred increment the counter.
				cache[row_index][j].counter++;
				tag_found=true;
				break;

			}
		}
	}

	return tag_found;
}


void L2cache::writerequesttoL2cache(string orig_tagdata,int dirty_bit,int readorwrite)
{
	int row_index;
	int ref_counter;
	bool tag_found = false;
	string Tagdata;

	L2cache *L2 = L2cache::get_L2cache_instance();

	if (readorwrite == 0)
	{
		L2->num_reads++;
	}
	else
	{
		L2->num_writes++;
	}


	L2_formatted_Rowindex_Tagdata(orig_tagdata, row_index, Tagdata); //now L2 understands.

	if (R_POLICY == 0)
	{
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
				if(cache[row_index][j].bit_dirty==0)
				cache[row_index][j].bit_dirty = dirty_bit;
				break;
			}

		}
	}
	else  //For L2 write hit LFU
	{

		for (int j = 0; j < assoc; j++)
		{


			if (Tagdata == cache[row_index][j].Tag_data)  //write hit
			{
				cache[row_index][j].orig_tagdata = orig_tagdata;
				cache[row_index][j].counter++;// recent element
				if (cache[row_index][j].bit_dirty == 0)
				cache[row_index][j].bit_dirty = dirty_bit;
				tag_found = true;
				break;
			}


		}

	}


	if (tag_found == false)                                       // miss
	{
		
		if (readorwrite == 0)
		{
			L2->readmiss++;
		}
		else
		{
			L2->writemiss++;
		}



			if (R_POLICY == 0) //LRU
			{
				L2_LRU_replace(row_index, Tagdata, orig_tagdata, dirty_bit); 																 //writerequesttoL1 must implement here
			}
			else  //LFU
			{
				L2_LFU_replace(row_index, Tagdata, orig_tagdata, dirty_bit); 	
			}
		
	}
}


