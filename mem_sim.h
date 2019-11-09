//yarin danon
//ID:305413122
#ifndef _MEM_SIM_H
#define _MEM_SIM_H
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pwd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <time.h>
#include <sys/shm.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>

#define PAGE_SIZE 5
#define NUM_OF_PAGES 25
#define MEMORY_SIZE 20



typedef struct page_descriptor
{
unsigned int V; // valid
unsigned int D; // dirty
unsigned int P; // permission
int frame; //the number of a frame if in case it is page-mapped
} page_descriptor;

struct sim_database
{
page_descriptor page_table[NUM_OF_PAGES]; //pointer to page table
int swapfile_fd; //swap file fd
int program_fd; //executable file fd
char main_memory[MEMORY_SIZE];
int data_bss_size;
int text_size;
int heap_stack_size;
};
void clear_system(struct sim_database * mem_sim);
void print_page_table(struct sim_database* mem_sim);
void print_swap(struct sim_database* mem_sim);
void print_memory(struct sim_database* mem_sim);
struct sim_database* init_system(char exe_file_name[], char swap_file_name[] , int text_size, int data_bss_size, int heap_stack_size );
char load (struct sim_database * mem_sim , int address);
void store(struct sim_database * mem_sim , int address, char value);
void read_exe(struct sim_database * mem_sim,int fd,int offset);
void num_frame();
int peek();
bool isEmpty();
bool isFull();
int size();
void insert(int data);
int removeData();
void init_new_page(struct sim_database * mem_sim,int address,int offset);
#endif