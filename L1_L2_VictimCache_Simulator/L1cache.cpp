
#define VC_size 1
#define L2size 1

#include "L1cache.h"
#include "L2cache.h"
#include "victim.h"
#include "iomanip"
#include "iostream"

L1cache* L1cache::L1_cache = NULL;                                  //  initialize the static variable


void  L1cache::create_get_L1cache_instance(int size, int blocksize, int assoc, int R_POLICY, int W_POLICY)
{
	if (L1_cache == NULL)
		L1_cache = new L1cache(size, blocksize, assoc, R_POLICY, W_POLICY);

}

L1cache*  L1cache::get_L1cache_instance()
{

	return L1_cache;

}

L1cache::L1cache(int size, int blocksize, int assoc, int R_POLICY, int W_POLICY)
{
	L1cache::R_POLICY = R_POLICY;
	L1cache::W_POLICY = W_POLICY;
	L1cache::blocksize = blocksize;
	L1cache::cachesize = size;
	L1cache::assoc = assoc;
	create_bit_mask();                 //need to initialise the cache here.
	int num_rows = (int)pow(2, indexselect);		    //create a dynamic array of L1_cell having a row of assoc size and
	count_set = new int[num_rows];

	for (int i = 0; i < num_rows; i++)
	{
		count_set[i] = 1000;
	}
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
	L1_miss_penalty = 20.000 + 0.5*(blocksize / 16.000);
	L1_hit_time = 0.25 + 2.5*((cachesize) / (512.000 * 1024)) + 0.025*((blocksize / 16.000)) + 0.025*assoc;

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

void L1cache::process_processor_instruction(string rw, string address)
{
	if (rw == "r")
		instruction_set.readorwrite = 0;            //read ==0
	else
		instruction_set.readorwrite = 1;            //write ==1

	double selectionbits = muxselect + indexselect;
	instruction_set.orig_tagdata = address;
	long int d_tempaddress = hex_to_decimal(address);
	bitset<32> b_tempaddress(d_tempaddress);
	long int d_tagdata = (long int)(d_tempaddress / pow(2, selectionbits));   //shift by selection bits to left to get tag
	instruction_set.tagdata = decimal_to_hex(d_tagdata);

	instruction_set.index = (b_tempaddress&index_bit_mask) >> muxselect;

	instruction_set.d_index = (instruction_set.index.to_ulong()); //can only range from 0 to indexselect or num of sets

	Instruct_array.push_back(instruction_set);

}


void L1cache::execute_processed_instruction()
{
	bool tag_found = false;
	vector<cache_Inst>::iterator   itr_instruct = Instruct_array.end();                    //holds the instruction
	VMcache *VM = VMcache::get_VMcache_instance();
	L2cache *L2 = L2cache::get_L2cache_instance();
	L1cache *L1 = L1cache::get_L1cache_instance();

	Instruct_array.pop_back();

	for (itr_instruct = Instruct_array.begin(); itr_instruct != Instruct_array.end(); itr_instruct++)
	{

		if (itr_instruct->readorwrite == 0)  //corresponds to the read
		{
			L1->num_reads++;
/***************************************Read hit**********************************************/
			if (R_POLICY == 0)
			{
				tag_found = L1_LRU_find(itr_instruct->orig_tagdata, itr_instruct->d_index, itr_instruct->tagdata);
			}
			else
			{
				tag_found = L1_LFU_find(itr_instruct->orig_tagdata, itr_instruct->d_index, itr_instruct->tagdata);
			}
/****************************************Read miss**********************************************/
			if (tag_found == false)
			{

				L1->readmiss++;

				if (R_POLICY == 0)
				{

/***********************LRU****************Read miss L1 + L2 only*****************************/
					if ((L1!= NULL) && (L2 != NULL)&&(VM==NULL))
					{
						L1_LRU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 0, 1);

						L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0,0);
					}
/****************************LRU**************Read miss L1 + all*****************************/
					else 
					{
						bool L1_tag_bit_validity = true;

						for (int j = 0; j < assoc; j++)
						{
							if (cache[itr_instruct->d_index][j].bit_validity == 0)
							{
								L1_tag_bit_validity = false;
								if (L1_tag_bit_validity == false)
									break;
							}
						}
						if (L1_tag_bit_validity == false) //atleast one place is available to write in L1.
						{
							L1_LRU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 0, 1);
							if(L2!=NULL)
							L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0,0);
						}
						else                             //swap with Victim
						{

							if (VM->VM_LRU_find(itr_instruct->orig_tagdata,0)) { L1->readmiss--; } //swaps if found
							else
							{
								L1_LRU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 0, 1);//else replace
								if (L2 != NULL)
								L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0,0);
							}
						}
					}

				}
/***********************LFU****************read miss***********************************************/
				else
				{

/***********************LFU****************read miss L1 + L2 only*****************************************/
					if ((L1 != NULL) && (L2 != NULL) && (VM == NULL))
					{
						L1_LFU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 0, 1);
						if (L2 != NULL)
						L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0,0);
					}
/***********************LFU****************read miss L1+ all*******************************/
					else 
					{
						bool L1_tag_bit_validity = true;

						for (int j = 0; j < assoc; j++)
						{
							if (cache[itr_instruct->d_index][j].bit_validity == 0)
							{
								L1_tag_bit_validity = false;
								if (L1_tag_bit_validity == false)
									break;
							}
						}
						if (L1_tag_bit_validity == false) //atleast one place is available to write in L1.
						{
							L1_LFU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 0, 1);
							if (L2 != NULL)
								L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0,0);
						}
						else                             //swap with Victim
						{

							if (VM->VM_LRU_find(itr_instruct->orig_tagdata,0)) { L1->readmiss--; } //swaps if found
							else
							{
								L1_LFU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 0, 1);//else replace
								if (L2 != NULL)
									L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0,0);
							}
						}
					}
				}
			}
		}
/******************************************write ******************************************************/
		else    //corresponds to write.
		{
			L1->num_writes++;

			int ref_counter;
			tag_found = false;
/***************LRU************************write hit******************************************************/
			if (R_POLICY == 0)
			{
				//For L1 write hit LRU
				for (int j = 0; j < assoc; j++)
				{
					if (cache[itr_instruct->d_index][j].Tag_data == itr_instruct->tagdata)  //write hit
					{
						cache[itr_instruct->d_index][j].orig_tagdata = itr_instruct->orig_tagdata;
						ref_counter = cache[itr_instruct->d_index][j].counter;
						increment_counter_DiffTag(itr_instruct->d_index, ref_counter);
						cache[itr_instruct->d_index][j].counter = 0;
						tag_found = true;

							cache[itr_instruct->d_index][j].bit_dirty = 1;

						break;
					}

				}
			}
/***************LFU************************write hit******************************************************/
			else  //For L1 write hit LFU
			{

				for (int j = 0; j < assoc; j++)
				{

					if (cache[itr_instruct->d_index][j].bit_validity == 1)
						if (itr_instruct->tagdata == cache[itr_instruct->d_index][j].Tag_data)  //write hit
						{
							cache[itr_instruct->d_index][j].orig_tagdata = itr_instruct->orig_tagdata;
							cache[itr_instruct->d_index][j].counter++;// recent element


								cache[itr_instruct->d_index][j].bit_dirty = 1;       //from pr			

							tag_found = true;
							break;


						}
				}

			}

/******************************************write miss******************************************************/

			if (tag_found == false)
			{
				L1->writemiss++;

				if (R_POLICY == 0)
				{
/****************LRU***********************write L1+L2 only******************************************************/

					if ((L1 != NULL) && (L2 != NULL) && (VM == NULL))
					{
						L1_LRU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 1, 1);
						if (L2 != NULL)
						L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0,0);
					}
/****************LRU***********************write L1+victim all*****************************************************/

					else 
					{
						bool L1_tag_bit_validity = true;

						for (int j = 0; j < assoc; j++)
						{
							if (cache[itr_instruct->d_index][j].bit_validity == 0)
							{
								L1_tag_bit_validity = false;
								if (L1_tag_bit_validity == false)
									break;
							}
						}
						if (L1_tag_bit_validity == false) //atleast one place is available to write in L1.
						{
							L1_LRU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 1, 1);
							if (L2 != NULL)
								L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0,0);
						}
						else                             //swap with Victim
						{
							if (VM->VM_LRU_find(itr_instruct->orig_tagdata,1)) { L1->writemiss--; } //swaps if found
							else
							{
								L1_LRU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 1, 1);//else replace
								if (L2 != NULL)
									L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0,0);

							}
						}
					}

				}
/****************LFU***********************write miss******************************************************/
				else
				{
/****************LFU***********************write miss L1+ L2 only******************************************************/
					if ((L1 != NULL) && (L2 != NULL) && (VM == NULL))
					{
						L1_LFU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 1, 1);

						if (L2 != NULL)
							L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0,0);
					}
/****************LFU***********************write miss L1+ all******************************************************/
					else 
					{
						bool L1_tag_bit_validity = true;

						for (int j = 0; j < assoc; j++)
						{
							if (cache[itr_instruct->d_index][j].bit_validity == 0)
							{
								L1_tag_bit_validity = false;
								if (L1_tag_bit_validity == false)
									break;
							}
						}
						if (L1_tag_bit_validity == false) //atleast one place is available to write in L1.
						{
							L1_LFU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 1, 1);
							if (L2 != NULL)
								L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0, 0);
						}
						else                             //swap with Victim
						{

							if (VM->VM_LRU_find(itr_instruct->orig_tagdata,1)) { L1->writemiss--; } //swaps if found
							else
							{
								L1_LFU_replace(itr_instruct->d_index, itr_instruct->tagdata, itr_instruct->orig_tagdata, 1, 1);//else replace
								if (L2 != NULL)
									L2->writerequesttoL2cache(itr_instruct->orig_tagdata, 0, 0);
							}
						}
					}

				}
			}
		}
	}
}


int L1cache::min_count(int row_index)
{


	int min_count = std::numeric_limits<int>::max();


	for (int j = 0; j < assoc; j++)
	{
		if (min_count > cache[row_index][j].counter)
		{
			min_count = cache[row_index][j].counter;
		}
	}

	return min_count;

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


void L1cache::DisplayResults()
{
	L1cache *L1 = L1cache::get_L1cache_instance();
	L2cache *L2 = L2cache::get_L2cache_instance();
	VMcache *VM = VMcache::get_VMcache_instance();
	double L1missrate=0;
	double L2missrate=0;

	int total_memory_traffic = 0;
	int num_rows = (int)pow(2, indexselect);

	if ((L1 != NULL) && (L2 != NULL)&&(VM==NULL))
	{
		total_memory_traffic = L2->writemiss + L2->readmiss + L2->writebacks;
	}
	if ((L1 != NULL) && (L2 == NULL) && (VM != NULL))
	{
		total_memory_traffic = L1->writemiss + L1->readmiss + VM->writebacks;
	}
	if ((L1 != NULL) && (L2 != NULL) && (VM != NULL))
	{
		total_memory_traffic = L2->writemiss + L2->readmiss + L2->writebacks;
	}



	std::cout << "===== Simulator configuration =====" << "\n";
	std::cout << "L1_BLOCKSIZE:" << setw(4) << blocksize << "\n";
	std::cout << "L1_SIZE :" << setw(4) << cachesize << "\n";
	std::cout << "L1_ASSOC :" << setw(4) << assoc << "\n";
	std::cout << "Victim Cache_SIZE" << setw(4) << ((VM!=NULL)?VM->cachesize:0) << "\n";
	std::cout << "L2_SIZE :" << setw(4) << ((L2!=NULL)?L2->cachesize:0) << "\n";
	std::cout << "L2_ASSOC :" << setw(4) << ((L2!=NULL)?L2->assoc:0) << "\n";
	std::cout << "trace_file : " << setw(4) << filename << "\n";
	std::cout << "Replacement Policy" << setw(4) << ((R_POLICY == 0) ? "LRU" : "LFU");
	std::cout << "\n===================================" << "\n\n\n";
	std::cout << "==== L1 contents ====" << "\n";

	for (int i = 0; i < num_rows; i++)
	{


		std::cout << "set" << i;

		for (int j = 0; j < assoc; j++)
		{
			std::cout << "\t" << cache[i][j].Tag_data << setw(5) << (((cache[i][j].bit_dirty == 1) ? "D" : ""));


		}

		
		std::cout << "\n";
	}
	 L1missrate = (double)((((double)(readmiss + writemiss) / (double)(num_reads + num_writes))));
         
	
	if (VM != NULL)
	 VM->DisplayResults();
cout<<"\n"<<"===== L2 contents ====="<<"\n";
	
	if(L2!=NULL)
	  L2->DisplayResults();

	if(L2!=NULL)
	 L2missrate = (double)((((double)(L2->readmiss +L2-> writemiss) / (double)(L2->num_reads + L2->num_writes))));

	std::cout << "\n======  Simulation results ======" << "\n";
	std::cout << "a.number of L1 reads :" <<"\t\t\t" <<setw(2) << num_reads << "\n";
	std::cout << "b.number of L1 read misses :"  <<"\t\t"<< setw(2) << readmiss << "\n";
	std::cout << "c.number of L1 writes :"  <<"\t\t\t"<< setw(2) << num_writes << "\n";
	std::cout << "d.number of L1 write misses :"  <<"\t\t"<< setw(2) << writemiss << "\n";
	std::cout << "e.L1 miss rate :"  <<"\t\t\t"<< setw(2) << std::fixed << std::setprecision(4) << L1missrate << "\n";
	std::cout << "f.number of swaps :"  <<"\t\t\t"<< setw(6)<<((VM!=NULL)?VM->num_swaps:0)<<"\n";
	std::cout << "g.victim cache writeback :"  <<"\t\t"<< setw(1)<< ((VM!=NULL)?VM->writebacks:0)<<"\n";
	std::cout << "h.number of L2 reads :" <<"\t\t\t" << setw(6) << ((L2 != NULL) ? L2->num_reads:0)<<"\n";
	std::cout << "i.number of L2 read misses :" <<"\t\t"<< setw(1) << ((L2 != NULL) ? L2->readmiss : 0)<<"\n";
	std::cout << "j.number of L2 writes :"  <<"\t\t\t"<< setw(6) << ((L2 != NULL) ? L2->num_writes : 0)<<"\n";
	std::cout << "k.number of L2 write misses :"  <<"\t\t"<< setw(1) << ((L2 != NULL) ? L2->writemiss : 0)<<"\n";
	std::cout << "l.L2 miss rate :"  <<"\t\t\t"<< setw(6) << std::fixed << std::setprecision(4) << L2missrate << "\n";
	std::cout << "m.number of writebacks from L2 :"  <<"\t"<< setw(1)<< ((L2 != NULL) ? L2->writebacks : 0) << "\n";
	std::cout << "n.Total memory traffic :"  <<"\t\t"<< setw(6)<<total_memory_traffic << "\n\n";
  
	double AAT1 = L1_hit_time + L1missrate*L1_miss_penalty;
	double AAT2 = 0;
	if (L2 != NULL)
	{
		AAT2 = L1_hit_time + L1missrate*(L2->L2_hit_time + L2missrate*L2->L2_miss_penalty);
	}
	if (AAT2 != 0)
	{
		cout << "====== Simulation results (performance) ======" << "\n";
		cout << "  1. average access time:            " << std::fixed << std::setprecision(4) << AAT2 << " ns"<<"\n";

	}
	else
	{
		cout << "==== Simulation results (performance) ====" << "\n";
		cout << "  1. average access time:            " << std::fixed << std::setprecision(4) << AAT1 << " ns" << "\n";


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


bool L1cache::L1_LFU_find(string orig_tagdata, int row_index, string Tagdata)
{
	bool tag_found = false;

	//For read hit 
	for (int j = 0; j < assoc; j++)
	{


		if (cache[row_index][j].Tag_data == Tagdata)
		{
			cache[row_index][j].orig_tagdata = orig_tagdata;
			//if referred increment the counter.
			cache[row_index][j].counter++;
			tag_found = true;
			break;

		}

	}

	return tag_found;
}

void L1cache::L1_LRU_replace(int row_index, string Tagdata, string orig_tagdata, int dirty_bit1, int dirty_bit2)
{
	string temp_tagdata;

	bitset <1> tag_found;
	VMcache *VM = VMcache::get_VMcache_instance();
	L2cache *L2 = L2cache::get_L2cache_instance();

	int	Loc_max = L1cache::Loc_max_count(row_index);


	for (int j = 0; j < assoc; j++)
	{
		cache[row_index][j].counter++;
	}

	temp_tagdata = cache[row_index][Loc_max].orig_tagdata; //previous tag

	cache[row_index][Loc_max].orig_tagdata = orig_tagdata;
	cache[row_index][Loc_max].Tag_data = Tagdata;
	cache[row_index][Loc_max].bit_validity = 1;
	cache[row_index][Loc_max].counter = 0;

	if (temp_tagdata != "")
	{
		if (VM!=NULL)  //has victim may be L2
		{

			VM->VM_LRU_replace(temp_tagdata, cache[row_index][Loc_max].bit_dirty);

		}

		if ((L2 != NULL) && (VM == NULL))  //has  L2 only write back
		{
		
		if (cache[row_index][Loc_max].bit_dirty == 1)
		{
				L2->writerequesttoL2cache(temp_tagdata, dirty_bit2,1);

		}

		}
	}

	cache[row_index][Loc_max].bit_dirty = dirty_bit1;

}

void L1cache::L1_LFU_replace(int row_index, string Tagdata, string orig_tagdata, int dirty_bit1, int dirty_bit2)
{
	VMcache *VM = VMcache::get_VMcache_instance();
	L2cache *L2 = L2cache::get_L2cache_instance();

	int min_counter = min_count(row_index);

	for (int j = 0; j < assoc; j++)
	{
		if ((cache[row_index][j].bit_validity == 0) || (cache[row_index][j].counter == min_counter))
		{
			string temp_tagdata = cache[row_index][j].orig_tagdata;
			cache[row_index][j].Tag_data = Tagdata;
			cache[row_index][j].orig_tagdata = orig_tagdata;
			cache[row_index][j].counter++;//count_set[row_index] + 1;  //set counter to min count (set_count+1)
			cache[row_index][j].bit_validity = 1; //set bit validity

			if (temp_tagdata != "")
			{
				if (VM!=NULL)  //has  victim may be L2
				{

					VM->VM_LRU_replace(temp_tagdata, cache[row_index][j].bit_dirty);

				}

		     if((L2 != NULL) && (VM == NULL))  //has  L2 only
		     {

				if (cache[row_index][j].bit_dirty == 1)
				{
						L2->writerequesttoL2cache(temp_tagdata, dirty_bit2,1);

				}

				}
			}

			cache[row_index][j].bit_dirty = dirty_bit1;
			break;
		}

	}


}

