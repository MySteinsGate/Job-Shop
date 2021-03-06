#include "jobshopgui_ga.h"
/*************** GA global vals ******************/
int product_count;
int proced_count;
int chromo_len;
int machine_count;

/***************************** GUI MAIN FUNC****************************/

CHROMO jobshop_GA_GUI(const DVector2D<PROCEDURE>& jobtable)
{
	product_count = jobtable.size();
	proced_count = jobtable[0].size();
	machine_count = proced_count;
	chromo_len = product_count * proced_count;

	DVector<CHROMO>* poparr = new DVector<CHROMO>[POP_NUM];

	for (auto i = 0; i < POP_NUM; ++i) {
		poparr[i].reserve(POPULATION_SIZE * 2);
		init_population(poparr[i], jobtable);
	}

	int local_best_cout = 0;
	CHROMO best_CHROMO;
	best_CHROMO.time = 1 << 22;

	for (int i = 1; i <= ITERATION_COUNT && local_best_cout < ITERATION_COUNT * 1 / 3; ++i) {

		TIME last_best_time = best_CHROMO.time;

		// communicate
		if (i % COMMUNACATION_CYCLE == 0) {
			communication(poparr);
		}
		
		// main part
		for (int k = 0; k < POP_NUM; ++k) {
			selfing_doubleseg(jobtable, poparr[k]);

			mutate_six(jobtable, poparr[k]);

			filter_by_sort(poparr[k]);

			if (i == ITERATION_COUNT) {
				break;
			}
		}

		// get best
		for (int i = 0; i < POP_NUM; i++) {
			if (poparr[i][0].time < best_CHROMO.time) {
				best_CHROMO = poparr[i][0];
			}
		}
		if (last_best_time == best_CHROMO.time) {
			++local_best_cout;
		}
		else {
			local_best_cout = 0;
		}
	}
	delete[] poparr;

	return best_CHROMO;

} // CHROMO jobshop_GA_GUI(...)



/**************************** public subfunc ****************************/

void communication(DVector<CHROMO>* pop) {
	DVector<CHROMO> tmp = pop[0];
	for (int i = 0; i < POP_NUM - 1; i++) {
		for (int k = 0; k < POPULATION_SIZE; k += POPULATION_SIZE / 10) {
			pop[i][k] = pop[i + 1][k];
		}
	}
	for (int k = 0; k < POPULATION_SIZE; k += POPULATION_SIZE / 10)
		pop[POP_NUM - 1][k] = tmp[k];
} //void communication(...)


void init_population(DVector<CHROMO>& pop, const DVector2D<PROCEDURE>& jobtable)
{
	DVector<int> init_chromo;
	for (int i = 0; i < product_count; ++i) {
		for (int j = 0; j < proced_count; ++j) {
			init_chromo.push_back(i);
		}
	}

	DVector<int> tmpchr;
	for (int i = 0; i < POPULATION_SIZE; ++i) {
		tmpchr = init_chromo.shuffle();
		TIME t = chromo_time(jobtable, tmpchr, 0);
		pop.push_back({ tmpchr, t });
	}
} // void init_population(...)


void selfing_doubleseg(const DVector2D<PROCEDURE>& jobtable, DVector<CHROMO>& pop)
{
	int p[4]{ 0 };
	int pop_orisize = pop.size();
	for (int i = 0; i < POPULATION_SIZE * CROSSOVER_RATE; ++i) {

		while (p[0] == p[1] || p[0] == p[2] || p[0] == p[3] || p[1] == p[2] || p[1] == p[3] || p[2] == p[3]) {
			p[0] = dtl::xorshiftRand() % (chromo_len + 1);
			p[1] = dtl::xorshiftRand() % (chromo_len + 1);
			p[2] = dtl::xorshiftRand() % (chromo_len + 1);
			p[3] = dtl::xorshiftRand() % (chromo_len + 1);
		}
		for (int i = 2, flag = 1, temp; i >= 0 && flag == 1; --i) {
			flag = 0;
			for (int j = 0; j <= i; ++j) {
				if (p[j] > p[j + 1]) {
					temp = p[j];
					p[j] = p[j + 1];
					p[j + 1] = temp;
					flag = 1;
				}
			}
		}
		pop.resize(pop.size() + 1);
		pop[pop_orisize + i].chromo.resize(chromo_len);

		int parent = dtl::xorshiftRand() % POPULATION_SIZE;

		int k = 0;
		for (int j = 0; j < p[0]; ++j) {
			pop[pop_orisize + i].chromo[k] = pop[parent].chromo[j];
			++k;
		}
		for (int j = p[2]; j < p[3]; ++j) {
			pop[pop_orisize + i].chromo[k] = pop[parent].chromo[j];
			++k;
		}
		for (int j = p[1]; j < p[2]; ++j) {
			pop[pop_orisize + i].chromo[k] = pop[parent].chromo[j];
			++k;
		}
		for (int j = p[0]; j < p[1]; ++j) {
			pop[pop_orisize + i].chromo[k] = pop[parent].chromo[j];
			++k;
		}
		for (int j = p[3]; j < chromo_len; ++j) {
			pop[pop_orisize + i].chromo[k] = pop[parent].chromo[j];
			++k;
		}

		pop[pop_orisize + i].time = chromo_time(jobtable, pop[pop_orisize + i].chromo, 1);
	}
} // void selfing_doubleseg(...)


void filter_by_sort(DVector<CHROMO>& pop)
{
	qsort(pop, 0, pop.size() - 1);
	pop.erase(POPULATION_SIZE, pop.size());
} // void filter_by_sort(...)


DVector<CHROMO> son;
void mutate_six(const DVector2D<PROCEDURE>& jobtable, DVector<CHROMO>& pop)
{
	son.resize(5);
	for (int i = 0; i < 5; ++i) {
		son[i].chromo.resize(chromo_len);
	}
	int pos1 = dtl::xorshiftRand() % chromo_len;
	int pos2 = dtl::xorshiftRand() % chromo_len;
	int pos3 = dtl::xorshiftRand() % chromo_len;
	for (int i = 0; i < POPULATION_SIZE; ++i) {
		son[0] = pop[i];
		son[0].chromo.swap(pos2, pos3);

		son[1] = pop[i];
		son[1].chromo.swap(pos1, pos3);

		son[2] = son[1];
		son[2].chromo.swap(pos1, pos3);

		son[3] = pop[i];
		son[3].chromo.swap(pos1, pos3);

		son[4] = son[3];
		son[4].chromo.swap(pos1, pos2);

		int bestpos = -1;
		TIME besttime = pop[i].time;
		for (int j = 0; j < 5; ++j) {
			son[j].time = chromo_time(jobtable, son[j].chromo, 1);
			if (son[j].time < besttime) {
				besttime = son[j].time;
				bestpos = j;
			}
		}
		if (0 <= bestpos && bestpos < 5) {
			pop[i] = son[bestpos];
		}
	}

} // void mutate_six(...)


/************************private funcs****************************/
CHROMO pivot;
int partition(DVector<CHROMO>& pop, int lo, int hi)
{
	//pop.swap(lo, lo + dtl::xorshiftRand() % (hi - lo + 1));
	pivot = pop[lo];
	while (lo < hi) {
		while (lo < hi && pop[hi].time >= pivot.time) {
			--hi;
		}
		pop[lo] = pop[hi];
		while (lo < hi && pop[lo].time <= pivot.time) {
			++lo;
		}
		pop[hi] = pop[lo];
	}
	pop[lo] = pivot;
	return lo;
} // int partition(...)


void qsort(DVector<CHROMO>& pop, int lo, int hi)
{
	if (lo < hi) {
		int pivotpos = partition(pop, lo, hi);
		qsort(pop, lo, pivotpos - 1);
		qsort(pop, pivotpos + 1, hi);
	}
} // void qsort(...)


DVector<int> machine_spare;
DVector<int> process;
DVector2D<TIME> endtime;
TIME chromo_time(const DVector2D<PROCEDURE>& jobtable, const DVector<int>& chromo, int flag)
{
	static bool fir = true;
	if (fir) {
		process.resize(product_count, 0);
		machine_spare.resize(machine_count, 0);
		endtime.resize(product_count);
		for (int i = 0; i < product_count; ++i) {
			endtime[i].resize(proced_count, 0);
		}
		fir = false;
	}
	else {
		for (int i = 0; i < product_count; ++i) {
			process[i] = 0;
		}
		for (int i = 0; i < machine_count; ++i) {
			machine_spare[i] = 0;
		}
		for (int i = 0; i < product_count; ++i) {
			for (int j = 0; j < proced_count; ++j) {
				endtime[i][j] = 0;
			}
		}
	}

	int product = 0;
	int proced = 0;
	int machine = 0;
	TIME begtime = 0;
	TIME lasting = 0;
	for (int i = 0; i < chromo_len; ++i) {
		int a = machine_spare.size();
		product = chromo[i];
		proced = process[product];
		if (proced == 0) {
			machine = jobtable[product][proced].machine;
			begtime = machine_spare[machine];
			lasting = jobtable[product][proced].duration;
			endtime[product][proced] = begtime + lasting;
			machine_spare[machine] = begtime + lasting;
		}
		else {
			machine = jobtable[product][proced].machine;
			if (machine_spare[machine] >= endtime[product][proced - 1]) {
				begtime = machine_spare[machine];
			}
			else {
				begtime = endtime[product][proced - 1];
			}
			lasting = jobtable[product][proced].duration;
			endtime[product][proced] = begtime + lasting;
			machine_spare[machine] = begtime + lasting;
		}
		++process[product];
	}
	TIME tottime = 0;
	int machine_spare_size = machine_spare.size();
	for (int i = 0; i < machine_spare_size; ++i) {
		if (tottime < machine_spare[i]) {
			tottime = machine_spare[i];
		}
	}
	return tottime;
}


// for testing, having lots of repeating codes w/ chromo_time()
DVector2D<MACHINE_TASK> get_result_table(const DVector2D<PROCEDURE>& jobtable, const CHROMO& CHR)
{
	DVector2D<MACHINE_TASK> res;
	res.resize(machine_count);
	for (auto i = 0; i < machine_count; ++i) {
		res[i].resize(product_count);
	}
	DVector<int> called;
	called.resize(machine_count, 0);

	/*******repeating start*******/
	static bool fir = true;
	if (fir) {
		process.resize(product_count, 0);
		machine_spare.resize(machine_count, 0);
		endtime.resize(product_count);
		for (int i = 0; i < product_count; ++i) {
			endtime[i].resize(proced_count, 0);
		}
		fir = false;
	}
	else {
		for (int i = 0; i < product_count; ++i) {
			process[i] = 0;
		}
		for (int i = 0; i < machine_count; ++i) {
			machine_spare[i] = 0;
		}
		for (int i = 0; i < product_count; ++i) {
			for (int j = 0; j < proced_count; ++j) {
				endtime[i][j] = 0;
			}
		}
	}

	int product = 0;
	int proced = 0;
	int machine = 0;
	TIME begtime = 0;
	TIME lasting = 0;
	for (int i = 0; i < chromo_len; ++i) {
		int a = machine_spare.size();
		product = CHR.chromo[i];
		proced = process[product];
		if (proced == 0) {
			machine = jobtable[product][proced].machine;
			begtime = machine_spare[machine];
			lasting = jobtable[product][proced].duration;
			endtime[product][proced] = begtime + lasting;
			machine_spare[machine] = begtime + lasting;
		}
		else {
			machine = jobtable[product][proced].machine;
			if (machine_spare[machine] >= endtime[product][proced - 1]) {
				begtime = machine_spare[machine];
			}
			else {
				begtime = endtime[product][proced - 1];
			}
			lasting = jobtable[product][proced].duration;
			endtime[product][proced] = begtime + lasting;
			machine_spare[machine] = begtime + lasting;
		}
		/*******repeating end*******/
		res[machine][called[machine]].product = product;
		res[machine][called[machine]].proced = proced;
		res[machine][called[machine]].start = begtime;
		res[machine][called[machine]].end = begtime + lasting;
		++called[machine];

		++process[product];
	}
	return res;
} // DVector2D<MACHINE_TASK> get_result_table(...)


TIME chromo_time_with_service(const DVector2D<PROCEDURE>& jobtable, const DVector<SRVQUEST>& srvquest_src,
	const DVector<int>& chromo, int flag)
{
	DVector<SRVQUEST> srvquest;

	static bool fir = true;
	if (fir) {
		process.resize(product_count, 0);
		machine_spare.resize(machine_count, 0);
		endtime.resize(product_count);
		for (int i = 0; i < product_count; ++i) {
			endtime[i].resize(proced_count, 0);
		}
		fir = false;
	}
	else {
		for (int i = 0; i < product_count; ++i) {
			process[i] = 0;
		}
		for (int i = 0; i < machine_count; ++i) {
			machine_spare[i] = 0;
		}
		for (int i = 0; i < product_count; ++i) {
			for (int j = 0; j < proced_count; ++j) {
				endtime[i][j] = 0;
			}
		}
	}
	//以上为 把工序号置为0，机器的开始时间置为0，工序结束时间置为0 

	int product = 0;//工件号 
	int proced = 0;//工序号 
	int machine = 0;//机器号 
	int fix_num = srvquest.size();
	TIME begtime = 0;//开始时间 
	TIME lasting = 0;//消耗时间

	for (int i = 0, k = 0; i < chromo_len; ++i) {  //遍历染色体 
		int a = machine_spare.size();
		product = chromo[i];
		proced = process[product];

		machine = jobtable[product][proced].machine;//获取机器号
													//获取机器号后开始询问此台机器是否要检修
		for (k = 0; k < fix_num; ++k) { //遍历检修表，看是否有此机器 
			if (machine == srvquest[k].machine) {
				break;
			}//一旦发现有此机器，就跳出，保留k，即保留检修的信息 
		}
		if (k < fix_num) { //也就是说此台机器需要检修 
							  //下面判断是否要开始检修了
							  //如果检修的开始时间比机器空闲的时间早，那么开始检修（当机器还没被调用完的时候） 
			if (srvquest[k].start <= machine_spare[machine]) {
				machine_spare[machine] = machine_spare[machine] + srvquest[k].duration;
				srvquest[k].machine = -1;//检修完后，就把它从检修表里踢出去 
			}
			else if (proced != 0 && srvquest[k].start > machine_spare[machine] 
				&& srvquest[k].start <= endtime[product][proced - 1]) {
				machine_spare[machine] = srvquest[k].start  + srvquest[k].duration;
				srvquest[k].machine = -1;//检修完后，就把它从检修表里踢出去
			}
		}

		if (proced == 0) { //如果是第一道工序，没有上一道工序 ，所以只关注机器是否空闲 

			begtime = machine_spare[machine];
			lasting = jobtable[product][proced].duration;
			endtime[product][proced] = begtime + lasting;//更新该工序结束时间 
			machine_spare[machine] = begtime + lasting;//更新该机器空闲时间点 
		}
		else { //如果不是第一道工序 

			if (machine_spare[machine] >= endtime[product][proced - 1]) { //比较一下上一道工序做完的时间和目前工序所用机器空闲的时间，取较大值为该工序开始时间 
				begtime = machine_spare[machine];
			}
			else {
				begtime = endtime[product][proced - 1];
			}
			lasting = jobtable[product][proced].duration;
			endtime[product][proced] = begtime + lasting;//更新该工序结束时间
			machine_spare[machine] = begtime + lasting;//更新该机器空闲时间点
		}
		++process[product];//该工序做完了，更新工序号，为下一道工序做准备 
	}
	//出来后再遍历一遍检修表，因为可能出现这两种情况
	//第一种，假设这台机器在最后一次调用的时候，机器的结束时间为 520 ，而检修开始时间为 521 ，那么这个检修在上述循环中无法操作
	//第二种，假设这台机器已经被调用完了，但是呢，TMD，还有一大堆的维修指令，那就修呗，无奈
	for (int i = 0; i < fix_num; ++i) {
		if (srvquest[i].machine != -1) {//这些就是那些还未处理的指令
										  //下面就分两种情况
										  //第一种，结束时间比维修开始时间大
										  //第二种，结束时间比维修开始时间小
			if (machine_spare[srvquest[i].machine] >= srvquest[i].start) { //第一种情况 
				machine_spare[srvquest[i].machine] = machine_spare[srvquest[i].machine] + srvquest[i].duration;
			}
			else { //第二种情况 
				machine_spare[srvquest[i].machine] = srvquest[i].start + srvquest[i].duration;
			}
		}
	}

	TIME tottime = 0;
	int machine_spare_size = machine_spare.size();
	for (int i = 0; i < machine_spare_size; ++i) {
		if (tottime < machine_spare[i]) {
			tottime = machine_spare[i];
		}
	}
	return tottime;
} // chromo_time_with_service


DVector2D<MACHINE_TASK> get_result_table_with_service(
	const DVector2D<PROCEDURE>& jobtable, const DVector<SRVQUEST>& srvquest_src, const CHROMO& CHR) {

	DVector<SRVQUEST> srvquest = srvquest_src;

	DVector2D<MACHINE_TASK> res;
	res.resize(machine_count);
	for (auto i = 0; i < machine_count; ++i) {
		res[i].resize(product_count);
		for (auto j = 0; j < product_count; ++j) {
			res[i][j].isService = false;
		}
	}
	DVector<int> called;
	called.resize(machine_count, 0);

	/*******repeating start*******/
	static bool fir = true;
	if (fir) {
		process.resize(product_count, 0);
		machine_spare.resize(machine_count, 0);
		endtime.resize(product_count);
		for (int i = 0; i < product_count; ++i) {
			endtime[i].resize(proced_count, 0);
		}
		fir = false;
	}
	else {
		for (int i = 0; i < product_count; ++i) {
			process[i] = 0;
		}
		for (int i = 0; i < machine_count; ++i) {
			machine_spare[i] = 0;
		}
		for (int i = 0; i < product_count; ++i) {
			for (int j = 0; j < proced_count; ++j) {
				endtime[i][j] = 0;
			}
		}
	}
	int product = 0;
	int proced = 0;
	int machine = 0;
	int fix_num = srvquest.size();
	TIME begtime = 0;
	TIME lasting = 0;

	/*假设结构体数组为
		struct fix
	{
		int machine_fix;
		int start_time_fix;
		int need_time_fix;
	}repair[fix_number];*/

	//先进行冒泡排序
	SRVQUEST temp;
	for (int k = fix_num - 1; k >= 0; --k) {
		for (int j = 0; j < k; ++j) {
			if (srvquest[j].start > srvquest[j + 1].start) { //升序排序
				temp = srvquest[j];
				srvquest[j] = srvquest[j + 1];
				srvquest[j + 1] = temp;
			}
		}
	}

	for (int i = 0, k = 0; i < chromo_len; ++i) {
		int a = machine_spare.size();
		product = CHR.chromo[i];
		proced = process[product];
		machine = jobtable[product][proced].machine;

		for (k = 0; k < fix_num; ++k) { //遍历检修表，看是否有此机器 
			if (machine == srvquest[k].machine) {
				break;
			}//一旦发现有此机器，就跳出，保留 k 
		}
		if (k < fix_num) {
			if (srvquest[k].start <= machine_spare[machine]) {
				res[machine].resize(res[machine].size() + 1);
				res[machine][called[machine]].isService = 1;//1表示处于维修状态    
				res[machine][called[machine]].start = machine_spare[machine];//开始时间
				res[machine][called[machine]].end = machine_spare[machine] + srvquest[k].duration;
				machine_spare[machine] = machine_spare[machine] + srvquest[k].duration;
				srvquest[k].machine = -1;//检修完后，就把它从检修表里踢出去
				++called[machine];//检修也算一次调用； 
			}
			else if (proced != 0 && srvquest[k].start > machine_spare[machine]
				&& srvquest[k].start <= endtime[product][proced - 1]) {

				res[machine].resize(res[machine].size() + 1);
				res[machine][called[machine]].isService = 1;//1表示处于维修状态    
				res[machine][called[machine]].start = srvquest[k].start;//开始时间
				res[machine][called[machine]].end = srvquest[k].start + srvquest[k].duration;
				machine_spare[machine] = srvquest[k].start + srvquest[k].duration;
				srvquest[k].machine = -1;//检修完后，就把它从检修表里踢出去
				++called[machine];//检修也算一次调用；			
			}
		}
		if (proced == 0) {
			begtime = machine_spare[machine];
			lasting = jobtable[product][proced].duration;
			endtime[product][proced] = begtime + lasting;
			machine_spare[machine] = begtime + lasting;
		}
		else {
			if (machine_spare[machine] >= endtime[product][proced - 1]) {
				begtime = machine_spare[machine];
			}
			else {
				begtime = endtime[product][proced - 1];
			}
			lasting = jobtable[product][proced].duration;
			endtime[product][proced] = begtime + lasting;
			machine_spare[machine] = begtime + lasting;
		}

		//以上为计算时间函数 
		res[machine][called[machine]].isService = 0;//0表示为机器处于工件加工状态 
		res[machine][called[machine]].product = product;
		res[machine][called[machine]].proced = proced;
		res[machine][called[machine]].start = begtime;
		res[machine][called[machine]].end = begtime + lasting;
		++called[machine];
		//用一个二维结构体数组记录调用过程 
		++process[product];
	}

	for (int i = 0; i < fix_num; ++i) {
		if (srvquest[i].machine != -1) {
			machine = srvquest[i].machine;
			res[machine].resize(res[machine].size() + 1);
			if (machine_spare[machine] >= srvquest[i].start) { //第一种情况
				res[machine][called[machine]].isService = true;//表示处于检修状态 
				res[machine][called[machine]].start = machine_spare[machine];
				res[machine][called[machine]].end = machine_spare[machine] + srvquest[i].duration;
				machine_spare[machine] = machine_spare[machine] + srvquest[i].duration;
				++called[machine];
			}
			else { //第二种情况 
				res[machine][called[machine]].isService = true;//表示处于检修状态
				res[machine][called[machine]].start = srvquest[i].start;
				res[machine][called[machine]].end = srvquest[i].start + srvquest[i].duration;
				machine_spare[machine] = srvquest[i].start + srvquest[i].duration;
				++called[machine];
			}
		}
	}

	return res;
} // get_result_table_with_service


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef GUI
CHROMO jobshop_GA(const DVector2D<PROCEDURE>& jobtable)
{
	product_count = jobtable.size();
	proced_count = jobtable[0].size();
	chromo_len = product_count * proced_count;
	machine_count = 0;
	for (int i = 0; i < product_count; ++i) {
		for (int j = 0; j < proced_count; ++j) {
			if (jobtable[i][j].machine > machine_count) {
				machine_count = jobtable[i][j].machine;
			}
		}
	}
	++machine_count;

	DVector<CHROMO>* pop = new DVector<CHROMO>[POP_NUM];
	for (int k = 0; k < POP_NUM; k++) {
		pop[k].reserve(POPULATION_SIZE * 2);
		init_population(pop[k], jobtable);
	}

	int local_best_cout = 0;
	CHROMO best_chormo;
	best_chormo.time = 1 << 22;

	for (int i = 1; i <= ITERATION_COUNT && local_best_cout < ITERATION_COUNT * 1 / 3; ++i) {
		TIME last_best_time = best_chormo.time;
		if (!(i % COMMUNACATION_CYCLE)) communication(pop);
		for (int k = 0; k < POP_NUM; k++) {
			//selfing_multi(jobtable, pop[k]);
			//crossover(jobtable, pop[k]);
			//if(local_best_cout > 5)
			selfing_doubleseg(jobtable, pop[k]);

			mutate_six(jobtable, pop[k]);
			//if(local_best_cout > 5)
			//mutate_all(jobtable, pop[k]);

			filter_by_sort(pop[k]);
			if (i == ITERATION_COUNT) {
				break;
			}
		}
		for (int i = 0; i < POP_NUM; i++) {
			if (pop[i][0].time < best_chormo.time) {
				best_chormo = pop[i][0];
			}
		}

		if (last_best_time == best_chormo.time) local_best_cout++;
		else local_best_cout = 0;
	}

	return best_chormo;
} // CHROMO GA(...)


DVector<int> chrtmp1;
DVector<int> chrtmp2;
void crossover(const DVector2D<PROCEDURE>& jobtable, DVector<CHROMO>& pop)
{
	int parent1 = 0;
	int parent2 = 0;
	long pos1 = 0;
	long pos2 = 0;
	long swaptmp;
	int pop_orisize = pop.size();
	for (int i = 0; i < POPULATION_SIZE * CROSSOVER_RATE / 2; ++i) {
		parent1 = dtl::xorshiftRand() % POPULATION_SIZE;
		parent2 = dtl::xorshiftRand() % POPULATION_SIZE;
		pos1 = dtl::xorshiftRand() % chromo_len;
		pos2 = dtl::xorshiftRand() % chromo_len;
		if (pos1 > pos2) {
			swaptmp = pos1;
			pos1 = pos2;
			pos2 = swaptmp;
		}
		pop.resize(pop.size() + 2);
		pop[pop_orisize].chromo.resize(chromo_len);
		pop[pop_orisize + 1].chromo.resize(chromo_len);
		for (int j = 0; j < pos1; ++j) {
			pop[pop_orisize].chromo[j] = -1;
			pop[pop_orisize + 1].chromo[j] = -1;
		}
		for (int j = pos1; j <= pos2; ++j) {
			pop[pop_orisize].chromo[j] = pop[parent1].chromo[j];
			pop[pop_orisize + 1].chromo[j] = pop[parent2].chromo[j];
		}
		for (int j = pos2 + 1; j < chromo_len; ++j) {
			pop[pop_orisize].chromo[j] = -1;
			pop[pop_orisize + 1].chromo[j] = -1;
		}

		// global temp
		chrtmp1 = pop[parent1].chromo;
		chrtmp2 = pop[parent2].chromo;

		for (int j = pos1; j <= pos2; ++j) {
			for (int k = 0; k < chromo_len; ++k) {
				if (chrtmp2[k] == pop[parent1].chromo[j]) {
					chrtmp2[k] = -1;
					break;
				}
			}
		}
		for (int j = pos1; j <= pos2; ++j) {
			for (int k = 0; k < chromo_len; ++k) {
				if (chrtmp1[k] == pop[parent2].chromo[j]) {
					chrtmp1[k] = -1;
					break;
				}
			}
		}

		for (int j = 0, k = 0; j < chromo_len && k < chromo_len; ) {
			if (pop[pop_orisize].chromo[j] == -1) {
				while (chrtmp2[k] == -1) {
					++k;
				}
				pop[pop_orisize].chromo[j] = chrtmp2[k];
				++k;
			}
			++j;
		}
		for (int j = 0, k = 0; j < chromo_len && k < chromo_len; ) {
			if (pop[pop_orisize + 1].chromo[j] == -1) {
				while (chrtmp1[k] == -1) {
					++k;
				}
				pop[pop_orisize + 1].chromo[j] = chrtmp1[k];
				++k;
			}
			++j;
		}
		pop[pop_orisize].time = chromo_time(jobtable, pop[pop_orisize].chromo, 1);
		pop[pop_orisize + 1].time = chromo_time(jobtable, pop[pop_orisize + 1].chromo, 1);
		pop_orisize += 2;
	}
} // void crossover(...)


void selfing(const DVector2D<PROCEDURE>& jobtable, DVector<CHROMO>& pop)
{
	for (int i = 0, pos, rndchr; i < POPULATION_SIZE * CROSSOVER_RATE; ++i) {
		rndchr = dtl::xorshiftRand() % POPULATION_SIZE;
		pos = dtl::xorshiftRand() % (chromo_len - 2) + 1;
		pop.resize(POPULATION_SIZE + i + 1);
		pop[POPULATION_SIZE + i].chromo.resize(chromo_len);
		for (int j = pos; j < chromo_len; ++j) {
			pop[POPULATION_SIZE + i].chromo[j - pos] = pop[rndchr].chromo[j];
		}
		for (int j = 0; j < pos; ++j) {
			pop[POPULATION_SIZE + i].chromo[chromo_len - pos + j] = pop[rndchr].chromo[j];
		}
		pop[POPULATION_SIZE + i].time = chromo_time(jobtable, pop[POPULATION_SIZE + i].chromo, 0);
	}
} // void selfing(...)


void selfing_multi(const DVector2D<PROCEDURE>& jobtable, DVector<CHROMO>& pop)
{
	int parent;
	int pos1;
	int pos2;
	int temp;
	for (int i = 0; i < POPULATION_SIZE * CROSSOVER_RATE; ++i) {
		parent = dtl::xorshiftRand() % POPULATION_SIZE;
		pos1 = dtl::xorshiftRand() % (chromo_len - 2) + 1;
		pos2 = dtl::xorshiftRand() % (chromo_len - 2) + 1;
		if (pos1 > pos2) {
			temp = pos1;
			pos1 = pos2;
			pos2 = temp;
		}
		pop.resize(POPULATION_SIZE + i + 1);
		pop[POPULATION_SIZE + i].chromo.resize(chromo_len);
		for (int j = pos1; j <= pos2; ++j) {
			int a = pop[parent].chromo[j];
			pop[POPULATION_SIZE + i].chromo[j - pos1] = a;
		}
		for (int j = 0; j < pos1; ++j) {
			pop[POPULATION_SIZE + i].chromo[j + 1 + pos2 - pos1] = pop[parent].chromo[j];
		}
		for (int j = pos2 + 1; j < chromo_len; ++j) {
			pop[POPULATION_SIZE + i].chromo[j] = pop[parent].chromo[j];
		}
		pop[POPULATION_SIZE + i].time = chromo_time(jobtable, pop[POPULATION_SIZE + i].chromo, 1);
	}
} // void selfing_multi(...)


void selfing_multi_pickbysort(const DVector2D<PROCEDURE>& jobtable, DVector<CHROMO>& pop)
{
	int parent;
	int pos1;
	int pos2;
	int temp;
	for (int i = 0; i < POPULATION_SIZE * CROSSOVER_RATE; ++i) {

		if (static_cast<double>(dtl::xorshiftRand()) / dtl::xorshiftRANDMAX < 0.7) {
			parent = dtl::xorshiftRand() % (POPULATION_SIZE / 3);
		}
		else {
			parent = dtl::xorshiftRand() % (POPULATION_SIZE * 2 / 3) + POPULATION_SIZE / 3 - 10;
		}


		pos1 = dtl::xorshiftRand() % (chromo_len - 2) + 1;
		pos2 = dtl::xorshiftRand() % (chromo_len - 2) + 1;
		if (pos1 > pos2) {
			temp = pos1;
			pos1 = pos2;
			pos2 = temp;
		}
		pop.resize(POPULATION_SIZE + i + 1);
		pop[POPULATION_SIZE + i].chromo.resize(chromo_len);
		for (int j = pos1; j <= pos2; ++j) {
			int a = pop[parent].chromo[j];
			pop[POPULATION_SIZE + i].chromo[j - pos1] = a;
		}
		for (int j = 0; j < pos1; ++j) {
			pop[POPULATION_SIZE + i].chromo[j + 1 + pos2 - pos1] = pop[parent].chromo[j];
		}
		for (int j = pos2 + 1; j < chromo_len; ++j) {
			pop[POPULATION_SIZE + i].chromo[j] = pop[parent].chromo[j];
		}
		pop[POPULATION_SIZE + i].time = chromo_time(jobtable, pop[POPULATION_SIZE + i].chromo, 1);
	}
} // void selfing_multi(...)


void filter(DVector<CHROMO>& pop)
{
	static int count = 1;
	double average_time = 0;
	int pop_size = pop.size();
	for (int i = 0; i < pop_size; ++i) {
		average_time += pop[i].time;
	}
	average_time /= pop_size;
	if (count % 3 != 0) {
		for (int i = pop_size - 1; 0 <= i && POPULATION_SIZE < pop_size; --i) {
			if (pop[i].time > average_time) {
				if (i == pop.size() - 1) {
					pop.pop_back();
				}
				else {
					pop[i] = pop.back();
					pop.pop_back();
				}
				--pop_size;
			}
		}
	}
	else {// reverse
		for (int i = 0; i < POPULATION_SIZE && POPULATION_SIZE < pop_size; ++i) {
			if (pop[i].time > average_time) {
				pop[i] = pop.back();
				pop.pop_back();
				--pop_size;
			}
		}
	}
	if (pop.size() > POPULATION_SIZE) {
		pop.erase(POPULATION_SIZE, pop.size());
	}
	++count;
} // void filter(...)


DVector<double> accumulate_rate;
DVector<bool> marked;
void filter_roulette(DVector<CHROMO>& pop)
{
	accumulate_rate.resize(POPULATION_SIZE * 2 + 1);
	marked.resize(POPULATION_SIZE * 2 + 1);

	double sumtime = 0;
	int popsize = pop.size();
	for (int i = 0; i < popsize; ++i) {
		marked[i] = false;
	}

	for (int i = 0; i < popsize; ++i) {
		sumtime += pop[i].time;
	}
	accumulate_rate[0] = 0;
	for (int i = 1; i < popsize + 1; ++i) {
		accumulate_rate[i] = accumulate_rate[i - 1] + pop[i - 1].time / sumtime;
	}
	for (int i = popsize - POPULATION_SIZE, j; i > 0; ) {
		double rnd = static_cast<double>(dtl::xorshiftRand()) / dtl::xorshiftRANDMAX;
		for (j = 1; accumulate_rate[j - 1] <= rnd; ++j);
		if (!marked[j]) {
			marked[j] = true;
			--i;
		}
	}
	for (int i = popsize - 1; i >= 0; --i) {
		if (marked[i]) {
			if (i == pop.size() - 1) {
				pop.pop_back();
			}
			else {
				pop[i] = pop.back();
				pop.pop_back();
			}
		}
	}
} // void filter_roulette(...)


DVector<bool> marked_tor;
void filter_tournament(DVector<CHROMO>& pop)
{
	marked_tor.resize(POPULATION_SIZE * 2);
	int popsize = pop.size();
	int s1;
	int s2;
	int s3;
	int min_num;
	for (int i = 0; i < popsize; ++i) {
		marked_tor[i] = false;
	}
	for (int i = 0; i < POPULATION_SIZE; ) {
		s1 = dtl::xorshiftRand() % popsize;
		s2 = dtl::xorshiftRand() % popsize;
		s3 = dtl::xorshiftRand() % popsize;
		if (pop[s1].time <= pop[s2].time) {
			if (pop[s1].time <= pop[s3].time) {
				min_num = s1;
			}
			else {
				min_num = s3;
			}
		}
		else {
			if (pop[s2].time <= pop[s3].time) {
				min_num = s2;
			}
			else {
				min_num = s3;
			}
		}
		if (!marked_tor[min_num]) {
			marked_tor[min_num] = true;
			++i;
		}
	}
	for (int i = popsize - 1; i >= 0; --i) {
		if (!marked[i]) {
			if (i == pop.size() - 1) {
				pop.pop_back();
			}
			else {
				pop[i] = pop.back();
				pop.pop_back();
			}
		}
	}
} // void filter_tournament(DVector<CHROMO>& pop)


void mutate(DVector<CHROMO>& pop)
{
	for (int i = 0; i < POPULATION_SIZE * MUTATION_RATE_BAD; ++i) {
		int rndchr = dtl::xorshiftRand() % POPULATION_SIZE;
		int pos1 = dtl::xorshiftRand() % chromo_len;
		int pos2 = dtl::xorshiftRand() % chromo_len;
		pop[rndchr].chromo.swap(pos1, pos2);
	}
} // void mutate(...)


void mutate_all(const DVector2D<PROCEDURE>& jobtable, DVector<CHROMO>& pop)
{
	DVector<int> c1;
	DVector<int> c2;
	int rndchr;
	int pos1;
	int pos2;
	TIME t1;
	TIME t2;
	for (int i = 0; i < POPULATION_SIZE; ++i) {
		rndchr = dtl::xorshiftRand() % POPULATION_SIZE;
		pos1 = dtl::xorshiftRand() % chromo_len;
		pos2 = dtl::xorshiftRand() % chromo_len;
		c1 = pop[rndchr].chromo;
		c1.swap(pos1, pos2);
		pos1 = dtl::xorshiftRand() % chromo_len;
		pos2 = dtl::xorshiftRand() % chromo_len;
		c2 = pop[rndchr].chromo;
		c2.swap(pos1, pos2);
		t1 = chromo_time(jobtable, c1, 1);
		t2 = chromo_time(jobtable, c2, 1);
		if (t1 < t2) {
			if (t1 < pop[rndchr].time) {
				pop[rndchr] = { c1, t1 };
			}
		}
		else if (t2 < t1) {
			if (t2 < pop[rndchr].time) {
				pop[rndchr] = { c2, t2 };
			}
		}
	}
} // void mutate_all(...)

#endif //GUI
