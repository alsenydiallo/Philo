#define _SVID_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "random.h"
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <math.h>

#define NBR_PHILO 5
#define EATING_MEAN 9
#define EATING_STD 3
#define THINKING_MEAN 11
#define THINKING_STD 7
#define MAX_EATING_TIME 100

int semID;

void eating(const int philo, const int eatingTime, const int totalEatingTime){
    printf("Philo %d is eating for %d seconds. [Time spent eating = %d]\n", philo
    , eatingTime, totalEatingTime);
    sleep(eatingTime);
}

void thinking(const int philo, const int thinkingTime, const int totalThinkingTime){
    printf("Philo %d is thinking for %d seconds.[Time spent thinking =%d]\n", philo, thinkingTime, totalThinkingTime);
    sleep(thinkingTime);
}

// This function simulates the philosophers eating and thinking.
void philosopher(int philo){  
  int eatingTime = 0;
  int totalEatingTime = 0;
  int thinkingTime = 0;
  int totalThinkingTime =0;
  srand(philo); 
  struct sembuf semSignal[2] = {{philo, 1, 0}, {(philo+1)%5, 1, 0}};
  struct sembuf semWait[2] = {{philo, -1, 0}, {(philo+1)%5, -1, 0}};
  
  while(totalEatingTime <= MAX_EATING_TIME){
    // Wait for chopsticks to become available.
    if((semop(semID, semWait, 2)) < 0){
      fprintf(stderr, "semop error: %s\n", strerror(errno));
      exit(0);
    } 

    /******* philosopher i is eating for a random amount of time.*******/
    if((eatingTime = randomGaussian(EATING_MEAN, EATING_STD)) < 0)
    	eatingTime = 0;

	eating(philo, eatingTime, totalEatingTime);
    totalEatingTime += eatingTime;
    
    // done eating, Release the chopsticks.
    if((semop(semID, semSignal, 2)) < 0){
      fprintf(stderr, "semop error: %s\n", strerror(errno));
      exit(0);
    }

    /****** Philo is thinking for a random amount of time. ********/
    if((thinkingTime = randomGaussian(THINKING_MEAN, THINKING_STD)) < 0) 
    	thinkingTime = 0;
	totalThinkingTime += thinkingTime;
    thinking(philo, thinkingTime, totalThinkingTime);
  }
  
  printf("Philo %i has finished eating. [Total time spent eating = %i]\n", philo, totalEatingTime);
  
  exit(0);
}

// Function to initialize semaphore's value to 1.
void makeSemaphore(int size){
  struct sembuf semSignal[1] = {{0, 1, 0}};
  // create semaphores
  if((semID = semget(IPC_PRIVATE, size , S_IRUSR | S_IWUSR)) < 0){
      fprintf(stderr, "semget error: %s\n", strerror(errno));
      exit(0);
  }
  //initialize semaphores
  for(int i = 0; i < size; i++){
    semSignal[0].sem_num = i;
    semop(semID, semSignal, 1);
  }
}

int main(void){
  int i;
  
  // create semaphores.
  makeSemaphore(NBR_PHILO);
  
  // Create the philosophers.
  for(i = 0; i < NBR_PHILO; i++){
    if(!fork())
      philosopher(i);  // Only the child process
  }
  
  // Waits for the children to exit. Only the parent will call this.
  for(i = 0; i < NBR_PHILO; i++)
    wait(NULL);
    
    return 0;
}
