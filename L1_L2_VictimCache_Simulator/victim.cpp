#define L2_size 1


#include "L2cache.h"
#include "L1cache.h"
#include "victim.h"
#include "iomanip"
#include "iostream"

VMcache* VMcache::VM_cache = NULL;                                  //  initialize the static variable


void  VMcache::create_get_VMcache_instance(int size, int blocksize)
{
	if (VM_cache == NULL)
		VM_cache = new VMcache(size, blocksize);

}

VMcache*  VMcache::get_VMcache_instance()
{

	return VM_cache;

}

VMcache::VMcache(int size, int blocksize)
{
	VMcache::R_POLICY = 0;
	VMcache::blocksize = blocksize;
	VMcache::cachesize = size;
	VMcache::assoc = (size/blocksize);                 //need to initialise the cache here.


	//column of 2^index.
	cache = new element [assoc];	

	int count = assoc - 1;

			for (int j = 0; j < assoc; j++)
			{
				

				cache[j].bit_dirty.reset();           //resets a cell
				cache[j].bit_validity.reset();
				cache[j].counter = count;
				cache[j].Tag_data = "";
				cache[j].sBlock_Data = "";

				--count;
			}

	num_swaps = 0;
	writebacks = 0;


}


int VMcache::Loc_max_count()
{
	int max_count = 0;
	int loc;


	for (int j = 0; j < assoc; j++)
	{
		if (cache[j].counter >= max_count)
		{
			max_count = cache[j].counter;
			loc = j;
		}

	}
	return loc;

}

void VMcache::increment_counter_DiffTag(int ref_counter)
{
	for (int j = 0; j < assoc; j++)
	{
		if (ref_counter > cache[j].counter)
			++cache[j].counter;

	}
}

void VMcache::DisplayResults()
{

	cout << "===== Victim Cache contents=====" << "\n";

	cout << "set 0:\t";

		for (int j = 0; j < assoc; j++)
		{
			cout  << cache[j].Tag_data << setw(5) << (((cache[j].bit_dirty == 1) ? "D" : ""))<<"\t";

		}


	}


long  int  VMcache::hex_to_decimal(string str)
{
	return (long int)stoll(str, 0, 16);
}


string VMcache::decimal_to_hex(long int lli_num)
{
	std::stringstream stream;
	stream << std::hex << lli_num;
	std::string result(stream.str());
	return result;
}

string VMcache::binary_to_hex(bitset<32> bit_num)
{
	stringstream res;
	res << hex << uppercase << bit_num.to_ullong();
	return res.str();

}

//used when there is a miss in VM.
void VMcache::VM_LRU_replace(string orig_tagdata, bitset<1>dirty_bit)
{
	
	L2cache *L2 = L2cache::get_L2cache_instance();
	string Tagdata;
	VM_formatted_Tagdata(orig_tagdata, Tagdata);
	int	Loc_max = VMcache::Loc_max_count();

	for (int j = 0; j < assoc; j++)
	{
		cache[j].counter++;
	}
	string temp_tagdata = cache[Loc_max].orig_tagdata;
	cache[Loc_max].orig_tagdata = orig_tagdata;
	cache[Loc_max].Tag_data = Tagdata;
	cache[Loc_max].bit_validity = 1;
	cache[Loc_max].counter = 0;

	if (cache[Loc_max].bit_dirty == 1)
	{
		writebacks++;

	}

	if ((L2!=NULL))
	{
		if (cache[Loc_max].bit_dirty == 1)
		{
			L2->writerequesttoL2cache(temp_tagdata, 1,1);		
			
		}
	}

	cache[Loc_max].bit_dirty = dirty_bit;

}

//find where it is a hit if hit then swap.
bool VMcache::VM_LRU_find(string orig_tagdata,int readorwrite)   //need to implement swap.
{
	L1cache *L1 = L1cache::get_L1cache_instance();
	int ref_counter;
	bool tag_found = false;
	string L1_tagdata;
	int L1_row_index;
	string temp_tagdata;
	string VM_Tagdata;
	bitset<1> temp_dirty;
	string temp_orig_data;	
	
	VM_formatted_Tagdata(orig_tagdata, VM_Tagdata);  //now Victim understands.
	//For read hit
	for (int j = 0; j < assoc; j++)
	{

		if (cache[j].Tag_data == VM_Tagdata)
		{   
			temp_dirty = cache[j].bit_dirty;
			temp_orig_data = cache[j].orig_tagdata;

			num_swaps++;
			L1->L1_formatted_Rowindex_Tagdata(orig_tagdata, L1_row_index, temp_tagdata); //only to know the row
			int	Loc_max = L1->Loc_max_count(L1_row_index);   
			int min_counter = L1->min_count(L1_row_index);

			if (L1->R_POLICY == 0) //if L1 uses LRU replacement policy which one to evict from L1
			{						
				VM_formatted_Tagdata(L1->cache[L1_row_index][Loc_max].orig_tagdata, temp_tagdata); //data to be placed VM.
				cache[j].Tag_data = temp_tagdata;
				cache[j].bit_dirty = L1->cache[L1_row_index][Loc_max].bit_dirty;
				cache[j].orig_tagdata = L1->cache[L1_row_index][Loc_max].orig_tagdata;

				L1->L1_formatted_Rowindex_Tagdata(orig_tagdata, L1_row_index, temp_tagdata);
				L1->cache[L1_row_index][Loc_max].orig_tagdata = temp_orig_data;
				L1->cache[L1_row_index][Loc_max].Tag_data = temp_tagdata;
				L1->cache[L1_row_index][Loc_max].bit_dirty = temp_dirty;


				for (int j = 0; j < L1->assoc; j++)  //L1 miss.
				{
					L1->cache[L1_row_index][j].counter++;
				}

				L1->cache[L1_row_index][Loc_max].counter = 0;

			/*	ref_counter = L1->cache[L1_row_index][Loc_max].counter;
			  	L1->increment_counter_DiffTag(L1_row_index, ref_counter);
				L1->cache[L1_row_index][Loc_max].counter = 0;*/
				

				ref_counter = cache[j].counter;   //VM hit
				increment_counter_DiffTag(ref_counter);
				cache[j].counter = 0;

				tag_found = true;
				break;

			}
			else  //if L1 uses LFU replacement policy
			{       
				for(int k=0;k<L1->assoc;k++)
						if ((L1->cache[L1_row_index][k].counter == min_counter))
						{   
						    VM_formatted_Tagdata(L1->cache[L1_row_index][k].orig_tagdata, temp_tagdata);
							cache[j].Tag_data = temp_tagdata;
							cache[j].bit_dirty = L1->cache[L1_row_index][k].bit_dirty;
							cache[j].orig_tagdata = L1->cache[L1_row_index][k].orig_tagdata;

							L1->L1_formatted_Rowindex_Tagdata(temp_orig_data, L1_row_index, temp_tagdata);
							L1->cache[L1_row_index][k].orig_tagdata = temp_orig_data;
							L1->cache[L1_row_index][k].Tag_data = temp_tagdata;
							if (readorwrite == 1)  //write to L1
							{
								L1->cache[L1_row_index][k].bit_dirty = 1;
							}
							else
							{
								L1->cache[L1_row_index][k].bit_dirty = temp_dirty;
							}
							L1->cache[L1_row_index][k].bit_validity = 1;

							ref_counter = cache[j].counter;   //VM hit
							increment_counter_DiffTag(ref_counter);
							cache[j].counter = 0;

							L1->cache[L1_row_index][k].counter++;
							tag_found = true;
							break;
						}
				}
			}
		}
	return tag_found;

	}


void VMcache::VM_formatted_Tagdata(string orig_tagdata, string &tag_data)
{

	long int d_tempaddress = VMcache::hex_to_decimal(orig_tagdata);
	d_tempaddress = d_tempaddress / blocksize;   //shift by block size
	tag_data = decimal_to_hex(d_tempaddress);

}



