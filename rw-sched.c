/*
 * File: rw.c
 * Author: Andy Saylor
 * Modified by: Domenic Murtari
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2012/03/19
 * Modify Date: 2014/03/28
 * Description: A small i/o bound program to copy N bytes from an input
 *              file to an output file. May read the input file multiple
 *              times if N is larger than the size of the input file.
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
#include <errno.h>

/* Local Defines */
#define MAXFILENAMELENGTH 80
#define DEFAULT_INPUTFILENAME "rwinput"
#define DEFAULT_OUTPUTFILENAMEBASE "rwoutput"
#define DEFAULT_BLOCKSIZE 1024
#define DEFAULT_TRANSFERSIZE 1024*100
#define DEFAULT_FORKS 10

int main(int argc, char* argv[]){
  
  int i;
  int rv;
  int policy;
  int numProcesses;
  int inputFD;
  int outputFD;
  char inputFilename[MAXFILENAMELENGTH];
  char outputFilename[MAXFILENAMELENGTH];
  char outputFilenameBase[MAXFILENAMELENGTH];

  ssize_t transfersize = 0;
  ssize_t blocksize = 0; 
  char* transferBuffer = NULL;
  ssize_t buffersize;
  struct sched_param param;
  pid_t pid;

  ssize_t bytesRead = 0;
  ssize_t totalBytesRead = 0;
  int totalReads = 0;
  ssize_t bytesWritten = 0;
  ssize_t totalBytesWritten = 0;
  int totalWrites = 0;
  int inputFileResets = 0;
  
  /* Process program arguments to select run-time parameters */
  /* Set supplied transfer size or default if not supplied */
  if(argc < 2){
    transfersize = DEFAULT_TRANSFERSIZE;
  }
  else{
    transfersize = atol(argv[1]);
    if(transfersize < 1){
      fprintf(stderr, "Bad transfersize value\n");
      exit(EXIT_FAILURE);
    }
  }
  
  /* Set supplied block size or default if not supplied */
  if(argc < 3){
    blocksize = DEFAULT_BLOCKSIZE;
  }
  else{
    blocksize = atol(argv[2]);
    if(blocksize < 1){
      fprintf(stderr, "Bad blocksize value\n");
      exit(EXIT_FAILURE);
    }
  }

  /* Set policy if supplied */
  if(argc < 4){
    policy = SCHED_OTHER;
  }
  else{
    if(!strcmp(argv[3], "SCHED_OTHER")){
      policy = SCHED_OTHER;
    }
    else if(!strcmp(argv[3], "SCHED_FIFO")){
      policy = SCHED_FIFO;
    }
    else if(!strcmp(argv[3], "SCHED_RR")){
      policy = SCHED_RR;
    }
    else{
      fprintf(stderr, "Un-handled scheduling policy\n");
      exit(EXIT_FAILURE);
    }
  }
  
  /* Set number of forks if supplied */
  if(argc < 5){
    numProcesses = DEFAULT_FORKS;
  }
  else{
    numProcesses = atol(argv[4]);
  }

  /* Set supplied input filename or default if not supplied */
  if(argc < 6){
    if(strnlen(DEFAULT_INPUTFILENAME, MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
      fprintf(stderr, "Default input filename too long\n");
      exit(EXIT_FAILURE);
    }
    strncpy(inputFilename, DEFAULT_INPUTFILENAME, MAXFILENAMELENGTH);
  }
  else{
    if(strnlen(argv[5], MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
      fprintf(stderr, "Input filename too long\n");
      exit(EXIT_FAILURE);
    }
    strncpy(inputFilename, argv[3], MAXFILENAMELENGTH);
  }
  
  /* Set supplied output filename base or default if not supplied */
  if(argc < 7){
    if(strnlen(DEFAULT_OUTPUTFILENAMEBASE, MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
      fprintf(stderr, "Default output filename base too long\n");
      exit(EXIT_FAILURE);
    }
    strncpy(outputFilenameBase, DEFAULT_OUTPUTFILENAMEBASE, MAXFILENAMELENGTH);
  }
  else{
    if(strnlen(argv[6], MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
      fprintf(stderr, "Output filename base is too long\n");
      exit(EXIT_FAILURE);
    }
    strncpy(outputFilenameBase, argv[4], MAXFILENAMELENGTH);
  }

  /* Confirm blocksize is multiple of and less than transfersize*/
  if(blocksize > transfersize){
    fprintf(stderr, "blocksize can not exceed transfersize\n");
    exit(EXIT_FAILURE);
  }
  if(transfersize % blocksize){
    fprintf(stderr, "blocksize must be multiple of transfersize\n");
    exit(EXIT_FAILURE);
  }

  /* Set process to max prioty for given scheduler */
  param.sched_priority = sched_get_priority_max(policy);
  
  /* Set new scheduler policy */
  fprintf(stdout, "Current Scheduling Policy: %d\n", sched_getscheduler(0));
  fprintf(stdout, "Setting Scheduling Policy to: %d\n", policy);
  if(sched_setscheduler(0, policy, &param)){
    perror("Error setting scheduler policy");
    exit(EXIT_FAILURE);
  }
  fprintf(stdout, "New Scheduling Policy: %d\n", sched_getscheduler(0));

  for(i = 0; i < numProcesses; i++){
    pid = fork();
    if(pid == 0){
      /* Allocate buffer space */
      buffersize = blocksize;
      if(!(transferBuffer = malloc(buffersize*sizeof(*transferBuffer)))){
        perror("Failed to allocate transfer buffer");
        exit(EXIT_FAILURE);
      }

      /* Open Input File Descriptor in Read Only mode */
      if((inputFD = open(inputFilename, O_RDONLY | O_SYNC)) < 0){
        perror("Failed to open input file");
        exit(EXIT_FAILURE);
      }

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

      /* Read from input file and write to output file*/
      do{
        /* Read transfersize bytes from input file*/
        bytesRead = read(inputFD, transferBuffer, buffersize);
        if(bytesRead < 0){
          perror("Error reading input file");
          exit(EXIT_FAILURE);
        }
        else{
          totalBytesRead += bytesRead;
          totalReads++;
        }

        /* If all bytes were read, write to output file*/
        if(bytesRead == blocksize){
          bytesWritten = write(outputFD, transferBuffer, bytesRead);
          if(bytesWritten < 0){
            perror("Error writing output file");
            exit(EXIT_FAILURE);
          }
          else{
            totalBytesWritten += bytesWritten;
            totalWrites++;
          }
        }
        /* Otherwise assume we have reached the end of the input file and reset */
        else{
          if(lseek(inputFD, 0, SEEK_SET)){
            perror("Error resetting to beginning of file");
            exit(EXIT_FAILURE);
          }
          inputFileResets++;
        }

      }while(totalBytesWritten < transfersize);

      /* Free Buffer */
      free(transferBuffer);

      /* Close Output File Descriptor */
      if(close(outputFD)){
        perror("Failed to close output file");
        exit(EXIT_FAILURE);
      }

      /* Close Input File Descriptor */
      if(close(inputFD)){
        perror("Failed to close input file");
        exit(EXIT_FAILURE);
      }

      exit(EXIT_SUCCESS);
    } else if(pid > 0) {
      printf("Forked child %d \n", pid);
    } else {
      perror("Forking failed: ");
      exit(EXIT_FAILURE);
    }
  }

  for(i = 0; i < numProcesses; i++){
    wait(NULL);  
  }
  
  return EXIT_SUCCESS;
}
