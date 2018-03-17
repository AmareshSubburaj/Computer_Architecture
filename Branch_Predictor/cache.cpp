


#include "cache.h"
#include "iomanip"
#include "iostream"


cache::cache(int M1, int N, int size, int blocksize, int assoc)  // gshare branch predictor
{
	cache::BTB_blocksize = blocksize;
	cache::BTB_cachesize = size;
	cache::BTB_assoc = assoc;

	gshare_indexsize = M1;
	GlobalShiftRegSize = N;
	GlobalHistoryRegisters = new bool[N];
	for (int i = 0; i < N; i++)             //initialise the GHR.
		GlobalHistoryRegisters[i] = false;

	int num_size = (int)pow(2, M1);
	gshare_BranPredictTable = new int[num_size];  //Prediction Table created.
	for (int i = 0; i < num_size; i++)             //initialise the GHR.
		gshare_BranPredictTable[i] = 2;

	create_bit_mask();                          //need to initialise the cache here.

	if ((BTB_cachesize != 0) && (assoc != 0))
	{

		int num_rows = (int)pow(2, BTB_indexselect);//create a dynamic array of L1_cell having a row of assoc size and

													//column of 2^index.
		L1cache = new element *[num_rows];
		for (int i = 0; i < num_rows; ++i)
			L1cache[i] = new element[assoc];

		if (assoc != 1)
		{
			for (int i = 0; i < num_rows; i++)
			{
				int count = assoc - 1;

				for (int j = 0; j < assoc; j++)
				{
					L1cache[i][j].counter = count;
					L1cache[i][j].Tag_data = "";

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

}

cache::cache(int K, int M1, int N, int M2, int size, int blocksize, int assoc) //hybrid branch predictor
{
	cache::BTB_blocksize = blocksize;

	cache::BTB_cachesize = size;
	cache::BTB_assoc = assoc;

	int num_counters = (int)pow(2, K);
	hybrid_BranPredictTable = new int[num_counters];
	for (int i = 0; i < num_counters; i++)
		hybrid_BranPredictTable[i] = 1;

	gshare_indexsize = M1;
	GlobalShiftRegSize = N;
	GlobalHistoryRegisters = new bool[N];
	for (int i = 0; i < N; i++)
		GlobalHistoryRegisters[i] = 0;

	num_counters = (int)pow(2, M2);
	Bimodel_BranPredictTable = new int[num_counters];  //Prediction Table created.
	for (int i = 0; i < num_counters; i++)
		Bimodel_BranPredictTable[i] = 2;

	num_counters = (int)pow(2, M1);
	gshare_BranPredictTable = new int[num_counters];
	for (int i = 0; i < num_counters; i++)
		gshare_BranPredictTable[i] = 2;

	create_bit_mask();                 //need to initialise the cache here.

	if ((BTB_cachesize != 0) && (assoc != 0))
	{

		int num_rows = (int)pow(2, BTB_indexselect);//create a dynamic array of L1_cell having a row of assoc size and

												//column of 2^index.
		L1cache = new element *[num_rows];

		for (int i = 0; i < num_rows; ++i)
			L1cache[i] = new element[assoc];

		if (assoc != 1)
		{
			for (int i = 0; i < num_rows; i++)
			{
				int count = assoc - 1;

				for (int j = 0; j < assoc; j++)
				{
					L1cache[i][j].counter = count;
					L1cache[i][j].Tag_data = "";

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
}

cache::cache(int M2, int size, int blocksize, int assoc)  //Bimodel
{
	cache::BTB_blocksize = blocksize;

	cache::BTB_cachesize = size;
	cache::BTB_assoc = assoc;


	int num_counters = (int)pow(2, M2);
	Bimodel_BranPredictTable = new int[num_counters];  //Prediction Table created.
	for (int i = 0; i < num_counters; i++)             //initialise the prediction table.
		Bimodel_BranPredictTable[i] = 2;

	create_bit_mask();

	if ((BTB_cachesize != 0) && (assoc != 0))
	{                             //need to initialise the cache here.
		int num_rows = (int)pow(2, BTB_indexselect);    //create a dynamic array of L1_cell having a row of assoc size and

														//column of 2^index.
		L1cache = new element *[num_rows];
		for (int i = 0; i < num_rows; ++i)
			L1cache[i] = new element[assoc];

		if (assoc != 1)
		{
			for (int i = 0; i < num_rows; i++)
			{
				int count = assoc - 1;

				for (int j = 0; j < assoc; j++)
				{
					L1cache[i][j].counter = count;
					L1cache[i][j].Tag_data = "";

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
}


void cache::create_bit_mask()
{

	BTB_muxselect = (int)log2(BTB_blocksize);

	if ((BTB_cachesize != 0) && (BTB_assoc != 0))
	{
		BTB_indexselect = (int)log2((BTB_cachesize / (BTB_blocksize*BTB_assoc)));
		double selectionbits = BTB_muxselect + BTB_indexselect;
		index_bit_mask = (bitset<32>)0;
		for (int i = BTB_muxselect; i < selectionbits; i++)
			index_bit_mask.set(i, 1);
	}
}

void cache::process_processor_instruction(int K, int M2, int M1, int N, string NT, string address, string MODEL)
{
	if (NT == "n")
		instruction_set.TakenBranch = false;            //not taken ==0
	else
		instruction_set.TakenBranch = true;            //taken ==1

	long int d_tempaddress = hex_to_decimal(address);
	bitset<32> b_tempaddress(d_tempaddress);
	instruction_set.PC = address;

	if (MODEL == "hybrid")
	{
		double  hybrid_selectionbits = BTB_muxselect + K;

		bitset <32> index_bit_mask = (bitset<32>)0;
		for (int i = BTB_muxselect; i < hybrid_selectionbits; i++)
			index_bit_mask.set(i, 1);

		instruction_set.hybrid_index = (b_tempaddress&index_bit_mask) >> BTB_muxselect;
		instruction_set.hybrid_d_index = (instruction_set.hybrid_index.to_ulong());

	}

	if ((MODEL == "bimodal") || (MODEL == "hybrid"))
	{
		double  bimodel_selectionbits = BTB_muxselect + M2;

		bitset <32> index_bit_mask = (bitset<32>)0;
		for (int i = BTB_muxselect; i < bimodel_selectionbits; i++)
			index_bit_mask.set(i, 1);

		instruction_set.bimodel_index = (b_tempaddress&index_bit_mask) >> BTB_muxselect;
		instruction_set.bimodel_d_index = (instruction_set.bimodel_index.to_ulong());
	}

	if ((MODEL == "gshare") || (MODEL == "hybrid"))
	{
		double  gshare_selectionbits = BTB_muxselect + M1;

		bitset <32> index_bit_mask = (bitset<32>)0;
		for (int i = BTB_muxselect; i < gshare_selectionbits; i++)
			index_bit_mask.set(i, 1);

		instruction_set.gshare_index = (b_tempaddress&index_bit_mask) >> BTB_muxselect;
		instruction_set.gshare_d_index = (instruction_set.gshare_index.to_ulong());

	}

	// for BTB table
	if ((BTB_cachesize != 0) && (BTB_assoc != 0))
	{
		double  BTB_selectionbits = BTB_muxselect + BTB_indexselect;
		long int d_tagdata = (long int)(d_tempaddress / pow(2, BTB_selectionbits));   //shift by selection bits to left to get tag
		instruction_set.tagdata = decimal_to_hex(d_tagdata);
		instruction_set.index = (b_tempaddress&index_bit_mask) >> BTB_muxselect;
		instruction_set.d_index = (instruction_set.index.to_ulong()); //can only range from 0 to indexselect or num of sets
	}
	Instruct_array.push_back(instruction_set);

}











void cache::execute_processed_instruction(string MODEL)
{

	vector<cache_Inst>::iterator   itr_instruct = Instruct_array.end();                    //holds the instruction
	bool BimodelTaken = false, gshareTaken = false;
	bool bimodelprediction = false, gshareprediction = false, hybridprediction = false;
	predictions = 0; mispredictions = 0 ;BP__predictions=0;NON_BP__predictions=0;
	Instruct_array.pop_back();

	for (itr_instruct = Instruct_array.begin(); itr_instruct != Instruct_array.end(); itr_instruct++)
	{  
		predictions++;

		if (MODEL == "bimodal")
		{
			if ((BTB_cachesize == 0) && (BTB_assoc == 0))
			{

				bimodelprediction = (TakenNotTaken(Bimodel_BranPredictTable[itr_instruct->bimodel_d_index]) == itr_instruct->TakenBranch) ? true : false;
				if (!bimodelprediction)mispredictions++;
				UpdateCounter(Bimodel_BranPredictTable[itr_instruct->bimodel_d_index], itr_instruct->TakenBranch);
			}
			else //BTB given
			{
				bool hit = LRU_read(itr_instruct->tagdata, itr_instruct->d_index);

				if (hit == true)
				{	BP__predictions++;
					bimodelprediction = (TakenNotTaken(Bimodel_BranPredictTable[itr_instruct->bimodel_d_index]) == itr_instruct->TakenBranch) ? true : false;
					if(!bimodelprediction)BP__mispredictions++;
					UpdateCounter(Bimodel_BranPredictTable[itr_instruct->bimodel_d_index], itr_instruct->TakenBranch);
				}
				else
				{
				NON_BP__predictions++;
				bimodelprediction = (false == itr_instruct->TakenBranch) ? true : false;			
									
				if(!bimodelprediction)
				BM__BTB_taken++;


				}

			}




		}
		else if (MODEL == "gshare")
		{


			if ((BTB_cachesize == 0) && (BTB_assoc == 0))
			{
				bitset<32> Shift(itr_instruct->gshare_d_index);
				bitset<32> shiftedvalue = Shift >> (gshare_indexsize - GlobalShiftRegSize);
				int A = shiftedvalue.to_ulong();
				bitset <32> mask = (bitset<32>)0;
				for (int i = 0; i < (gshare_indexsize - GlobalShiftRegSize); i++)
					mask.set(i, 1);
				int D = (Shift&mask).to_ulong();
				int B = BoolArrayintoint(GlobalHistoryRegisters);
				int C = A^B;
				bitset<32> S(C);
				S = S << (gshare_indexsize - GlobalShiftRegSize);
				C = S.to_ulong() + D;
				int gshare_index = C;

				gshareprediction = (TakenNotTaken(gshare_BranPredictTable[gshare_index]) == itr_instruct->TakenBranch) ? true : false;
				if (!gshareprediction)mispredictions++;
				UpdateCounter(gshare_BranPredictTable[gshare_index], itr_instruct->TakenBranch);

				BoolArrayShiftRight(GlobalHistoryRegisters, itr_instruct->TakenBranch);



			}
			else //BTB given
			{ 
				bool hit = LRU_read(itr_instruct->tagdata, itr_instruct->d_index);

				if (hit)
				{
					bitset<32> Shift(itr_instruct->gshare_d_index);
					bitset<32> shiftedvalue = Shift >> (gshare_indexsize - GlobalShiftRegSize);
					int A = shiftedvalue.to_ulong();
					bitset <32> mask = (bitset<32>)0;
					for (int i = 0; i < (gshare_indexsize - GlobalShiftRegSize); i++)
						mask.set(i, 1);
					int D = (Shift&mask).to_ulong();
					int B = BoolArrayintoint(GlobalHistoryRegisters);
					int C = A^B;
					bitset<32> S(C);
					S = S << (gshare_indexsize - GlobalShiftRegSize);
					C = S.to_ulong() + D;
					int gshare_index = C;
					
					BP__predictions++;
					gshareprediction = (TakenNotTaken(gshare_BranPredictTable[gshare_index]) == itr_instruct->TakenBranch) ? true : false;
					if(!gshareprediction)BP__mispredictions++;
					UpdateCounter(gshare_BranPredictTable[gshare_index], itr_instruct->TakenBranch);

					BoolArrayShiftRight(GlobalHistoryRegisters, itr_instruct->TakenBranch);
				}
				else
				{
				NON_BP__predictions++;
						
				gshareprediction = (false == itr_instruct->TakenBranch) ? true : false;				
									

				if(!gshareprediction)
				BM__BTB_taken++;

				}
			}

		}
		else if (MODEL == "hybrid")
		{
			if ((BTB_cachesize == 0) && (BTB_assoc == 0))
			{

				bitset<32> Shift(itr_instruct->gshare_d_index);
				bitset<32> shiftedvalue = Shift >> (gshare_indexsize - GlobalShiftRegSize);
				int A = shiftedvalue.to_ulong();
				bitset <32> mask = (bitset<32>)0;
				for (int i = 0; i < (gshare_indexsize - GlobalShiftRegSize); i++)
					mask.set(i, 1);
				int D = (Shift&mask).to_ulong();
				int B = BoolArrayintoint(GlobalHistoryRegisters);
				int C = A^B;
				bitset<32> S(C);
				S = S << (gshare_indexsize - GlobalShiftRegSize);
				C = S.to_ulong() + D;
				int gshare_index = C;

				BimodelTaken = (TakenNotTaken(Bimodel_BranPredictTable[itr_instruct->bimodel_d_index]) == itr_instruct->TakenBranch) ? true : false;


				gshareTaken = (TakenNotTaken(gshare_BranPredictTable[gshare_index]) == itr_instruct->TakenBranch) ? true : false;


				if (hybrid_BranPredictTable[itr_instruct->hybrid_d_index] >= 2)
				{	
					if(!gshareTaken)mispredictions++;
					UpdateCounter(gshare_BranPredictTable[gshare_index], itr_instruct->TakenBranch);

				}
				else
				{	
					if (!BimodelTaken)mispredictions++;
					UpdateCounter(Bimodel_BranPredictTable[itr_instruct->bimodel_d_index], itr_instruct->TakenBranch);
				}

				if (BimodelTaken == gshareTaken)
				{
					//do nothing
				}
				else if ((BimodelTaken==false)&&(gshareTaken==true))
				{
					UpdateCounter(hybrid_BranPredictTable[itr_instruct->hybrid_d_index], true);
				}
				else if ((gshareTaken==false)&&(BimodelTaken==true))
				{
					UpdateCounter(hybrid_BranPredictTable[itr_instruct->hybrid_d_index], false);

				}

				BoolArrayShiftRight(GlobalHistoryRegisters, itr_instruct->TakenBranch);



			}
			else   //BTB used
			{
				bool hit = LRU_read(itr_instruct->tagdata, itr_instruct->d_index);

				if (hit == true)
				{
					bitset<32> Shift(itr_instruct->gshare_d_index);
					bitset<32> shiftedvalue = Shift >> (gshare_indexsize - GlobalShiftRegSize);
					int A = shiftedvalue.to_ulong();
					bitset <32> mask = (bitset<32>)0;
					for (int i = 0; i < (gshare_indexsize - GlobalShiftRegSize); i++)
						mask.set(i, 1);
					int D = (Shift&mask).to_ulong();
					int B = BoolArrayintoint(GlobalHistoryRegisters);
					int C = A^B;
					bitset<32> S(C);
					S = S << (gshare_indexsize - GlobalShiftRegSize);
					C = S.to_ulong() + D;
					int gshare_index = C;
			
					BP__predictions++;
					

					BimodelTaken = (TakenNotTaken(Bimodel_BranPredictTable[itr_instruct->bimodel_d_index]) == itr_instruct->TakenBranch) ? true : false;


					gshareTaken = (TakenNotTaken(gshare_BranPredictTable[gshare_index]) == itr_instruct->TakenBranch) ? true : false;


					if (hybrid_BranPredictTable[itr_instruct->hybrid_d_index] >= 2)
					{
						UpdateCounter(gshare_BranPredictTable[gshare_index], itr_instruct->TakenBranch);
						if(!gshareTaken)BP__mispredictions++;

					}
					else
					{
						UpdateCounter(Bimodel_BranPredictTable[itr_instruct->bimodel_d_index], itr_instruct->TakenBranch);
						if(!BimodelTaken)BP__mispredictions++;
					}

					if (BimodelTaken == gshareTaken)
					{
						//do nothing
					}
					else if ((BimodelTaken==false)&&(gshareTaken==true))
					{
					UpdateCounter(hybrid_BranPredictTable[itr_instruct->hybrid_d_index], true);
					}
					else if ((gshareTaken==false)&&(BimodelTaken==true))
					{
					UpdateCounter(hybrid_BranPredictTable[itr_instruct->hybrid_d_index], false);

					}

					BoolArrayShiftRight(GlobalHistoryRegisters, itr_instruct->TakenBranch);
				}
				else
				{
				NON_BP__predictions++;
				hybridprediction = (false == itr_instruct->TakenBranch) ? true : false;				
									
				if(!hybridprediction)
				BM__BTB_taken++;				


				}


			}


		}

	}

}


int cache::Loc_max_count(int row_index)
{
	int max_count = 0;
	int loc;


	for (int j = 0; j < BTB_assoc; j++)
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
	for (int j = 0; j < BTB_assoc; j++)
	{
		if (ref_counter > L1cache[row_index][j].counter)
			++L1cache[row_index][j].counter;

	}
}

bool cache::LRU_read(string Tagdata, int row_index)
{

	int ref_counter = 0;
	bitset<1> tag_found;
	tag_found.reset();


	//For read hit
	for (int j = 0; j < BTB_assoc; j++)
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


		for (int j = 0; j < BTB_assoc; j++)
		{
			L1cache[row_index][j].counter++;
		}

		L1cache[row_index][Loc_max].Tag_data = Tagdata;
		L1cache[row_index][Loc_max].counter = 0;

		
	}

	if(tag_found ==0) return false; else return true;
	
}




void cache::DisplayResults(string MODEL, int K, int M1, int N, int M2, int SIZE, int ASSOC, string sfile)
{
	int num_rows;




if((BTB_cachesize==0)&& (BTB_assoc==0))
{	cout<<"COMMAND"<<"\n";
	if (MODEL == "bimodal")
	{
		cout << "./sim" << " " << MODEL << " " << M2 << " " << SIZE << " " << ASSOC << " " << sfile<<"\n";
		cout << "OUTPUT"<<"\n";
		cout << "number of predictions:" << "\t"<<predictions<< "\n";
		cout << "number of mispredictions:" << "\t"<<mispredictions<<"\n";
		cout << "misprediction rate:" << "\t"<<std::fixed<<std::setprecision(2)<<((double)(mispredictions*100)/(double)predictions)<<"%"<<"\n";
	}

	if (MODEL == "gshare")
	{
		cout << "./sim" << " " << MODEL << " " << M1 << " "<<N<<" " << SIZE << " " << ASSOC << " " << sfile << "\n";
		cout << "OUTPUT"<<"\n";
		cout << "number of predictions:" << "\t"<<predictions<< "\n";
		cout << "number of mispredictions:" << "\t"<<mispredictions<< "\n";
		cout << "misprediction rate:" << "\t"<<std::fixed<<std::setprecision(2)<<((double)(mispredictions*100)/(double)predictions)<<"%"<< "\n";
	}
	if (MODEL == "hybrid")
	{
		cout << "./sim" << " " << MODEL << " "<<K<<" " << M1 << " " << N << " "<<M2<<" " << SIZE << " " << ASSOC << " " << sfile << "\n";
		cout << "OUTPUT"<< "\n";
		cout << "number of predictions:" << "\t"<<predictions<< "\n";
		cout << "number of mispredictions:" << "\t"<<mispredictions<< "\n";
		cout << "misprediction rate:" << "\t"<<std::fixed<<std::setprecision(2)<<((double)(mispredictions*100)/(double)predictions)<<"%"<< "\n";
	}
}







	if ((BTB_cachesize != 0) && (BTB_assoc != 0))
	{
	mispredictions = BP__mispredictions+BM__BTB_taken;
	cout<<"COMMAND"<<"\n";
	if (MODEL == "bimodal")
	{
		cout <<"./sim" << " " << MODEL << " " << M2 << " " << SIZE << " " << ASSOC << " " << sfile<<"\n";
		cout <<"OUTPUT"<<"\n";
		cout<< "size of BTB:"<< SIZE<<"\n";
		cout <<"number of branches:" << "\t"<<predictions<< "\n";
		cout<< "number of predictions from branch predictor:"<<BP__predictions<<"\n";
		cout<< "number of mispredictions from branch predictor:"<<BP__mispredictions<<"\n";
		cout<< "number of branches miss in BTB and taken:"<<BM__BTB_taken<<"\n";
		cout<< "total mispredictions:"<< mispredictions<<"\n";
		cout << "misprediction rate:" << "\t"<<std::fixed<<std::setprecision(2)<<((double)(mispredictions*100)/(double)predictions)<<"%"<<"\n";
	}

	if (MODEL == "gshare")
	{
		cout << "./sim" << " " << MODEL << " " << M1 << " "<<N<<" " << SIZE << " " << ASSOC << " " << sfile << "\n";
		cout <<"OUTPUT"<<"\n";
		cout<< "size of BTB:"<< SIZE<<"\n";
		cout <<"number of branches:" << "\t"<<predictions<< "\n";
		cout<< "number of predictions from branch predictor:"<<BP__predictions<<"\n";
		cout<< "number of mispredictions from branch predictor:"<<BP__mispredictions<<"\n";
		cout<< "number of branches miss in BTB and taken:"<<BM__BTB_taken<<"\n";
		cout<< "total mispredictions:"<< mispredictions<<"\n";
		cout << "misprediction rate:" << "\t"<<std::fixed<<std::setprecision(2)<<((double)(mispredictions*100)/(double)predictions)<<"%"<<"\n";

	}
	if (MODEL == "hybrid")
	{
		cout << "./sim" << " " << MODEL << " "<<K<<" " << M1 << " " << N << " "<<M2<<" " << SIZE << " " << ASSOC << " " << sfile << "\n";
		cout <<"OUTPUT"<<"\n";
		cout<< "size of BTB:"<< SIZE<<"\n";
		cout <<"number of branches:" << "\t"<<predictions<< "\n";
		cout<< "number of predictions from branch predictor:"<<BP__predictions<<"\n";
		cout<< "number of mispredictions from branch predictor:"<<BP__mispredictions<<"\n";
		cout<< "number of branches miss in BTB and taken:"<<BM__BTB_taken<<"\n";
		cout<< "total mispredictions:"<< mispredictions<<"\n";
		cout << "misprediction rate:" << "\t"<<std::fixed<<std::setprecision(2)<<((double)(mispredictions*100)/(double)predictions)<<"%"<<"\n";
	}


		int num_rows = (int)pow(2, BTB_indexselect);
		cout << "\nFINAL BTB CONTENTS" << "\n";
		for (int i = 0; i < num_rows; i++)
		{


			cout << "set " << i<<"	"<<":";

			for (int j = 0; j < BTB_assoc; j++)
			{
				cout << "\t" << L1cache[i][j].Tag_data;


			}

			cout << "\n";
		}

	}
	else{

	}
	if (MODEL == "bimodal")
	{
		cout << "FINAL BIMODAL CONTENTS" << "\n";
		num_rows = pow(2, M2);
		for (int i = 0; i < num_rows; i++)
		{
			cout<<i<<"\t"<<Bimodel_BranPredictTable[i]<<"\n";
		}

     }
	if (MODEL == "gshare")
	{
		cout << "FINAL GSHARE CONTENTS" << "\n";
		num_rows = pow(2, M1);

		for (int i = 0; i < num_rows; i++)
		{
			cout << i << "\t" << gshare_BranPredictTable[i] << "\n";
		}

	}
	if (MODEL == "hybrid")
	{
		cout << "FINAL CHOOSER CONTENTS" << "\n";
		num_rows = pow(2, K);

		for (int i = 0; i < num_rows; i++)
		{
			cout << i << "\t" << hybrid_BranPredictTable[i] << "\n";
		}

		cout << "FINAL GSHARE CONTENTS" << "\n";
		num_rows = pow(2, M1);

		for (int i = 0; i < num_rows; i++)
		{
			cout << i << "\t" << gshare_BranPredictTable[i] << "\n";
		}

		cout << "FINAL BIMODAL CONTENTS" << "\n";
		num_rows = pow(2, M2);

		for (int i = 0; i < num_rows; i++)
		{
			cout << i << "\t" << Bimodel_BranPredictTable[i] << "\n";
		}
	}


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

void cache::UpdateCounter(int &Counter, bool Taken)
{
	switch (Taken)
	{

	case true:
	{
		switch (Counter) {

		case 0:  Counter = 1; break;
		case 1:  Counter = 2; break;
		case 2:  Counter = 3; break;
		case 3:  Counter = 3; break;
		}
	}

	break;

	case false:
	{
		switch (Counter) {

		case 0:  Counter = 0; break;
		case 1:  Counter = 0; break;
		case 2:  Counter = 1; break;
		case 3:  Counter = 2; break;

		}
	}

	}

}

bool cache::TakenNotTaken(int Counter)
{
	if ((Counter == 0) || (Counter == 1))
		return false;
	else if ((Counter == 2) || (Counter == 3))
		return true;

	return false;
}


void cache::BoolArrayShiftRight(bool GlobalShiftReg[], bool Taken)
{
	for (int i = 0; i <= GlobalShiftRegSize - 1; i++)
	{
		if (i != (GlobalShiftRegSize - 1))
			GlobalShiftReg[i] = GlobalShiftReg[i + 1];
		else
		{
			if (Taken)
				GlobalShiftReg[GlobalShiftRegSize - 1] = 1;
			else
				GlobalShiftReg[GlobalShiftRegSize - 1] = 0;
		}

	}

}

int cache::BoolArrayintoint(bool GlobalShiftReg[])
{


	int val = 0, temp;
	for (int i = 0; i < GlobalShiftRegSize; i++)
	{
		if (GlobalShiftReg[i] == true) temp = 1; else temp = 0;
		val += (temp * pow(2, i));
	}

	return val;

}
