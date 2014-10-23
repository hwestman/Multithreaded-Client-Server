/* 
 * File:   SemLock.h
 * Author: hwestman
 *
 * Created on September 25, 2013, 10:41 AM
 */

#ifndef SemLock_H
#define	SemLock_H

#include "Headers.h"
#include "common.h"


class SemLock{
private:
    pthread_mutex_t m;
    pthread_cond_t c;
    bool running;
    
    int *slot;
    char *serverFlag;
    
    void Lock(){ pthread_mutex_lock(&m); }          // <- helper inlines added
    void Unlock(){ pthread_mutex_unlock(&m); }
public:
    
    SemLock();
    void setSlot(int *aSlot,char *aServerFlag);
    int write(int factor);
    bool isRunning();
    void startRunning();
    void stopRunning();
    void killIt();
    
};
class ThreadPool{
private:
        bool running;
        pthread_t **threadPool;
        pthread_mutex_t lock;
        pthread_cond_t  condition;

        job dispatchQueue[QUEUESIZE];
        int poolSize,queueCount,queueStart,queueEnd;

        void Lock(){ if(pthread_mutex_lock(&lock)!=0){cout << "i cant lock this hsit"<<endl;} }          // <- helper inlines added
        void Unlock(){ pthread_mutex_unlock(&lock); }
        void popJob(job &j);

    public:
        ThreadPool();
        ~ThreadPool();
        void initialize();
        void setPoolSize(int size);
        void pushJob(job j);
        void destroyPool();
        void executeJob();
        void stopRunning();
        bool isRunning();
    
    
};
class Queries{
    private:
        SemLock queries[MAXQUERIES];
        int nrOfQueries;
        int testRunCount;
        int getAvailableIndex();
        int mainProgress[MAXQUERIES][INTBITS];
        void rotateAndFactorize(int number,int index);
        
    public:
        
        Queries();
        int sendFactor(int index,int factor);
        int addQuery(int number);
        int endQuery(int index,bool testmode);
        void initiateSlots(int *slots,char *serverFlags);
        void writeProgress(int *progress);
        void updateProgress(int index,int jobIndex,int percent);
        void stopRunning();
    
};


#endif	/* SemLock_H */

