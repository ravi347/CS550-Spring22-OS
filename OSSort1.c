// Parallel Bubble Sort using Processes
// Author: Ravi Teja Tadiboina

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <string.h>

#define MAX_COUNT 1000 // Max integers to sort
#define MAX_NUM 1000   // Generate numbers between 0 and MAX_NUM
#define MAX_P 5	   // Max Processes

int N; // Number of integers to sort
int P; // Number of worker Processes

typedef struct Shm
{

	int array[MAX_COUNT];
	int pass;
	int child_done[MAX_P];
	int swap_count[MAX_P];
	int processids[MAX_P];
	int exit_now;

} Shm;

Shm *shm;

void generate_numbers()
{
	int i;

	srandom(time(NULL));

	for (i = 0; i < N; i++)
	{
		shm->array[i] = random() % MAX_NUM;
	}
}

void print_numbers()
{
	int i;

	for (i = 0; i < N; i++)
	{
		printf("%d ", shm->array[i]);
	}
	printf("\n");
}

int compare_and_swap(int i, int j)
{
#ifdef DEBUG
	fprintf(stderr, "i %d j %d\n", i, j);
#endif

	if (shm->array[i] > shm->array[j])
	{
		long temp = shm->array[i];
		shm->array[i] = shm->array[j];
		shm->array[j] = temp;
		return 1;
	}

	return 0;
}

int bubble(int start, int n, int id, int pass)
{
#ifdef DEBUG
	fprintf(stderr, "start %d n %d worker %d pass %d\n", start, n, id, pass);
#endif
	int swap_count;
	if (start + n > N)
	{
		n = N - start;
	}
	while (shm->child_done[id] != 1)
	{
		swap_count = 0;
		int next = start;

		assert(start < N - 1);

		if (pass)
		{
			if (!(next % 2))
				next = next + 1;
		}
		else
		{
			if (next % 2)
				next = next + 1;
		}

		while ((next + 1) < (start + n))
		{
			swap_count += compare_and_swap(next, next + 1);
			next += 2;
		}
		shm->swap_count[id] = swap_count;
		shm->child_done[id] = 1;		
	}
	return swap_count;	
}

void bubble_sort(int start, int n, int id, int pass)
{
	int last_count, swap_count = N;

 #ifdef DEBUG
	print_numbers();
 #endif

	do
	{
		last_count = swap_count;
		swap_count = bubble(start, n, id, pass);
#ifdef DEBUG
		print_numbers();
		// fprintf(stderr, "last_count %d swap_count %d\n", last_count, swap_count);
#endif

	} while (shm->child_done[id] != 1);
}

void set_count(int P, int V)
{
	for (int i = 0; i < P; i++)
	{
		shm->swap_count[i] = V;
	}
}

int main(int argc, char *argv[])
{
	int keep_sorting = 0;
	int status;
	int pid;
	int start, end, temp_end;
	struct timeval startPoint, endPoint;
	key_t key;
	int shmid;
	int mode;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s N\n", argv[0]);
		return 1;
	}

	N = strtol(argv[1], (char **)NULL, 10);
	P = strtol(argv[2], (char **)NULL, 10);

	if ((N < 2) || (N > MAX_COUNT))
	{
		fprintf(stderr, "Invalid N. N should be between 2 and %d.\n", MAX_COUNT);
		return 2;
	}

	if((P > 5) || (P < 2)){
	    fprintf(stderr, "Do not include processes more than 5\n");
	 	exit(0);
	 }

	/*if ((key = ftok("test_shm", 'X')) < 0)
	{
		perror("ftok");
		exit(1);
	}*/

	if ((shmid = shmget(1111, (1 * (sizeof(Shm))), 0644 | IPC_CREAT)) < 0)
	{
		perror("shmget");
		exit(1);
	}

	shm = (Shm *)shmat(shmid, (void *)0, 0);
	if (shm == (Shm *)(-1))
	{
		perror("shmat");
		exit(1);
	}

	generate_numbers();
	print_numbers();
	
	for (int i = 0; i < P; i++)
	{
		shm->child_done[i] = 0;
	}
	set_count(P, 0);

	int range = N / P;
	shm->exit_now = 0;
	shm->pass = 0;
	gettimeofday(&startPoint, NULL);

	while (shm->exit_now != 1)
	{
		keep_sorting += 1;

		for (int i = 0; i < P; i++)
		{
			pid = fork();

			if (pid < 0)
			{
				perror("fork failed:");
				exit(1);
			}

			if (pid == 0)
			{
				int start = i * range;
				int n = range + 1;
				bubble_sort(start, n, i, shm->pass);
				exit(0);
			}

			if (pid > 0)
			{
				shm->processids[i] = pid;
				//summation of child_done
				int res1 = 0;
				for (int i = 0; i < P; i++){
					res1 += shm->child_done[i];
				}

				if (res1 == P)
				{	
					//set the child_done array to 0
					for (int i = 0; i < P; i++) {
						shm->child_done[i] = 0;
					}
					shm->pass = 1 - shm->pass;
				}

				//summation of swap_count
				int res2 = 0;
				for (int i = 0; i < P; i++){
					res2 += shm->swap_count[i];
				}

				if ((keep_sorting % P) == 0 && res2 == 0)
				{
						shm->exit_now = 1;
				}
			}
		}
	}

	for (int i = 0; i < P; i++)
	{
		wait(NULL);
		waitpid(shm->processids[i], &status, 0);
	}
	for (int i = 0; i < P; i++)
	{
		printf("stored pids %d: %d\n", i, shm->processids[i]);
	}
	gettimeofday(&endPoint, NULL);
	print_numbers();
	//Calculate and display sorting task time
    double sortingOperationTime = (double)(endPoint.tv_sec) + (double)(endPoint.tv_usec)
    / 1000000.0 - (double)(startPoint.tv_sec)
    - (double)(startPoint.tv_usec) / 1000000.0;
    printf("Get time of the day Elapsed: %f seconds\n", (double)sortingOperationTime);
    
	fprintf(stderr, "Done.\n");

	//detach from shared memory
    shmdt(shm->array);

    // Destroy shared memory
    shmctl(shmid, IPC_RMID, NULL);

	return 0;
}