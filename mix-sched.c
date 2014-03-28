/*
 * File: mix-sched.c
 * Author: Domenic Murtari
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2014/03/28
 * Description: A small program to statistically calculate the value of pi
 *              using a specific scheduling policy, and writing bogus data to
 *              a file after every iteration.
 */

/* Include Flags */
#define _GNU_SOURCE

/* System Includes */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sched.h>
#include <math.h>

/* Local Defines */
#define MAXFILENAMELENGTH 80
#define DEFAULT_OUTPUTFILENAMEBASE "mixoutput"
#define DEFAULT_ITERATIONS 1000000
#define DEFAULT_FORKS 10
#define RADIUS (RAND_MAX / 2)

inline double dist(double x0, double y0, double x1, double y1){
  return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

inline double zeroDist(double x, double y){
  return dist(0, 0, x, y);
}

int int main (int argc, char const *argv[]){
  
  int numProcesses;
  struct sched_param param;
  int policy;
  pid_t pid;
  
  int rv;
  int outputFD;
  char outputFilename[MAXFILENAMELENGTH];
  char outputFilenameBase[MAXFILENAMELENGTH];
  
  long i, j;
  double x, y;
  long iterations;  
  double inCircle = 0.0;
  double inSquare = 0.0;
  double pCircle = 0.0;
  double piCalc = 0.0;
  
  /* Set iterations if supplied */
  if(argc < 2){
    iterations = DEFAULT_ITERATIONS;
  }
  else{
    iterations = atol(argv[1]);
    if(iterations < 1){
      fprintf(stderr, "Bad iterations value\n");
      exit(EXIT_FAILURE);
    }
  }
  
  /* Set scheduling policy if supplied */
  if(argc < 3){
    policy = SCHED_OTHER
  }
  else{
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
  
  /* Set number of forks if supplied */
  if(argc < 4){
    numProcesses = DEFAULT_FORKS;
  }
  else{
    numProcesses = atol(argv[3]);
  }
  
  /* Set supplied output filename base or default if not supplied */
  if(argc < 5){
    if(strnlen(DEFAULT_OUTPUTFILENAMEBASE, MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
      fprintf(stderr, "Default output filename base too long\n");
      exit(EXIT_FAILURE);
    }
    strncpy(outputFilenameBase, DEFAULT_OUTPUTFILENAMEBASE, MAXFILENAMELENGTH);
  }
  else{
    if(strnlen(argv[4], MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
      fprintf(stderr, "Output filename base is too long\n");
      exit(EXIT_FAILURE);
    }
    strncpy(outputFilenameBase, argv[4], MAXFILENAMELENGTH);
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
      
      /* Open Output File Descriptor in Write Only mode with standard permissions*/
      rv = snprintf(outputFilename, MAXFILENAMELENGTH, "%s-%d",
        outputFilenameBase, getpid());    
      if(rv > MAXFILENAMELENGTH){
        fprintf(stderr, "Output filename length exceeds limit of %d characters.\n",
        MAXFILENAMELENGTH);
        exit(EXIT_FAILURE);
      }
      else if(rv < 0){
        perror("Failed to generate output filename");
        exit(EXIT_FAILURE);
      }
      if((outputFD = open(outputFilename, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,
         S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0){
        perror("Failed to open output file");
        exit(EXIT_FAILURE);
      }
      
      /* Calculate pi using statistical method across all iterations*/
      for(i=0; i<iterations; i++){
        x = (random() % (RADIUS * 2)) - RADIUS;
        y = (random() % (RADIUS * 2)) - RADIUS;
        if(zeroDist(x,y) < RADIUS){
          inCircle++;
        }
        inSquare++;
        fprintf(outputFD, "This is random rate being written");
      }

      /* Finish calculation */
      pCircle = inCircle/inSquare;
      piCalc = pCircle * 4.0;

      /* Print result */
      fprintf(stdout, "pi = %f\n", piCalc);
      
      exit(EXIT_SUCCESS);
    } else if(pid > 0) {
      printf("Forked child %d\n", pid);
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