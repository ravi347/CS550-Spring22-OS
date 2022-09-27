// Parallel Bubble Sort using Threads
// Author: Ravi Teja Tadiboina
#define _GNU_SOURCE
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>

#define MAX_COUNT 1000000 //Max integers to sort
#define MAX_NUM 100 // Generate numbers between 0 and MAX_NUM
#define MAX_T 100

long number[MAX_COUNT];
void *bubble_sort(void *arg);

int thread_count;
long no_elts; 
long range;
long *array;
long *thread_index;
int pass;
int exit_now;
int done[MAX_T];
int swap_counter[MAX_T];

void generate_numbers()
{
	int i;

	srandom(time(NULL));

	for(i=0; i<no_elts; i++)
		array[i] = random()%MAX_NUM;
}

void done_array(int thread_count,int value){
	for(int i=0;i<thread_count;i++){
		done[i] = value;
	}
}

void set_counter(int thread_count,int value){
	for(int i=0;i<thread_count;i++){
		swap_counter[i] = value;
	}
}

void print_numbers() 
{
	int i;

	for(i=0; i<no_elts; i++) 
		printf("%ld ", array[i]);
	printf("\n");
}

int compare_and_swap(int i, int j) 
{
#ifdef DEBUG
	fprintf(stderr,"i %d j %d\n", i, j);
#endif
	// assert ( i<N );
	// assert ( j<N );

	if( array[i] > array[j]) {
		long temp =  array[i] ;
		array[i] = array[j] ;
		array[j] = temp;
		return 1;
	}

	return 0;
}

// even-odd pass bubbling from start to start+n
int bubble(int start, int n, int id, int pass) 
{
#ifdef DEBUG
	fprintf(stderr, "start %d n %d pass %d\n", start, n, pass);
#endif

	if(start + n > no_elts){
		n = no_elts - start;
	}
	while (done[id] != 1)
	{
	int swap_count = 0;
	int next = start;

	assert (start < no_elts-1); // bug if we start at the end of array

	if (pass) { // sort odd-even index pairs
		if ( !(next % 2) ) 
			next = next + 1;
	} else  { // sort even-odd index pairs
		if (next % 2) 
			next = next + 1;
	}

	while ( (next+1) < (start+n) ) {
		swap_count += compare_and_swap(next, next+1);
		next+=2;
	}
	swap_counter[id] = swap_count;
	done[id] = 1;
	}
	return 0;
}

int summationswap( int thread_count){
	int res = 0;
	for(int i = 0; i < thread_count; i++){
		res += swap_counter[i];
	}
	return res;
}


int summationdone( int thread_count){
	int res = 0;
	for(int i = 0; i < thread_count; i++){
		res += done[i];
	}
	return res;
}

int main (int argc, char **argv)
{
	char *str_thread_count;
	int r, trunc; 
	pthread_t *tid;
        int sfd;
	//long total_sum = 0;

	thread_count = atoi(argv[1]);
	no_elts = atoi(argv[2]);

	range = no_elts / thread_count;	


	thread_index = (long *) malloc(sizeof(long)*thread_count);
	array = (long *) malloc(sizeof(long)*no_elts);
	tid = (pthread_t*) malloc(sizeof(pthread_t)*thread_count);


	if(tid == NULL)
		perror ("Cannot allocate memory for threads\n");

	generate_numbers();
	print_numbers();
	done_array(thread_count,0);
	set_counter(thread_count,0);

	range = no_elts/thread_count;
	pass = 0;
	int looper = 0;
	exit_now = 0;
	
     while(exit_now != 1){
     		looper += 1;

	     	for (int i = 0; i < thread_count; i++) {
				thread_index[i] = i;

				int start = i * range;
				int end = range + 1;
		
				if ((r = pthread_create (&tid [i], NULL, bubble_sort, (void*)&thread_index[i])) != 0) {

					perror("Cannot create multiple threads\n");
				}

				for (int i = 0; i < thread_count; i++) {
					if ((r = pthread_join (tid [i], NULL)) != 0) {

						//perror("Cannot join multiple threads\n");
					}
				}
			
			}
		
			if(summationdone(thread_count) == thread_count){
				done_array(thread_count,0);
				pass = 1 - pass;
			}

			if((looper%thread_count)==0 && (summationswap(thread_count)== 0)){
					exit_now = 1;
			}
     }  
	
	fprintf(stdout, "Sorted sequence is as follows:\n");
	print_numbers();
	fprintf(stderr, "Done.\n");
         
	return 0;
}

void *bubble_sort(void *args) 
{
	int id = *(int *)args;
	long start = id * range;
	long n = range + 1;

	int last_count, swap_count = no_elts;


#ifdef DEBUG
	print_numbers();
#endif

	do {
		last_count = swap_count;
		swap_count = bubble(start,n, id, pass); // 0 for single-process sorting
#ifdef DEBUG
		print_numbers();
		fprintf(stderr,"last_count %d swap_count %d\n", last_count, swap_count);
#endif
		// pass = 1-pass;
	} while(done[id] != 1);
	return NULL;
}