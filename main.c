//yarin danon
//ID:305413122

#include "mem_sim.h"

#define CODE 25
#define DATA 50
#define HEAP_STACK 50


int main(int argc,char argv[])
{
	char val;
	struct sim_database * mem_sim = init_system("exec_file", "swap_file" ,CODE, DATA, HEAP_STACK);
	val = load (mem_sim , 64);
	val = load (mem_sim , 66);
	val = load (mem_sim , 2);
	store(mem_sim , 98,'X');
	val = load (mem_sim ,16);
	val = load (mem_sim ,70);
	store(mem_sim ,32,'Y');
	store (mem_sim ,15,'Z');
	val = load (mem_sim ,23);
	print_page_table(mem_sim);
	print_memory(mem_sim); 
	print_swap(mem_sim); 
	clear_system(mem_sim);
}








