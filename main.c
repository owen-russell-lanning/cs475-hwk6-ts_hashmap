
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "ts_hashmap.h"

int threadTest(void *arg)
{
	// init the random seed
	srand(time(NULL)); // using time as random seed from https://stackoverflow.com/questions/822323/how-to-generate-a-random-int-in-c

	
	ts_hashmap_t *hashMap = (struct ts_hashmap_t*)arg;

	const int TIMES_TO_TEST = 2000;

	
	for (int i = 0; i < TIMES_TO_TEST; i++)
	{
		// generate a random int for key and value
		int key = rand();
		int val = rand();

		//do a put get and delete
		put(hashMap, key, val);
		get(hashMap, key);
		del(hashMap, key);
	}

	


	return 1;
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("Usage: %s <num threads> <hashmap capacity>\n", argv[0]);
		return 1;
	}

	srand(time(NULL));
	int num_threads = atoi(argv[1]);
	int capacity = (unsigned int)atoi(argv[2]);

	ts_hashmap_t *hashMap = initmap(capacity);
	// make thread array
	pthread_t threads[num_threads];

	// start threads
	for (int i = 0; i < num_threads; i++)
	{


		
		int result = pthread_create(&threads[i], NULL, threadTest, hashMap);

	}

	
	// wait for them to finish
	for (int i = 0; i < num_threads; i++)
	{

		pthread_join(threads[i], NULL);
	}

	

	return 0;
}
