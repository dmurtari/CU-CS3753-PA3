/*
 * File: mix-sched.c
 * Author: Domenic Murtari
 * Project: CSCI 3753 Programming Assignment 3
 * Create Date: 2014/03/28
 * Description: A small program to statistically calculate the value of pi
 *              using a specific scheduling policy, and writing the intermediate
 *              results to a file.
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
#define DEFAULT_INPUTFILENAME "rwinput"
#define DEFAULT_OUTPUTFILENAMEBASE "rwoutput"
#define DEFAULT_BLOCKSIZE 1024
#define DEFAULT_TRANSFERSIZE 1024*100
#define DEFAULT_ITERATIONS 1000000
#define DEFAULT_FORKS 10
#define RADIUS (RAND_MAX / 2)

inline double dist(double x0, double y0, double x1, double y1){
  return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

inline double zeroDist(double x, double y){
  return dist(0, 0, x, y);
}
