
#include "common.h"
#include "SemLock.h"
//http://exceptional-code.blogspot.com.au/2013/05/a-c-thread-pool-implementation-using.html

Queries::Queries(){
    nrOfQueries = 0;
    testRunCount = 0;

}

int Queries::getAvailableIndex(){
    int index = -1;
    
    if(nrOfQueries<=MAXQUERIES){
        for(int i=0;i<MAXQUERIES;i++){
            if(!queries[i].isRunning()){
                index = i;
                break;
            }
        }
    }
    return index;
}

int Queries::addQuery(int number){
    
    int index = getAvailableIndex();
    
    if(index!=-1){
        
        queries[index].startRunning();
        
        nrOfQueries++;
        //MEMSET ITS PROGRESS!
        memset(mainProgress[index],0,sizeof(int)*INTBITS);

        rotateAndFactorize(number,index);
    }else{
        cout << "Tried adding query when full, shame on client!"<<endl;
    }
    return index;
}

void Queries::rotateAndFactorize(int number,int index){
    unsigned long rightRotated;
    int j,i;
    job aJob;
    
    if(number == 0){
        for(int i=0;i<10;i++){
            
            aJob.index = index;
            aJob.jobIndex = i;
            aJob.number = (testRunCount*10)+i;
            aJob.testRun = 1;
            threadPool.pushJob(aJob);
        }
        testRunCount++;
    }else{
        
        for(i = 0;i<32;i++){
            
            //rightRotated = (number >> i) | (number << (INTBITS - i));
            long c = number << (32-i); 
            rightRotated = (number >> (i)) | c;

            //cout << rightRotated<<endl;
            aJob.index = index;
            aJob.testRun = 0;
            aJob.jobIndex = i;
            aJob.number = (int)rightRotated;

            threadPool.pushJob(aJob);
            //factoriseNumber(rightRotated,factors);   
        }
    }
}
int Queries::endQuery(int index,bool testmode){
    
    int done = 1;
    int maxJob;
    
    if(testmode){
        maxJob = 10;
    }else{
        maxJob = INTBITS;
    }
    for(int i=0;i<maxJob;i++){
        if(mainProgress[index][i] != TOTAL){
            //cout << "jobindex: "<<i<<" is not done yet its: "<<mainProgress[index][i]<<endl;
            done = 0;
            break;
        }
    }
    if(done){
        if(testmode){
            testRunCount--;
        }
        
        queries[index].write(-1);
        queries[index].stopRunning();
        nrOfQueries--;
    }
    return 0;
}
void Queries::initiateSlots(int *slots,char *serverFlags){
    for(int i=0;i<MAXQUERIES;i++){
        queries[i].setSlot(&slots[i],&serverFlags[i]);
    }
    
}
int Queries::sendFactor(int index,int factor){
	
    queries[index].write(factor);
    return 0;
}
void Queries::updateProgress(int index,int jobIndex,int percent){
    mainProgress[index][jobIndex] = percent;
    
}

void Queries::writeProgress(int *progress){
    
    int progSum;
    for(int j=0;j<MAXQUERIES;j++){
        if(queries[j].isRunning()){
            progSum =0;
            for(int i=0;i<INTBITS;i++){
                progSum+=mainProgress[j][i];
            }
            progress[j] = getPercent(progSum,INTBITS*100);
            
        }   
    }
    
}
/**
 * 
 */
void Queries::stopRunning(){
    for(int i =0;i<MAXQUERIES;i++){
        queries[i].stopRunning();
        queries[i].killIt();
    }
    //cout << "Server killed all SemLocks"<<endl;
}

/*
 *******************************************************************************
 */

/*SEMAPHORE*/
SemLock::SemLock(){
    running = false;
    int condRet = pthread_cond_init(&c, NULL);
    if(condRet != 0){
        cout << "cond init probelm "<< condRet <<endl;
    }
    int lockRet = pthread_mutex_init(&m,NULL);
    if(lockRet != 0){
        cout << "lock init probelm "<< lockRet <<endl;
    }
}
void SemLock::setSlot(int *aSlot,char *aServerFlag){
    slot = aSlot;
    serverFlag = aServerFlag;
}
int SemLock::write(int factor){
    
    this->Lock();
        while(*serverFlag != '0'){
			
            int cret = pthread_cond_wait(&c,&m);
           
            if(cret != 0){
               cout << "condret error in SemLock: "<< cret <<endl;
               break;
            }
        }
        
        *slot = factor;
        *serverFlag = '1';
        //Wait til client has read before we signal
        while(*serverFlag == '1'){
        
        }
        //pthread_cond_broadcast(&c);
		pthread_cond_signal(&c);
    this->Unlock();
    
    return 0;
}
bool SemLock::isRunning(){
    return running;
}
void SemLock::startRunning(){
    running = true;
}
void SemLock::stopRunning(){
    running = false;
    
    
    
}

void SemLock::killIt(){
    pthread_cond_destroy(&c);
    pthread_mutex_destroy(&m);
}

/*
 *******************************************************************************
 */

ThreadPool::ThreadPool(){
    
    running = true;
    queueEnd = 0;
    queueStart = 0;
    queueCount = 0;
    
    
    //TODO: CHECK IF ITS SET AS ARGUMENT
    poolSize = 64;
    threadPool = (pthread_t **)malloc(sizeof(pthread_t)*poolSize);
    
    int condRet = pthread_cond_init(&condition, NULL);
    if(condRet != 0){
        cout << "cond init probelm "<< condRet <<endl;
    }
    int lockRet = pthread_mutex_init(&lock,NULL);
    if(lockRet != 0){
        cout << "lock init probelm "<< lockRet <<endl;
    }
}
ThreadPool::~ThreadPool(){
    
    //for(int i=0;i<poolSize;i++){
        //delete []threadPool[i];
        
    //}
    free(threadPool);
    
}
void ThreadPool::initialize(){
    cout << "POOLSIZE IS: "<<poolSize<<endl;
    for (int i = 0; i < poolSize; i++) {
        pthread_t tid;
        
        int rc = pthread_create(&tid,NULL,slave, (void*) threadPool);
        
        if (rc){
            printf("ERROR = %d\n", rc);
        }
        this->threadPool[i] = &tid;
                
  }
}
void ThreadPool::destroyPool(){
     void* result;
     int ret;
	 
    for(int i = 0;i<poolSize;i++){
		#if defined(_WIN32)
		//Why wont windows join? it just shuts down
		#else
        ret = pthread_join(*this->threadPool[i], &result);
		#endif
		pthread_cond_broadcast(&condition);
    }

    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&lock);
    
    //cout << "Server has joined all threads and killed all mutexes"<<endl;
    

}
void ThreadPool::pushJob(job j){
    
    Lock();
    
   
    dispatchQueue[queueEnd] = j;
    queueEnd = (queueEnd+1)%QUEUESIZE;
    
    queueCount++;
    pthread_cond_broadcast(&condition);
    Unlock();
    

}

void ThreadPool::popJob(job &j){
    int ret = 0;
    
   
    j = dispatchQueue[queueStart];
   
   queueStart = (queueStart+1)%QUEUESIZE; 

    queueCount--;
}

void ThreadPool::executeJob(){
    
    job jobToExecute;
    while(running) {
    
        Lock();
        while ((running) && (queueCount==0)) {
          // Wait until there is a task in the queue
          // Unlock mutex while wait, then lock it back when signaled

            int cret = pthread_cond_wait(&condition,&lock);
            if(cret != 0){
              cout << "condret error in SemLock: "<< cret <<endl;
              break;
           }

        }

        // If the thread was woken up to notify process shutdown, return from here
        if (!running) {

            Unlock();
            pthread_exit(NULL);
        }else{
            //job to execute
            popJob(jobToExecute);
            
            Unlock();
            if(jobToExecute.testRun){
                activeQueries.sendFactor(jobToExecute.index,jobToExecute.number);
                activeQueries.updateProgress(jobToExecute.index,jobToExecute.jobIndex,100);
                activeQueries.endQuery(jobToExecute.index,jobToExecute.testRun);
            }else{
                factoriseNumber(&jobToExecute);
            }
            
        }
  }
}
void ThreadPool::stopRunning(){
    running = false;
    pthread_cond_broadcast(&condition);
    destroyPool();
}
void ThreadPool::setPoolSize(int size){
    poolSize = size;

}
bool ThreadPool::isRunning(){
    return running;
}
