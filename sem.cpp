#include <stdio.h>
#include <iostream>
#include <cstdlib>
#include <cstdio>

#include <semaphore.h>
#include <pthread.h>

const int MAX = 10;
int loops = 5;

int buffer[MAX];
int fill = 0;
int use = 0;

int entry = 0;

sem_t empty;
sem_t full;
sem_t mutex;

void put(int value) {
  buffer[fill] = value;
  fill = (fill + 1) % MAX;
}

void * producer(void * arg) {
  int i;
  // Each thread writes 'loops' elements into the buffer
  for ( i = 0; i < loops; i++ ) {
    sem_wait(&empty);	// decrease number of empty elements
    sem_wait(&mutex);	// lock this put-increment operation from other threads
    put(entry);		// put i into array
    entry++;		// increment global counter
    //printf("producing %i\n", i);
    sem_post(&mutex);	// release lock
    sem_post(&full);	// number of written elements increased
  }
}

int get() {
  int tmp = buffer[use];
  use = (use + 1) % MAX;
  return tmp;
}

void * consumer(void * arg) {
  int i;
  // Each thread reads 'loops' elements from the buffer
  for ( i = 0; i < loops; i++ ) {
    sem_wait(&full); 	// decrease number of full elements
    sem_wait(&mutex);	// lock this get-increment operation from other threads
    int tmp = get();
    printf("%d\n", tmp);
    sem_post(&mutex);	// release lock
    sem_post(&empty); 	// increase number of get-elements
  }
}


int main(int argc, char *argv[]) {
  
  sem_init(&empty, 0, MAX); 	// MAX elements not initalized
  sem_init(&full, 0, 0);	// 0 elements initalized
  sem_init(&mutex, 0, 1);	// 0 elements initalized

  loops = atoi(argv[1]);

  pthread_t pid1, cid1, pid2, cid2;
  if (pthread_create(&pid1, NULL, producer, NULL)) {
    fprintf(stderr, "Error creating thread %i\n", 1);
  }
  if (pthread_create(&pid2, NULL, producer, NULL)) {
    fprintf(stderr, "Error creating thread %i\n", 2);
  }
  if (pthread_create(&cid1, NULL, consumer, NULL)) {
    fprintf(stderr, "Error creating thread %i\n", 3);
  }
  if (pthread_create(&cid2, NULL, consumer, NULL)) {
    fprintf(stderr, "Error creating thread %i\n", 4);
  }
  
  if (pthread_join(pid1, NULL)) {
    fprintf(stderr, "Error joining thread\n");
    return 2;
  }
  if (pthread_join(pid2, NULL)) {
    fprintf(stderr, "Error joining thread\n");
    return 2;
  }
  if (pthread_join(cid1, NULL)) {
    fprintf(stderr, "Error joining thread\n");
    return 2;
  }
  if (pthread_join(cid2, NULL)) {
    fprintf(stderr, "Error joining thread\n");
    return 2;
  }

  return 0;
}