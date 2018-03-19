/*Author Xuyang Zhang*/
/*Readme: to complie: gcc -O3 -Wall -g  -o exec_file lab4.c -lpthread */


#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#define MATRIX_A_ROW 1200
#define MATRIX_A_COL 1000
#define MATRIX_B_ROW 1000
#define MATRIX_B_COL 500
#define MATRIX_C_ROW 1200
#define MATRIX_C_COL 500

static double start_time;
static struct timezone zone; 
int C[1200][500];
int A[1200][1000];
int B[1000][500];
int result[1200][500];
double elapse_time = 0;
int thread_num = 0;
int NUM_THREADS;
pthread_t tid;       //Thread ID
pthread_attr_t attr; //Set of thread attributes


void start_timer()
{
  zone.tz_minuteswest = 0;
  zone.tz_dsttime = 0;
  struct timeval time;
  gettimeofday(&time, &zone);
  start_time = (double) time.tv_sec + 1e-6*time.tv_usec;
}

double elapsed_time() {
  struct timeval time;
  gettimeofday(&time, &zone);
  double new_time = (double) time.tv_sec + 1e-6*time.tv_usec;
  return new_time - start_time;
}

/*Initial matrix A anD B*/
void iniMAtrix()
{	int i,j;
	for (i = 0; i < MATRIX_A_ROW; i++)
	{
		for(j = 0; j < MATRIX_A_COL; j++)
		{
			A[i][j] = i + j;
		}
	}

	for (i = 0; i < MATRIX_B_ROW; i++)
	{
		for(j = 0; j < MATRIX_B_COL; j++)
		{
			B[i][j] = i + j;
		}
	}

}

/*compare multi-thread result and one thread multiplication result C*/
int error_check()
{
	int i, j;
	for (i = 0; i < MATRIX_C_ROW; i++)
	{
		for(j = 0; j < MATRIX_C_COL; j++)
		{
			if(C[i][j] != result[i][j])
				return 1;
		}
	}

	return 0;
}

/*used for debug, print elements in C and result*/
int print_error_check()
{
	int i, j;
	for (i = 0; i < MATRIX_C_ROW; i++)
	{
		for(j = 0; j < MATRIX_C_COL; j++)
		{
			printf("C[%d][%d]: %d \n", i,j, C[i][j] );
			printf("R[%d][%d]: %d \n", i,j, result[i][j] );
			
		}
	}

	return 0;
}


void iniZero(int r[][500])
{
	int i,j;
	for (i = 0; i < MATRIX_C_ROW; i++)
	{
		for(j = 0; j < MATRIX_C_COL; j++)
		{
			r[i][j] = 0;
		}
	}

}

/*MAtrix Multiplication by single thread*/
void multiply(  )
{
	int i, j ,k;
	for (i = 0; i < MATRIX_C_ROW; i++)
	{
		for(j = 0; j < MATRIX_C_COL; j++)
		{
			C[i][j] = 0;
			for (k = 0; k < MATRIX_A_COL; k++)
			{
				C[i][j] += A[i][k] * B[k][j];
			}
		}
	}

}

/*multithread multiplication*/
/*the algoritm is to divided Matrix A by row and use small blocks multiply with B*/
void *multiply_thread(void* arg)
{
	int num = thread_num ++;
	int operation = MATRIX_A_ROW / NUM_THREADS;
	int rest = MATRIX_A_ROW % NUM_THREADS;
	int start, end;
	int i, j, k, sum;
	if(thread_num == 0)
	{	/*the first thread does more job*/
		start = operation*num;
		end = (operation*(num + 1)) + rest;
	}
	else
	{	
		start = operation*num + rest;
		end = (operation*(num + 1));
	}

	for(i = start; i < end; i++)
	{
		
		for (j = 0; j < MATRIX_B_COL; j++)
		{
			sum =  0;
			for(k = 0; k < MATRIX_A_COL; k++)
			{
				sum += A[i][k]*B[k][j];
			}
			result[i][j] = sum;
		}
		

	}

	return 0;

}


/*execute multiple threads*/
void thread_exec(int n)
{

	NUM_THREADS = n;
	int i;
	thread_num = 0;
	pthread_t tid[NUM_THREADS]; 
	/* Get the default attributes */
   	pthread_attr_init(&attr);

	start_timer();
	for(i = 0; i < NUM_THREADS; i++)
	{		
		pthread_create(&tid[i],&attr,multiply_thread,NULL);
	}
	/*Join Thread*/
	for(i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(tid[i], NULL);
	}

	elapse_time = elapsed_time();

	
}


int main (int argc, char *argv[])
{
	int error;
	int num_thread;

	/*matrix initialize*/
	iniMAtrix();
	iniZero(result);
	
	/*ont thread*/
	start_timer();
	multiply( );
	elapse_time = elapsed_time();
	printf("Thread %d:	", 1);
	printf("time: %.2f \n", elapse_time);

	/* two threads*/
	num_thread = 2;
	thread_exec(num_thread);
	printf("Thread %d:	", num_thread);
	printf("time: %.2f   	", elapse_time);
	error = error_check();
	if(error ==0 )
	{
		printf("No Error\n");
	}
	else
	{
		printf("Error Found\n");
	}
	
	/* three threads*/
	num_thread = 3;
	thread_exec(num_thread);
	printf("Thread %d:	", num_thread);
	printf("time: %.2f   	", elapse_time);
	error = error_check();
	if(error == 0 )
	{
		printf("No Error\n");
	}
	else
	{
		printf("Error Found\n");
	}
	
	/* four threads*/
	num_thread = 4;
	thread_exec(num_thread);
	printf("Thread %d:	", num_thread);
	printf("time: %.2f   	", elapse_time);
	error = error_check();
	if(error == 0)
	{
		printf("No Error\n");
	}
	else
	{
		printf("Error Found\n");
	}
	
	/* five threads*/
	num_thread = 5;
	thread_exec(num_thread);
	printf("Thread %d:	", num_thread);
	printf("time: %.2f   	", elapse_time);
	error = error_check();
	if(error == 0)
	{
		printf("No Error\n");
	}
	else
	{
		printf("Error Found\n");
	}

	/* six threads*/
	num_thread = 6;
	thread_exec(num_thread);
	printf("Thread %d:	", num_thread);
	printf("time: %.2f   	", elapse_time);
	error = error_check();
	if(error == 0)
	{
		printf("No Error\n");
	}
	else
	{
		printf("Error Found\n");
	}
	
	/* seven threads*/
	num_thread = 7;
	thread_exec(num_thread);
	printf("Thread %d:	", num_thread);
	printf("time: %.2f   	", elapse_time);
	error = error_check();
	if(error == 0)
	{
		printf("No Error\n");
	}
	else
	{
		printf("Error Found\n");
	}
	
	return 0;
}
