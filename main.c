#include<stdlib.h>
#include<pthread.h>
#include<semaphore.h>
#define BUFFER_SIZE 5;
#define EUSE 1  /* Error: incorrect usage */

/*GLobal Variable*/

typedef int buffer_item;
buffer_item buffer[BUFFER_SIZE];
int currentSize;
unsigned int seed;
pthread_mutex_t mutex;

sem_t empty,full;


/*buffer*/
public int insert_item(buffer_item item){
	if(currentSize>=0 && currentSize<BUFFER_SIZE){
		buffer[currentSize]=item;
		currentSize++;
		printf("producer produced %d\n", item);
		return 0;
	}else{
		printf("error: insertion failed\n");
		return -1;
	}
}

public int remove_item(buffer_item *item){
	int i;
	if(currentSize<=0){
		printf("error: the buffer is empty\n");
		return -1;
	}else{
		*item=buffer[0];
		for(i=1;i<currentSize;i++){
			buffer[i-1]=buffer[i];		
		}
		currentSize--;
		printf("consumer consumed %d\n", *item);
	}

}

/*Producer*/
public void *producer(void *param){
	buffer_item rand;
	
	while(1){
		sleep(rand());
		sem_wait(&empty);
		pthread_mutex_lock(&mutex);/*critical section*/
		rand=rand_r(&seed);
		if(insert_item(rand)<0)
			printf("error\n");	
		
		pthread_mutex_unlock(&mutex);
		sem_post(&full);
	}

}
/*Consumer*/
public void *consumer(void *param){
	buffer_item rand;
	while(1){
		sleep(rand();
		sem_wait(&full);
		pthread_mutex_lock(&mutex);/*critical section*/
		if(remove_item(&rand)<0)
			printf("error\n");
		
		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
		
	}
}

int main(int argc, char *argv[]){
	int i;
	int sleepTime;
	int num_producer;
	int num_consumer;
	if(argc ==1){
		printf("Usage: ./thread num_threads \n");
		exit(EUSE);	
	}
	/*get sleep time, number of producers, number of consumers from commandline*/
	sleepTime=atoi(argv[0]);
	num_producer=atoi(argv[1]);
	num_consumer=atoi(argv[2]);
	pthread_t tid_p[num_producer];
	pthread_t tid_c[num_consumer];
	
	/*initialize mutex, full empty*/
	sem_init(&empty,0, BUFFER_SIZE);
	sem_init(&full, 0, 0);
	pthread_mutex_init(&mutex, NULL);

	/*create producer threads*/
	/*create comsumer threads*/
	
	
}
