/* 
 * File:   common.h
 * Author: hwestman
 *
 * Created on August 16, 2013, 1:12 PM
 */

#ifndef COMMON_H
#define	COMMON_H

#include "Headers.h"
#include "SemLock.h"


//Structures



//Libraries depending on the above

//Signals
void eventHandler (int sig);
void initiateSignals();

//Factorising operations
//void rotateAndFactorize(int number,int index);
void factoriseNumber(job *theJob);

//Client server
void startClient(struct sharedMemory);
void startServer(struct sharedMemory,int poolSize);


void writeToNumber(int req,int *number,char *clientFlag,int &nrOfOutstandingQueries);
int getInNum(char *input);
void *slave(void *arg);

//Shared variables and semaphores
void initiatedSharedVariables(sharedMemory *sm);
void releaseSharedVariables(sharedMemory *sm);

void getSharedVariablePtrs(sharedMemory *sm,sharedMemoryPtrs *smptrs);



int getKBHitString(char *string,int &strLen);
int getPercent(int t,int b);


#if defined(_WIN32)
	void readResponse(outstandingQuery *queries,int *slot,int &nrOfQueries,char *serverflag,SYSTEMTIME &timer);
	void resetTimer(SYSTEMTIME &timer);
	float TimerStop(SYSTEMTIME &timer);
	void checkTimer(SYSTEMTIME &timer,int *progress);
#else
	void readResponse(outstandingQuery *queries,int *slot,int &nrOfQueries,char *serverflag,timeval &timer);
	void resetTimer(timeval &timer);
	float TimerStop(timeval &timer);
	void checkTimer(timeval &timer,int *progress);
#endif

void printProgress(int *progress);
void printBar(int percentile);

#if defined(_WIN32)
void winFork(int poolSize);
#else
//Nonblocking IO
void nonblock(int state);
int kbhit();
#endif
/*
 * GLOBAL NONO VARIABLES
 */

//TODO: Refactor out as parameters 
extern sharedMemory shaMemory;

//TODO: Refactor out to the scope of the server
extern ThreadPool threadPool;
extern Queries activeQueries;

extern outstandingQuery queries[MAXQUERIES];
//extern int *progress;
extern int repeated;
extern int quit;
extern int testmode;
extern int released;

#endif	/* COMMON_H */

