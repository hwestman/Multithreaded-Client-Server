#include "common.h"
/**
 * Starts up the server side of the application, reading queries in a continous 
 * loop checking for a new querie. writing progress to the shared progress 
 * array for each iteration as well
 * @param sm
 * @param poolSize
 */
void startServer(sharedMemory sm,int poolSize){
    
    
	sharedMemoryPtrs smptrs;
	getSharedVariablePtrs(&sm,&smptrs);

	threadPool.setPoolSize(poolSize);
    threadPool.initialize();
	activeQueries.initiateSlots(smptrs.slot,smptrs.serverflag);
    
    /*
     *Main loop for the server 
     */
    
    while(!quit){
		    
        if(*smptrs.clientflag == '1'){  
			
            if(*smptrs.number == -1 || quit == 1){
                quit = 1;
                
            }else{
				
                int index = activeQueries.addQuery(*smptrs.number);
                
                if(index == -1){
                    cout << "attempted to add query when full, shame on client!"<<endl;
                }
                *smptrs.number = index;
                
            }
            *smptrs.clientflag = '0';
         
        }
        if(!quit){
			activeQueries.writeProgress(smptrs.progress);
			#if defined(_WIN32)
				Sleep(10);
			#else
				usleep(10000);
			#endif
            
        }
        
    }
	
    activeQueries.stopRunning(); //TODO: stop threadpool
    threadPool.stopRunning();
    cout << endl<<"quiting server after all has been nicely shutdown" <<endl;
}
/**
 * A function given as parameter to start up new threads that 
 * executes jobs in the dispatchqueue
 * @param arg
 * @return 
 */
void *slave(void *arg){
    
    threadPool.executeJob();
    return NULL;
}

/**
 * Factoring the number given in the job structure, and sends results to
 * the global queries based on the indexes in the job struct
 * @param theJob
 */
void factoriseNumber(job *theJob){
    
    int number = theJob->number;
    int progress;
    
    int half = number/2;
    
    int i=1;
    
    while(i<=half && threadPool.isRunning()){
        
        if(number%i == 0){
            //cout << "Job: "<<theJob->jobIndex<<" number: "<<number<<" factor: "<<i<<endl;
            activeQueries.sendFactor(theJob->index,i);
        }
        activeQueries.updateProgress(theJob->index,theJob->jobIndex,getPercent(i,half));

        i++;
        //usleep(10000);
		
    }
    
    if(threadPool.isRunning()){
        activeQueries.updateProgress(theJob->index,theJob->jobIndex,100);
        activeQueries.endQuery(theJob->index,theJob->testRun);
    }else{
        //This was manually shutdown and it did not complete
		pthread_exit(NULL);
		cout << "im a thread who manually shut down, bitch"<<endl;
    }
    
    
}
/**
 * Returns the percentile of given parameters
 * @param t
 * @param b
 * @return 
 */
int getPercent(int t,int b){
        float top = t;
        float bot = b;
        float c = (top/bot)*100;
        return (int) c;
}