


#include "cache.h"
#include "iomanip"
#include "iostream"



cache::cache(int size, int blocksize, int assoc, int R_POLICY)
{
	cache::blocksize = blocksize;
	cache::cachesize = size;
	cache::assoc = assoc;
	create_bit_mask();                 //need to initialise the cache here.
	int num_rows = (int)pow(2, indexselect);		    //create a dynamic array of L1_cell having a row of assoc size and
	count_set = new int[num_rows];

	for (int i = 0; i < num_rows; i++)
	{
		count_set[i] = 1000;
	}
	//column of 2^index.
	L1cache = new element *[num_rows];
	for (int i = 0; i < num_rows; ++i)
		L1cache[i] = new element[assoc];

	if (R_POLICY == 0)
	{
		if (assoc != 1)
		{
			for (int i = 0; i < num_rows; i++)
			{
				int count = assoc - 1;

				for (int j = 0; j < assoc; j++)
				{

					L1cache[i][j].bit_dirty.reset();           //resets a cell
					L1cache[i][j].bit_validity.reset();
					L1cache[i][j].counter = count;
					L1cache[i][j].Tag_data = "";
					L1cache[i][j].sBlock_Data = "";

					--count;
				}
			}
		}

		else
		{
			//	int count = num_rows - 1;

			for (int i = 0; i < num_rows; i++)
			{
				L1cache[i][0].counter = 1023;

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
				L1cache[i][j].bit_dirty.reset();           //resets a cell
				L1cache[i][j].bit_validity.reset();
				L1cache[i][j].counter = 0;
				L1cache[i][j].Tag_data = "";
				L1cache[i][j].sBlock_Data = "";

			}

		}

	}

	readmiss = 0;
	writemiss = 0;
	num_reads = 0;
	num_writes = 0;
	writebacks = 0;
	L1_miss_penalty = 20.000 + 0.5*(blocksize / 16.000);
        L1_hit_time = 0.25 + 2.5*((cachesize)/(512.000*1024)) + 0.025*((blocksize/16.000)) + 0.025*assoc;

}

void cache::create_bit_mask()
{

	muxselect = (int)log2(blocksize);
	indexselect = (int)log2((cachesize / (blocksize*assoc)));
	double selectionbits = muxselect + indexselect;
	index_bit_mask = (bitset<32>)0;
	for (int i = muxselect; i < selectionbits; i++)
		index_bit_mask.set(i, 1);
}

void cache::process_processor_instruction(string rw, string address)
{
	if (rw == "r")
		instruction_set.readorwrite = 0;            //read ==0
	else
		instruction_set.readorwrite = 1;            //write ==1

//	int num_cells = (cachesize / (blocksize*indexselect));
	double selectionbits = muxselect + indexselect;

	long int d_tempaddress = hex_to_decimal(address);
	bitset<32> b_tempaddress(d_tempaddress);
	long int d_tagdata = (long int)(d_tempaddress / pow(2, selectionbits));   //shift by selection bits to left to get tag
	instruction_set.tagdata = decimal_to_hex(d_tagdata);

	instruction_set.index = (b_tempaddress&index_bit_mask) >> muxselect;

	instruction_set.d_index = (instruction_set.index.to_ulong()); //can only range from 0 to indexselect or num of sets

	Instruct_array.push_back(instruction_set);

}


void cache::execute_processed_instruction(int LFU, int WTNA)
{
	R_POLICY = LFU;
	W_POLICY = WTNA;

	vector<cache_Inst>::iterator   itr_instruct = Instruct_array.end();                    //holds the instruction

	Instruct_array.pop_back();

	for (itr_instruct = Instruct_array.begin(); itr_instruct != Instruct_array.end(); itr_instruct++)
	{

		if (itr_instruct->readorwrite == 0)  //corresponds to the read
		{
			if (LFU == 1)
			{
				LFU_read(itr_instruct->tagdata, itr_instruct->d_index, WTNA);

			}
			else
			{
				LRU_read(itr_instruct->tagdata, itr_instruct->d_index, WTNA);

			}

		}

		else    //corresponds to write.
		{


			if (LFU == 1)
			{
				LFU_write(itr_instruct->tagdata, itr_instruct->d_index, WTNA);
			}
			else
			{

				LRU_write(itr_instruct->tagdata, itr_instruct->d_index, WTNA);

			}
		}
	}
}

int cache::min_count(int row_index)
{


	int min_count = std::numeric_limits<int>::max();


	for (int j = 0; j < assoc; j++)
	{
		if (min_count > L1cache[row_index][j].counter)
		{
			min_count = L1cache[row_index][j].counter;
		}
	}

	return min_count;

}

int cache::Loc_max_count(int row_index)
{
	int max_count = 0;
	int loc;


	for (int j = 0; j < assoc; j++)
	{
		if (L1cache[row_index][j].counter >= max_count)
		{
			max_count = L1cache[row_index][j].counter;
			loc = j;
		}

	}
	return loc;

}

void cache::increment_counter_DiffTag(int row_index, int ref_counter)
{
	for (int j = 0; j < assoc; j++)
	{
		if (ref_counter > L1cache[row_index][j].counter)
			++L1cache[row_index][j].counter;

	}
}

void cache::LRU_read(string Tagdata, int row_index, int WTNA)
{
	num_reads++;
	int ref_counter = 0;
	bitset<1> tag_found;
	tag_found.reset();


	//For read hit
	for (int j = 0; j < assoc; j++)
	{
		if (L1cache[row_index][j].Tag_data == Tagdata)
		{
			ref_counter = L1cache[row_index][j].counter;
			increment_counter_DiffTag(row_index, ref_counter);
			L1cache[row_index][j].counter = 0;
			tag_found.set(0, 1);
			break;

		}

	}
	//read miss here
	if (tag_found == 0)
	{
		int	Loc_max = cache::Loc_max_count(row_index);
		readmiss++;

		for (int j = 0; j < assoc; j++)
		{
			L1cache[row_index][j].counter++;
		}

		L1cache[row_index][Loc_max].Tag_data = Tagdata;
		L1cache[row_index][Loc_max].bit_validity = 1;
		L1cache[row_index][Loc_max].counter = 0;



		if (WTNA != 1)
		{
			if (L1cache[row_index][Loc_max].bit_dirty == 1)
				writebacks++;

		}
		L1cache[row_index][Loc_max].bit_dirty = 0; //it will load from main memory



	}

}

void cache::LRU_write(string Tagdata, int row_index, int WTNA)
{
	num_writes++;
	int ref_counter;
	bitset <1> tag_found;

	//For write hit
	for (int j = 0; j < assoc; j++)
	{
		if (L1cache[row_index][j].Tag_data == Tagdata)  //write hit
		{
			ref_counter = L1cache[row_index][j].counter;
			increment_counter_DiffTag(row_index, ref_counter);
			L1cache[row_index][j].counter = 0;
			//increment other counters according to LRU policy
			tag_found.set(0, 1);


			if (WTNA != 1)
			{

			//	if (L1cache[row_index][j].bit_dirty == 1)
			//		writebacks++;

				L1cache[row_index][j].bit_dirty = 1;

			}




			break;
		}

	}

	//write  miss  
	if (tag_found == 0)
	{
		if (WTNA)						//writeThrough Not Allocate
		{
			writemiss++;
		}
		else							// Writeback WriteAllocate
		{
			int	Loc_max = cache::Loc_max_count(row_index);

			writemiss++;

			for (int j = 0; j < assoc; j++)
			{
				L1cache[row_index][j].counter++;
			}

			L1cache[row_index][Loc_max].Tag_data = Tagdata;
			L1cache[row_index][Loc_max].bit_validity = 1;
			L1cache[row_index][Loc_max].counter = 0;


			if (L1cache[row_index][Loc_max].bit_dirty == 1)
				writebacks++;

			L1cache[row_index][Loc_max].bit_dirty = 1;



		}
	}

}


void cache::LFU_read(string Tagdata, int row_index, int WTNA)
{

	bitset<1> tag_found;
	tag_found.reset();
	num_reads++;

	//For read hit
	for (int j = 0; j < assoc; j++)
	{

		if (L1cache[row_index][j].bit_validity == 1)
		{
			if (L1cache[row_index][j].Tag_data == Tagdata)
			{
				//if referred increment the counter.
				L1cache[row_index][j].counter++;
				tag_found.set(0, 1);
				break;

			}
		}


	}


	if (tag_found == 0)  //read miss
	{
		readmiss++;


		for (int j = 0; j < assoc; j++)
		{
			int min_counter = min_count(row_index);

			if (L1cache[row_index][j].bit_validity == 0)
			{
				L1cache[row_index][j].Tag_data = Tagdata;
				L1cache[row_index][j].counter = count_set[row_index] + 1;  //set counter to min count (set_count+1)
				L1cache[row_index][j].bit_validity = 1; //set bit validity
				L1cache[row_index][j].bit_dirty = 0;  //since updated from main memory.[WBWA]
				break;
			}
			else
			{
				if (L1cache[row_index][j].counter == min_counter)   //element with min count found
				{
					L1cache[row_index][j].Tag_data = Tagdata;
					count_set[row_index] = L1cache[row_index][j].counter;
					L1cache[row_index][j].counter = count_set[row_index] + 1;  //set counter to set_counter+1

					if (WTNA != 1)
					{

						if (L1cache[row_index][j].bit_dirty == 1)
							writebacks++;

					}

					L1cache[row_index][j].bit_dirty = 0;  //updated from  Main memory .[WBWA]




					break;

				}


			}

		}

	}

}



void cache::LFU_write(string Tagdata, int row_index, int WTNA)
{

	bitset<1> tag_found;
	tag_found.reset();
	num_writes++;

	//For write hit
	for (int j = 0; j < assoc; j++)
	{

		if (L1cache[row_index][j].bit_validity == 1)
			if (Tagdata == L1cache[row_index][j].Tag_data)  //write hit
			{
				L1cache[row_index][j].counter++;// recent element

				if (WTNA != 1)
				{
				//	if (L1cache[row_index][j].bit_dirty == 0)
				//  	writebacks++;

					L1cache[row_index][j].bit_dirty = 1;         //update the dirty bit for [WBWA]			
				}

				tag_found.set(0, 1);
				break;


			}
	}

	//write  miss  
	if (tag_found == 0)
	{
		if (WTNA)  //writeThrough Not Allocate
		{

			writemiss++;

		}
		else   // Writeback WriteAllocate
		{

			writemiss++;


			for (int j = 0; j < assoc; j++)
			{
				int min_counter = min_count(row_index);

				if (L1cache[row_index][j].bit_validity == 0)
				{

					L1cache[row_index][j].Tag_data = Tagdata;
					L1cache[row_index][j].counter = count_set[row_index] + 1;  //set counter to min count (set_count+1)
					L1cache[row_index][j].bit_validity = 1; //set bit validity
					L1cache[row_index][j].bit_dirty = 1;  //write miss allocates and writes a value != main memory
					break;
				}
				else
				{
					if (L1cache[row_index][j].counter == min_counter)   //element with min count found
					{

						L1cache[row_index][j].Tag_data = Tagdata;
						count_set[row_index] = L1cache[row_index][j].counter;
						L1cache[row_index][j].counter = count_set[row_index] + 1;  //set counter to set_counter+1




						 if (L1cache[row_index][j].bit_dirty == 1)
							writebacks++;

						L1cache[row_index][j].bit_dirty = 1;

						break;

					}


				}

			}

		}

	}
}

void cache::DisplayResults()
{


	int num_rows = (int)pow(2, indexselect);


	cout << "==== Simulator configuration ====" << "\n";
	cout << "L1_BLOCKSIZE:" << setw(4) << blocksize<<"\n";
	cout << "L1_SIZE :" << setw(4) << cachesize<<"\n";
	cout << "L1_ASSOC :" << setw(4) << assoc<<"\n";
	cout << "L1_REPLACEMENT_POLICY :" << setw(4) << R_POLICY<<"\n";// R_POLICY
	cout << "L1_WRITE_POLICY :" << setw(4) << W_POLICY<<"\n";
	cout << "trace_file : " << setw(4) << filename<<"\n";
	cout << "==================================="<<"\n";

	for (int i = 0; i < num_rows; i++)
	{


		cout << "set" << i;

		for (int j = 0; j < assoc; j++)
		{
			cout << "\t" << L1cache[i][j].Tag_data << setw(5) << (((L1cache[i][j].bit_dirty == 1) ? "D" : ""));


		}

		cout << "\n";
	}
         double missrate = (double)((((double)(readmiss + writemiss) / (double)(num_reads + num_writes))));
	cout << "==== Simulation results(raw) ====" << "\n";
	cout << "a.number of L1 reads :" << "\t\t"<< num_reads<<"\n";
	cout << "b.number of L1 read misses :" << "\t" << readmiss<<"\n";
	cout << "c.number of L1 writes :" << "\t\t" << num_writes<<"\n";
	cout << "d.number of L1 write misses :" << "\t" << writemiss<<"\n";
	cout << "e.L1 miss rate :"<< "\t\t" << std::fixed << std::setprecision(4) << missrate<<"\n";
	cout << "f.number of writebacks from L1 :"<< writebacks<<"\n";
        if(W_POLICY==0)
	cout << "g.total memory traffic :"<< "\t" << (writemiss + readmiss + writebacks)<<"\n";
	else
	cout << "g.total memory traffic :"<< "\t" << (readmiss + num_writes)<<"\n";
	
	cout<<"   ==== Simulation results (performance) ===="<<"\n";
	cout<<"  1. average access time:            " <<std::fixed<<std::setprecision(4)<<L1_hit_time + missrate*L1_miss_penalty<<" ns";


}

 long  int  cache::hex_to_decimal(string str)
{   
	return (long int)stoll(str, 0, 16);
}


string cache::decimal_to_hex(long int lli_num)
{
	std::stringstream stream;
	stream << std::hex << lli_num;
	std::string result(stream.str());
	return result;
}

string cache::binary_to_hex(bitset<32> bit_num)
{
	stringstream res;
	res << hex << uppercase << bit_num.to_ullong();
	return res.str();

}



