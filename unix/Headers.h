/* 
 * File:   Headers.h
 * Author: hwestman
 *
 * Created on October 14, 2013, 10:09 PM
 */

#ifndef HEADERS_H
#define	HEADERS_H

#if defined(_WIN32)
	

    #define UNICODE
	#define _UNICODE
    #include <process.h>
	#include <windows.h>
	#include <conio.h>
#else
    #include <unistd.h>
    #include <sys/ipc.h>
    #include <sys/shm.h>
    #include <sys/sem.h>
    #include <sys/wait.h>
    #include <sys/time.h>
    
    #include <termios.h>
#endif
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <sys/types.h>
#include <signal.h>

const int STRSIZE = 128;
const int ARGLENGTH = 10;
const int MAXFACTORS = 256;
/**
 * the nr 32
 */
const int INTBITS = 32;
const int MAXQUERIES = 10;
const int QUEUESIZE = INTBITS*MAXQUERIES;
const int TOTAL = 100;
//#define MAXQUERIES 10;
#define STDIN_FILENO 0
#define NB_ENABLE 1
#define NB_DISABLE 0

#if defined(_WIN32)
	
#endif

using namespace std;


struct sharedMemoryPtrs{
	int *number;
	char *clientflag;
	int *slot;
	char *serverflag;
	int *progress;
};

struct sharedMemory{
#if defined(_WIN32)
	HANDLE number;
	HANDLE clientflag;
	HANDLE slot;
	HANDLE serverflag;
	HANDLE progress;
#else
	int number;
    int clientflag;
    int slot;
    int serverflag;
    int progress;
#endif
};



struct job{
    int index;
    int number;
    int jobIndex;
    bool testRun;
};

struct outstandingQuery{
    
	#if defined(_WIN32)
		SYSTEMTIME startTime;
	#else
		timeval startTime;
	#endif
    bool running;
    bool testRun;
};

#endif	/* HEADERS_H */

