//yarin danon
//ID:305413122
#include "mem_sim.h"
//global array that hold the Which frame are free
int memory_mange[MEMORY_SIZE/PAGE_SIZE];
//hold the next frame "index" that need to replace 
int in = 0;
//method that manage the the global "in" 
void num_frame()
{
	in++;
	if(in%(MEMORY_SIZE/PAGE_SIZE) == 0)
		in = 0;
}

//A queue of pages
#define MAX MEMORY_SIZE/PAGE_SIZE

int intArray[MAX];
int front = 0;
int rear = -1;
int itemCount = 0;

//return the page that next in the queue
int peek() 
{
   return intArray[front];
}
//return if the queue is empty or not
bool isEmpty() 
{
   return itemCount == 0;
}
//return if the queue is full or not
bool isFull() 
{
   return itemCount == MAX;
}
//return who many in the queue
int size() 
{
   return itemCount;
}  
//insert to the queue
void insert(int data) 
{

   if(!isFull()) 
   {
	
      if(rear == MAX-1) 
	  {
         rear = -1;            
      }       

      intArray[++rear] = data;
      itemCount++;
   }
}
//remove from the queue
int removeData() 
{
   int data = intArray[front++];
	
   if(front == MAX) 
   {
      front = 0;
   }
	
   itemCount--;
   return data;  
}

//initialization of a new page from the heap
void init_new_page(struct sim_database * mem_sim,int address,int offset)
{
	//checks if the memory is full and return the free index 
	int found = -1;
	for(int i = 0 ;i < MEMORY_SIZE/PAGE_SIZE ; i++)
	{
		if(memory_mange[i] == 0)
		{
			found = i;
			memory_mange[i] = 1;
			break;
		}	
	}
	//case thst found a free frame in the memory
	if(found != -1)
	{
		for(int i = 0 ; i < PAGE_SIZE ; i++)
		{
			mem_sim->main_memory[found*PAGE_SIZE+i] = '0';
		}
		mem_sim->page_table[address/PAGE_SIZE].V = 1;
		mem_sim->page_table[address/PAGE_SIZE].frame = found;
		insert(address/PAGE_SIZE);
	}
	//case that the memory is full
	else
	{
		//checks who is the next page in the queue
		int q = peek();
		//check if the page dirty and write to the swap file if necessary 
		if(mem_sim->page_table[q].D == 1)
		{

			char* str = (char*)malloc(sizeof(char)*PAGE_SIZE);
			for(int i = 0 ;i < PAGE_SIZE ; i++)
			{
				strncpy(&str[i],&mem_sim->main_memory[mem_sim->page_table[q].frame*PAGE_SIZE+i],1);
			}
			lseek(mem_sim->swapfile_fd, q*PAGE_SIZE, SEEK_SET);
			write(mem_sim->swapfile_fd,str,PAGE_SIZE);
			free(str);
		}
		//update
		mem_sim->page_table[q].frame = -1;
		mem_sim->page_table[q].V = 0;
		char buff [PAGE_SIZE];
		//initializs zero in the main memory
		for(int i = 0 ; i < PAGE_SIZE ; i++)
		{		
			mem_sim->main_memory[in*PAGE_SIZE+i] = '0';
		}
		//updates
		mem_sim->page_table[address/PAGE_SIZE].V = 1;
		mem_sim->page_table[address/PAGE_SIZE].frame = in;
		num_frame();
		removeData();
		insert(address/PAGE_SIZE);	
	}
}
//read from the exe or swap
void read_exe(struct sim_database * mem_sim,int address,int offset)
{ 
	//checks if the memory is full and return the free index 
	int page = address/PAGE_SIZE;
	int found = -1;
	for(int i = 0 ;i < MEMORY_SIZE/PAGE_SIZE ; i++)
	{
		if(memory_mange[i] == 0)
		{
			found = i;
			memory_mange[i] = 1;
			break;
		}	
	}
	//case thst found a free frame in the memory
	if(found != -1)
	{
		char buff [PAGE_SIZE];
		//read from exe file
		if(mem_sim->page_table[page].D == 0)
		{
			lseek(mem_sim->program_fd, address-offset, SEEK_SET);
			read(mem_sim->program_fd,&buff,sizeof(buff));
		}
		//read fom swap file
		else
		{
			lseek(mem_sim->swapfile_fd, address-offset, SEEK_SET);
			read(mem_sim->swapfile_fd,&buff,sizeof(buff));
		}
		//update the main memory
		for(int i = 0 ; i < PAGE_SIZE ; i++)
		{
			mem_sim->main_memory[found*PAGE_SIZE+i] = buff[i];
		}
		//updates
		mem_sim->page_table[page].V = 1;
		mem_sim->page_table[page].frame = found;
		insert(page);
	}
	//case that the memory is full
	else
	{
		//checks who is the next page in the queue
		int q = peek();
		//check if the page dirty and write to the swap file if necessary 
		if(mem_sim->page_table[q].D == 1)
		{
			char* str = (char*)malloc(sizeof(char)*PAGE_SIZE);
			for(int i = 0 ;i < PAGE_SIZE ; i++)
			{
				strncpy(&str[i],&mem_sim->main_memory[mem_sim->page_table[q].frame*PAGE_SIZE+i],1);
			}
			lseek(mem_sim->swapfile_fd, q*PAGE_SIZE, SEEK_SET);
			write(mem_sim->swapfile_fd,str,PAGE_SIZE);
			free(str);
		}
		//updates
		mem_sim->page_table[q].frame = -1;
		mem_sim->page_table[q].V = 0;
		char buff [PAGE_SIZE];
		//reade from exe file
		if(mem_sim->page_table[page].D == 0)
		{
			lseek(mem_sim->program_fd, address-offset, SEEK_SET);
			read(mem_sim->program_fd,&buff,sizeof(buff));
		}
		//read from swap file
		else
		{
			lseek(mem_sim->swapfile_fd, address-offset, SEEK_SET);
			read(mem_sim->swapfile_fd,&buff,sizeof(buff));
		}
		//update the main memory
		for(int i = 0 ; i < PAGE_SIZE ; i++)
		{		
			strncpy(&mem_sim->main_memory[in*PAGE_SIZE+i],&buff[i],1);
		}
		//updates
		mem_sim->page_table[page].V = 1;
		mem_sim->page_table[page].frame = in;
		num_frame();
		removeData();
		insert(page);	
	}
	
}
//initialization
struct sim_database* init_system(char exe_file_name[], char swap_file_name[] , int text_size, int data_bss_size, int heap_stack_size )
{	
	//open exe file
	int swap ,exe;
	int page_text , page_bss ,page_heap;
	exe = open(exe_file_name,0600);
	if(exe < 0)
	{
		printf("ERR in exe\n");
		exit(1);
	}
	//open swap file
	swap = open(swap_file_name , O_RDWR |O_CREAT|O_TRUNC,0600);
	if(swap < 0)
	{
		printf("ERR in swap\n");
		exit(1);
	}
	//update the struct
	struct sim_database * sim_db;
	sim_db = (struct sim_database*)malloc(sizeof(struct sim_database));
	sim_db->program_fd = exe;
	sim_db->swapfile_fd = swap;
	sim_db->text_size = text_size;
	sim_db->data_bss_size = data_bss_size;
	sim_db->heap_stack_size = heap_stack_size;
	
	
	////initializ the main memory with '0';
	for(int i = 0; i < MEMORY_SIZE ; i++)
	{
		sim_db->main_memory[i] = '0';
		memory_mange[i] = 0;
	}
	////initializ the page of text,data bss,heap
	page_text = text_size/PAGE_SIZE+1;
	page_bss = data_bss_size/PAGE_SIZE+1;
	page_heap = heap_stack_size/PAGE_SIZE+1;
	
	if((text_size%PAGE_SIZE) == 0)
		page_text--;
	if(data_bss_size%PAGE_SIZE == 0)
		page_bss--;
	if(heap_stack_size%PAGE_SIZE == 0)
		page_heap--;
	
	
	for(int i = 0 ; i < NUM_OF_PAGES ; i++)
	{
		sim_db->page_table[i].V = 0;
		sim_db->page_table[i].D = 0;
		sim_db->page_table[i].frame = -1;
		// for code P = 0
		if(i < page_text)
			sim_db->page_table[i].P = 0;
		//for the rest P = 1
		else if(i < page_bss+page_heap+page_text)
			sim_db->page_table[i].P = 1;
	}
	return sim_db;
}
//load page to the memory
char load (struct sim_database * mem_sim , int address)
{
	if(address >= (mem_sim->text_size+mem_sim->data_bss_size+mem_sim->heap_stack_size))
		return '\0';
	int page_addr;
	int page = address/PAGE_SIZE;
	int offset = address%PAGE_SIZE;
	//if valid return the value
	if(mem_sim->page_table[page].V == 1)
	{
		page_addr = mem_sim->page_table[page].frame*PAGE_SIZE+offset;
		return mem_sim->main_memory[page_addr];
	}
	//if not valid and Permission = 0
	else if(mem_sim->page_table[page].P == 0)
	{
		read_exe(mem_sim,address,offset);			
	}
	//if not valid and Permission = 1	
	else
	{
		int left = mem_sim->text_size;
		int right = mem_sim->data_bss_size+ mem_sim->text_size;
		//if dirty = 0 and Permission = 1
		if(mem_sim->page_table[page].D == 0)
		{
			//check if the address is for data bss
			if( (address >= left )&&(address < right))
			{
				read_exe(mem_sim,address,offset);
			}
			//case that address is for heap
			else
			{
				mem_sim->page_table[page].D = 1;
				init_new_page(mem_sim, address,offset);
			}
		}
		//dirty = 1 and Permission = 1
		else
		{
			read_exe(mem_sim,address,offset);
		}			
	}
	page_addr = mem_sim->page_table[page].frame*PAGE_SIZE+offset;
	return mem_sim->main_memory[page_addr];
	
}
//store char in the memory
void store(struct sim_database * mem_sim , int address, char value)
{
	if(address < mem_sim->text_size)
		return;
	
	if(address >= (mem_sim->text_size+mem_sim->data_bss_size+mem_sim->heap_stack_size))
		return;
	int page_addr;
	int page = address/PAGE_SIZE;
	int offset = address%PAGE_SIZE;
	
	//if valid write the value
	if(mem_sim->page_table[page].V == 1)
	{
		page_addr = mem_sim->page_table[page].frame*PAGE_SIZE+offset;
		mem_sim->main_memory[page_addr] = value;
		mem_sim->page_table[page].D = 1;
		return;
	}
	
	else if(mem_sim->page_table[page].P == 0)
	{
		read_exe(mem_sim,address,offset);			
	}
		
	else
	{
		int left = mem_sim->text_size;
		int right = mem_sim->data_bss_size+ mem_sim->text_size;
		if(mem_sim->page_table[page].D == 0)
		{
			if( (address >= left )&&(address < right))
			{
				read_exe(mem_sim,address,offset);
			}
			else
			{
				mem_sim->page_table[page].D = 1;
				init_new_page(mem_sim, address,offset);
			}
		}	
		else
		{
			read_exe(mem_sim,address,offset);
		}			
	}
	//write the value
	page_addr = mem_sim->page_table[page].frame*PAGE_SIZE+offset;
	mem_sim->main_memory[page_addr] = value;
	mem_sim->page_table[page].D = 1;
}

void clear_system(struct sim_database * mem_sim)
{
	close(mem_sim->program_fd);
	close(mem_sim->swapfile_fd);
	free(mem_sim);
}

/**************************************************************************************/
void print_memory(struct sim_database* mem_sim) {
int i;
printf("\n Physical memory\n");
for(i = 0; i < MEMORY_SIZE; i++) {
printf("[%c]\n", mem_sim->main_memory[i]);
}
}
/************************************************************************************/
void print_swap(struct sim_database* mem_sim) {
char str[PAGE_SIZE];
int i;
printf("\n Swap memory\n");
lseek(mem_sim->swapfile_fd, 0, SEEK_SET); // go to the start of the file
while(read(mem_sim->swapfile_fd, str, PAGE_SIZE) == PAGE_SIZE) {
for(i = 0; i < PAGE_SIZE; i++) {
printf("[%c]\t", str[i]);
}
printf("\n");
}
}
/***************************************************************************************/
void print_page_table(struct sim_database* mem_sim) {
int i;
printf("\n page table \n");
printf("Valid\t Dirty\t Permission \t Frame\n");
for(i = 0; i < NUM_OF_PAGES; i++) {
printf("[%d]\t[%d]\t[%d]\t[%d]\n", mem_sim->page_table[i].V,
mem_sim->page_table[i].D,
mem_sim->page_table[i].P, mem_sim->page_table[i].frame);
}
}
