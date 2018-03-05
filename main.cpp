/* 
 * File:   main.cpp
 * Author: hwestman
 *
 * Created on August 16, 2013, 1:11 PM
 * 
 * This is a multithreaded client server system 
 * please find documentation in the parent folder
 * 
 */


#include "common.h"


sharedMemory shaMemory;
ThreadPool threadPool;
Queries activeQueries;
outstandingQuery queries[MAXQUERIES];
//int *progress;
int repeated;
int quit;
int testmode;
int released;

/**
 * Forks a process to start the server process, and runs the startclient function
 * on the main thread
 * @param argc
 * @param argv
 * @return 
 */
int main(int argc, char *argv[]) {

	int poolSize;
	if(argc < 3){
		//THIS IS THE DEFAULT FOR UNIX AND THE PARENT PROCESS FOR WINDOWS
		quit = 0;
        if(argc == 1){
			poolSize = INTBITS;
            
        }else if(argc == 2){
            poolSize = atoi(argv[1]);
		}
        
        initiateSignals();
        initiatedSharedVariables(&shaMemory);
		
		#if defined(_WIN32)
		//WINDOWS PARENT CODE
			//FORK TO DO STARTSERVER
			
			winFork(poolSize);
			startClient(shaMemory);
		#else
			int pid;
			pid = fork();
			if(pid == -1){
				perror("Fork error");
	            
			}else if(pid==0){
				//child
				startServer(shaMemory,poolSize);
			}else{
				startClient(shaMemory);
			}
		
		#endif
	}else if(argc == 3){
	//WINDOWS CHILD, STARTSERVER
		quit = 0;
		//GOTTA GET SHAMEMORY FROM PARAM
		startServer(shaMemory,atoi(argv[1]));
		
	}
        
    releaseSharedVariables(&shaMemory);
    return 0;
}

/**
 * Initiates signals for IDE-quit, ctrl-c quit and terminal quit
 * 
 */
void initiateSignals(){
	#if defined(_WIN32)
	#else
			
    signal(SIGINT, eventHandler);
    signal(SIGQUIT, eventHandler);
    signal(SIGKILL, eventHandler);
    signal(SIGTERM, eventHandler);
	#endif
}
/**
 * Sets the global quit flag signaling processes and threads to exit,
 * takes the signal recieved as param in order to perform potential 
 * actions
 * @param sig
 */
void eventHandler (int sig){
    //cout << sig<<endl;
    quit = 1;
    if(!released){
        releaseSharedVariables(&shaMemory);
    }
}
#if defined(_WIN32)
void winFork(int poolSize){

	TCHAR szPath[STRSIZE];
	int r = GetModuleFileNameW( NULL, szPath, STRSIZE );
	
	if(!r)
	{
		cout << "couldnt get filename" << endl;
	}
	char szArgs[STRSIZE];
	int n = sprintf_s(szArgs,"%s %d %s","serverProcess",poolSize,"childFlag");
	
	size_t size = strlen(szArgs) + 1;
	wchar_t* wa = new wchar_t[size];
	mbstowcs(wa,szArgs,size);
    
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if(!CreateProcess(szPath,wa, 
			NULL,
			NULL, 
			FALSE, 
			0,
			NULL,
			NULL,
			&si,
			&pi))
	{
	   cout<< "Creation failed" << endl << GetLastError << endl;
	}
}
#endif