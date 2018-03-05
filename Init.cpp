#include "common.h"

#if defined(_WIN32)
LPTSTR SMCLIENTFLAG = L"CLIENTFLAG";
LPTSTR SMNUMBER = L"NUMBER";
LPTSTR SMSERVERFLAG = L"SERVERFLAG";
LPTSTR SMSLOT = L"SLOT";
LPTSTR SMPROGRESS = L"PROGRESS";
#endif
/**
 * TODO: Remove all unused share memory, like the shared mutexes
 * Initiates all the shared memory need to communicate between server/client
 * @param sm
 * An allrady defined shared memory structured to be initialised
 */
void initiatedSharedVariables(sharedMemory *sm){

	#if defined(_WIN32)
	sm->clientflag = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(char),SMCLIENTFLAG);
	sm->number = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(int),SMNUMBER);
	sm->slot = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(int)*MAXQUERIES,SMSLOT);
	sm->serverflag = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(char)*MAXQUERIES,SMSERVERFLAG);
	sm->progress = CreateFileMapping(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(int)*MAXQUERIES,SMPROGRESS);
    
	sharedMemoryPtrs smptrs;
	getSharedVariablePtrs(sm,&smptrs);

	memset(smptrs.serverflag, '0', sizeof(char)*MAXQUERIES);
	memset(smptrs.slot, 0, sizeof(int)*MAXQUERIES);
	*smptrs.clientflag = '0';

	#else
    /*
     * NUMBER is the inputnumber used to communicate requests
     */ 
    sm->number = shmget(IPC_PRIVATE, sizeof(int), 0xFFFF);
    if(sm->number<=0){  
        perror("shmget number");
    }
    int *number = (int*) shmat(sm->number, NULL, 0);
    *number = 0;
    
    /*
     * CLIENTFLAG is the flag used to syncronize the inputnumber
     * 
     */
    sm->clientflag = shmget(IPC_PRIVATE, sizeof(char), 0xFFFF);
    if(sm->clientflag<=0){
        perror("shmget clientflag");
    }
    char *clientflag = (char*) shmat(sm->clientflag, NULL, 0);
    *clientflag = '0';
    
    /*
     * SLOT is the array of ints used to write responses from the server
     * 
     */ 
    sm->slot = shmget(IPC_PRIVATE, sizeof(int) * MAXQUERIES, 0xFFFF);
    if(sm->slot<=0){
        perror("shmget clientflag");
    }
    int *slot = (int*) shmat(sm->slot, NULL, 0);
    memset(slot, 0, sizeof(int)*MAXQUERIES);
    
    /*
     * SERVERFLAG is the array of flags to syncronise responses for the client,
     * they correspond to slot
     */ 
    sm->serverflag = shmget(IPC_PRIVATE, sizeof(char) * MAXQUERIES, 0xFFFF);
    if(sm->clientflag<=0){
        perror("shmget clientflag");
    }
    char *serverflag = (char*) shmat(sm->serverflag, NULL, 0);
    memset(serverflag, '0', sizeof(char)*MAXQUERIES);
    
    sm->progress = shmget(IPC_PRIVATE, sizeof(int) * MAXQUERIES, 0xFFFF);
    if(sm->progress<=0){
        perror("shmget clientflag");
    }
#endif
    
}

/**
 * Releasing the shared memory reserverd in initiatesharedvariables, also
 * releasing mutex and conditionvariables with corresponding attributes
 * @param sm
 * 
 */
void releaseSharedVariables(sharedMemory *sm){
    sharedMemoryPtrs smptrs;
	getSharedVariablePtrs(sm,&smptrs);
#if defined(_WIN32)

	UnmapViewOfFile(smptrs.clientflag);
	CloseHandle(sm->clientflag);

	UnmapViewOfFile(smptrs.number);
	CloseHandle(sm->number);

	UnmapViewOfFile(smptrs.progress);
	CloseHandle(sm->progress);

	UnmapViewOfFile(smptrs.serverflag);
	CloseHandle(sm->serverflag);

	UnmapViewOfFile(smptrs.slot);
	CloseHandle(sm->slot);
#else
    if(shmdt(smptrs.number)==-1){
        //perror("shmdt number");
    }
    shmctl(sm->number, IPC_RMID, NULL);
    
    //CLIENTFLAG
    if(shmdt(smptrs.clientflag)==-1){
        //perror("shmdt clientflag");
    }
    shmctl(sm->clientflag, IPC_RMID, NULL);
    
    
    //SLOT
    if(shmdt(smptrs.slot)==-1){
        //perror("shmdt slot");
    }
    shmctl(sm->slot, IPC_RMID, NULL);
    
    //SERVERFLAG
    if(shmdt(smptrs.serverflag)==-1){
        //perror("shmdt serverflag");
    }
    shmctl(sm->serverflag, IPC_RMID, NULL);
    
    //PROGRESS
    if(shmdt(smptrs.progress)==-1){
        //perror("shmdt progress");
    }
    shmctl(sm->progress, IPC_RMID, NULL);
#endif
}

/**
 * initiates the given smptrs structure with pointer to the shared memory
 * of the sm structure
 * @param sm
 * @param smptrs
 */

void getSharedVariablePtrs(sharedMemory *sm,sharedMemoryPtrs *smptrs){
#if defined(_WIN32)
	sm->clientflag = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,SMCLIENTFLAG);
	VOID *cf = MapViewOfFile(sm->clientflag, FILE_MAP_ALL_ACCESS,0,0,0);
	smptrs->clientflag = (char *) cf;

	sm->number = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,SMNUMBER);
	VOID *nr = MapViewOfFile(sm->number, FILE_MAP_ALL_ACCESS,0,0,0);
	smptrs->number = (int *) nr;

	sm->progress = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,SMPROGRESS);
	VOID *pr = MapViewOfFile(sm->progress, FILE_MAP_ALL_ACCESS,0,0,0);
	smptrs->progress = (int *) pr;

	sm->serverflag = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,SMSERVERFLAG);
	VOID *sf = MapViewOfFile(sm->serverflag, FILE_MAP_ALL_ACCESS,0,0,0);
	smptrs->serverflag = (char *) sf;

	sm->slot = OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,SMSLOT);
	VOID *sl = MapViewOfFile(sm->slot, FILE_MAP_ALL_ACCESS,0,0,0);
	smptrs->slot = (int *) sl;

#else
	smptrs->number = (int*) shmat(sm->number, NULL, 0);
    smptrs->clientflag = (char*) shmat(sm->clientflag, NULL, 0);
    smptrs->slot = (int*) shmat(sm->slot, NULL, 0);
    smptrs->serverflag = (char*) shmat(sm->serverflag, NULL, 0);
    smptrs->progress = (int*) shmat(sm->progress, NULL, 0);
#endif
}
