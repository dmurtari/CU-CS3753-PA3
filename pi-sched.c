/*
 * File: pi-sched.c
 * Author: Andy Sayler
 * Modified by: Domenic Murtari
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2012/03/07
 * Modify Date: 2014/03/19
 * Description:
 * 	This file contains a simple program for statistically calculating pi using a
 *  specific scheduling policy. 
 */

/* Local Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define DEFAULT_ITERATIONS 1000000
#define DEFAULT_FORKS 10
#define RADIUS (RAND_MAX / 2)

inline double dist(double x0, double y0, double x1, double y1){
  return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

inline double zeroDist(double x, double y){
  return dist(0, 0, x, y);
}

int main(int argc, char* argv[]){

  long i;
  int j;
  int numProcesses;
  long iterations;
  struct sched_param param;
  int policy;
  double x, y;
  double inCircle = 0.0;
  double inSquare = 0.0;
  double pCircle = 0.0;
  double piCalc = 0.0;
  pid_t pid;


  /* Process program arguments to select iterations and policy */
  /* Set default iterations if not supplied */
  if(argc < 2){
    iterations = DEFAULT_ITERATIONS;
  }
  
  /* Set default policy if not supplied */
  if(argc < 3){
    policy = SCHED_OTHER;
  }
 
  /* Set default number of forks if not supplied */
  if(argc < 4){
    numProcesses = DEFAULT_FORKS;
  }
     
  /* Set iterations if supplied */
  if(argc > 1){
    iterations = atol(argv[1]);
    if(iterations < 1){
      fprintf(stderr, "Bad iterations value\n");
      exit(EXIT_FAILURE);
    }
  }
  
  /* Set policy if supplied */
  if(argc > 2){
    if(!strcmp(argv[2], "SCHED_OTHER")){
      policy = SCHED_OTHER;
    }
    else if(!strcmp(argv[2], "SCHED_FIFO")){
      policy = SCHED_FIFO;
    }
    else if(!strcmp(argv[2], "SCHED_RR")){
      policy = SCHED_RR;
    }
    else{
      fprintf(stderr, "Un-handled scheduling policy\n");
      exit(EXIT_FAILURE);
    }
  }
  
  /* Set number of processes if supplied */
  if(argc > 3){
    numProcesses = atol(argv[3]);
  }

  /* Set process to max priority for given scheduler */
  param.sched_priority = sched_get_priority_max(policy);
  
  /* Set new scheduler policy */
  fprintf(stdout, "Current Scheduling Policy: %d\n", sched_getscheduler(0));
  fprintf(stdout, "Setting Scheduling Policy to: %d\n", policy);
  if(sched_setscheduler(0, policy, &param)){
    perror("Error setting scheduler policy");
    exit(EXIT_FAILURE);
  }
  fprintf(stdout, "New Scheduling Policy: %d\n", sched_getscheduler(0));

  for(j=0; j<numProcesses; j++){
    pid = fork();
    if(pid == 0){
      /* Calculate pi using statistical methode across all iterations*/
      for(i=0; i<iterations; i++){
        x = (random() % (RADIUS * 2)) - RADIUS;
        y = (random() % (RADIUS * 2)) - RADIUS;
        if(zeroDist(x,y) < RADIUS){
          inCircle++;
        }
        inSquare++;
      }

      /* Finish calculation */
      pCircle = inCircle/inSquare;
      piCalc = pCircle * 4.0;

      /* Print result */
      fprintf(stdout, "pi = %f\n", piCalc);
      exit(EXIT_SUCCESS);
    } else if(pid > 0) {
      printf("Forked child %d \n", pid);
    } else {
      fprintf(stderr, "Forking failed");
      exit(EXIT_FAILURE);
    }
  }

  for(j=0; j<numProcesses; j++){
    wait(NULL);  
  }
  
  return 0;
}
