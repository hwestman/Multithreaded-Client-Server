#include "common.h"

/**
 * Initiats the main loop of the client that is taking non-blocking input and
 * outputing responses from the server
 * @param sm
 * Shared memory structure used to communicate between server and client
 */
void startClient(sharedMemory sm){
    char cmd[STRSIZE];
    int cmdLen,inNum;
    cmdLen = inNum = 0;
    
	#if defined(_WIN32)
		SYSTEMTIME timer;
	#else
		timeval timer;
	#endif
    
	
	resetTimer(timer);
    //TODO:refactor to initiation
    for(int i=0;i<MAXQUERIES;i++){
        queries[i].running = 0;
    }
    int nrOfOutstandingQueries = 0;
    
    sharedMemoryPtrs smptrs;
	getSharedVariablePtrs(&sm,&smptrs);
#if defined(_WIN32)
#else
	nonblock(NB_ENABLE);
#endif
    cout << "Enter nr to rotate and factor or quit/ctrl-c" <<endl;
    
    while(!quit){
        
        //Check if timeout has gone
        if(kbhit()){    //Checking for keyboard input
            
            if(getKBHitString(cmd,cmdLen)){ //Checking if the user is done typing command (end with CR)
                
                //Check that we can write to shared memory, should it block?
                //TODO: Check that nrofqueries hasnt exceeded 10, should block
                int res = getInNum(cmd);    //converting input to int, will be -1 if input is quit
                
                if(res == -1){
                    quit = 1;
                    *smptrs.number = res;
                    *smptrs.clientflag = '1';  //Setting 
                }else if(res == 0){
                    for(int i=0;i<3;i++){
                        writeToNumber(0,smptrs.number,smptrs.clientflag,nrOfOutstandingQueries);
                    }
                    
                }else{
					
                    writeToNumber(res,smptrs.number,smptrs.clientflag,nrOfOutstandingQueries);

                    
                }
            }   
        }
        //check if anything is to be read from shared mem
        if(nrOfOutstandingQueries){
            readResponse(queries,smptrs.slot,nrOfOutstandingQueries,smptrs.serverflag,timer);
            checkTimer(timer,smptrs.progress);
            
            
        }
        #if defined(_WIN32)
			Sleep(10);
		#else
			usleep(10000);
		#endif
        
    }
    //WAIT FOR SERVERSHUTDOWN SOMEHOW
#if defined(_WIN32)
#else
	nonblock(NB_DISABLE);
#endif
    cout << endl<<"quiting client after telling server and nicely wrapping myself up" <<endl;
}

/**
 * Writes int to shared variable if its available for writing
 * @param req
 * @param number
 * @param clientFlagPtr
 * @param nrOfOutstandingQueries
 */
void writeToNumber(int req,int *number,char *clientFlagPtr,int &nrOfOutstandingQueries){
    
    if(nrOfOutstandingQueries<MAXQUERIES){
        nrOfOutstandingQueries++;

        *number = req;
        *clientFlagPtr = '1';  //Setting 

        //Wait for server to have picked up the query and returned an index
        while(*clientFlagPtr == '1'){
            //TODO:Redesign hot to sync this
        }
        if(*number != -1){
            
            if(req == 0){
                queries[*number].testRun = 1;
            }else{
                queries[*number].testRun = 0;
            }
            
            queries[*number].running = 1;
			#if defined(_WIN32)
				GetSystemTime(&queries[*number].startTime);
			#else
				gettimeofday(&queries[*number].startTime, NULL);
			#endif       
        }else{
			
            cout << "Me (the client) has to stop addign queries when its full"<<endl;
        }


    }else{
        cout << endl<<endl<<"No go, the queue is full!"<<endl<<endl;
    }
    
}
/**
 * Reads responses from the server according to active queries
 * @param slot
 * is the array of shared memory containing responses
 * @param serverflag
 * Array of flags corresponding to the slots of shared memory, containing 
 * flags for syncronisations so the client reads without loosing data
 */
#if defined(_WIN32)
void readResponse(outstandingQuery *queries,int *slot,int &nrOfQueries,char *serverflag,SYSTEMTIME &timer){
#else
void readResponse(outstandingQuery *queries,int *slot,int &nrOfQueries,char *serverflag,timeval &timer){
#endif
    int tmpNr = 1;
    for(int i=0;i<MAXQUERIES;i++){
        
        if(queries[i].running){
			//cout << "reading from index: "<<i<<endl;
            if(serverflag[i] == '1'){
                if(slot[i] == -1){
                    
                    queries[i].running = 0;
                    nrOfQueries--;
                    cout << endl<<"query "<< i << " is done in: " <<TimerStop(queries[i].startTime)<< " sec"<<endl;
                }else{
                    //TODO: Remove testmode
                    
                    repeated = 0;
                    resetTimer(timer);
                    cout <<"Q"<<i<<": "<<slot[i] << " ";
                    if(queries[i].testRun){
                        cout << endl;
                    }else{
                        fflush(0);
                    }
                    
                }
                serverflag[i] = '0';
            }
        }
    }
    
}
/**
 * Takes a string and checks if it is the quit and returns -1, else it converts
 * input to int,
 * TODO: validate for other strings, returning -2 if its invalid input
 * @param input
 * @return 
 */
int getInNum(char *input){
    int ret = -1;
    if(strcmp(input,"q") != 0){
        ret = atoi(input);
    }
    
    return ret;
}
/**
 * Checking if the user is done inputing a string, its delimited by CR
 * should be used after the kbhit function returns true and unread input is 
 * residing in the buffer
 * If input is not CR, it appends it to the parameter string and incrementing 
 * the strlen
 * @param string
 * The string that is buffering input
 * @param strLen
 * Current length of the string
 * @return 
 * True if the input is ended with a CR, false if more is expected
 */
int getKBHitString(char *string,int &strLen){
    int ret = 0;
#if defined(WIN32)
    char c = _getch();
#else
char c = fgetc(stdin);
#endif
if((c == '\n') || (c == '\r')){
        string[strLen] = '\0';
        strLen = 0;
        ret = 1;
        printf("\b");		//- delete it
        fflush(0);
    }else{
        string[strLen] = c;
        strLen+=1;
    }
    
    return ret;
}

/**
 * Checks for keyboardhits
 * @return 
 */
#if defined(_WIN32)

#else
int kbhit(){
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); 
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    
    return FD_ISSET(STDIN_FILENO, &fds);
}


/**
 * Setting the terminalstate to non-canonical, reading input imidiately from
 * console imidiately
 * @param state
 * if we want canonical to be on/off
 */
void nonblock(int state){
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);  //- get the terminal state

    if (state == NB_ENABLE){
        ttystate.c_lflag &= ~ICANON;   //- turn off canonical mode
        ttystate.c_cc[VMIN] = 1;       //- minimum input chars read
    }
	else if (state==NB_DISABLE){
       ttystate.c_lflag |= ICANON;     //- turn on canonical mode
    }
    //- set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}
#endif
/**
 * Returns a float containing seconds passed since the given parameter
 * @param timer
 * @return 
 */
#if defined(_WIN32)
	float TimerStop(SYSTEMTIME &timer){
		SYSTEMTIME time2;
		GetSystemTime(&time2);

		long millis = (timer.wHour*3600000)+(timer.wMinute*60000)+(timer.wSecond * 1000) + timer.wMilliseconds;
		long millis2 = (time2.wHour*3600000)+(time2.wMinute*60000)+(time2.wSecond * 1000) + time2.wMilliseconds;
		
		float ret = millis2 - millis;
		return ret/1000;
	}
	void checkTimer(SYSTEMTIME &timer,int *progress){
		SYSTEMTIME time2;
		GetSystemTime(&time2);

		long millis = (timer.wHour*3600000)+(timer.wMinute*60000)+(timer.wSecond * 1000) + timer.wMilliseconds;
		long millis2 = (time2.wHour*3600000)+(time2.wMinute*60000)+(time2.wSecond * 1000) + time2.wMilliseconds;
		
		float timeElapsed = millis2-millis;
		if(timeElapsed>=500){
			printProgress(progress);
			resetTimer(timer);
		}

	}
	void resetTimer(SYSTEMTIME &timer){
		GetSystemTime(&timer);
	}

#else

/**
 * Takes a timer structure and returns the time passed since the given structure
 * was initiated in milliseconds
 * @param timer
 * @return 
 */
float TimerStop(timeval &timer){
	timeval time2;
    gettimeofday(&time2, NULL);
    long millis = (timer.tv_sec * 1000) + (timer.tv_usec / 1000);
    long millis2 = (time2.tv_sec * 1000) + (time2.tv_usec / 1000);

	float ret = millis2 - millis;
	return ret/1000;
	

}
/**
 * Takes a timer structure and a pointer to the progress array.
 * it checks if the time since given structure was initiated has passed 500 ms
 * if so it passed the progress structure to the printprogress function
 * @param timer
 * @param progress
 */
void checkTimer(timeval &timer,int *progress){

    timeval time2;
    gettimeofday(&time2, NULL);
    long millis = (timer.tv_sec * 1000) + (timer.tv_usec / 1000);
    long millis2 = (time2.tv_sec * 1000) + (time2.tv_usec / 1000);
    long timeElapsed = millis2 - millis;
    
    if(timeElapsed>=500){
        printProgress(progress);
        resetTimer(timer);
    }
}
void resetTimer(timeval &timer){
	gettimeofday(&timer, NULL);
}

#endif

/**
 * Takes the progress array as parameter and prints out the progress for each 
 * outstanding querie
 * @param progress
 */
void printProgress(int *progress){
    
    if(repeated){
		
        for(int k=0;k<2;k++){
			#if defined(_WIN32)
			//The printf does not work on windows, native cursor functionality should be implemented 
			#else
			printf("\033[K"); /* delete line */
            printf("\033[A"); /* move cursor one line up */
			#endif
        }
        
        for(int j = 0;j<MAXQUERIES;j++){
            if(queries[j].running){
                
				#if defined(_WIN32)
					//IMPLEMENT CURSORS
				#else
					printf("\033[K"); /* delete line */
					printf("\033[A"); /* move cursor one line up */
				#endif
                fflush(0);
            }
        }

    
    }
    cout << endl;
    for(int i=0;i<MAXQUERIES;i++){
        
        if(queries[i].running){
            
            cout << "query "<<i<<" : "<<progress[i]<<"%";
            printBar(progress[i]);
            cout << endl;
            
        }
    }
    cout << endl;
    repeated = 1;
    
}
/**
 * Prints a progressbar from 0-20 based on a percentile given as param
 * @param percentile
 */
void printBar(int percentile){
    
    float res = percentile/5;
    percentile = (int)res;
    cout << "|";
    for(int i=0;i<percentile;i++){
        cout << "#";
    }
    for(int j=0;j<20-percentile;j++){
        cout << "_";
    }
    cout << "|";
    fflush(0);
}
