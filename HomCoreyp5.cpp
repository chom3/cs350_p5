#include <stdio.h>
#include "time_functions.h"
#include <stdlib.h>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <vector>

using namespace std;

//Global Variables
time_t x = 0;
unsigned short y = 0;
int itgr = 0;
int rrCount = 0;
int wwCount = 0;
int nWriters = 0, nReaders = 0, WaitingWriters = 0, WaitingReaders = 0;
//Condition Variables
pthread_mutex_t canRead = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t canWrite = PTHREAD_MUTEX_INITIALIZER;
// mutex mtx;
// unique_lock<mutex> lck(mtx);
// condition_variable canRead, canWrite;
/**
* startread function will begin the monitor for the reading.
* 	begin
if busy then OKtoread.wait;
readercount := readercount + 1;
OKtoread.signal;
(* Once one reader can start, they all can *)
end startread;
startread will also print:
>>> DB value read =: SSS:mmm by reader number: rr
*/
void startread(int rrCount){
	get_wall_clock(&x, &y);
	cout << ">>>DB value read =: " << x << ":" << y << " by reader number: " << rrCount << endl;
	if (nWriters == 1 || WaitingWriters > 0)
	{
		++WaitingReaders;
		pthread_mutex_lock(&canRead);
		// canRead.wait(lck);
		--WaitingReaders;
	}
	++nReaders;
	pthread_mutex_unlock(&canRead);
	// canRead.notify_one();
}
/**
* endread function will begin the monitor for the reading.
* 	begin
readercount := readercount - 1;
if readercount = 0 then OKtowrite.signal;
end endread;
*/
void endread(){
	if (--nReaders == 0)
	{
		pthread_mutex_unlock(&canWrite);
		// canWrite.notify_one();
	}
}
/**
* startwrite function will begin the monitor for the reading.
* 	begin
if busy OR readercount != 0 then OKtowrite.wait;
busy := true;
end startwrite;
startwrite will also print:
*** DB value set to: SSS:mmm by writer number: ww
*/
void startwrite(int wwCount){
	get_wall_clock(&x, &y);
	cout << "*** DB value set to: " << x << ":" << y << " by writer number: " << wwCount << endl;
	if (nWriters == 1 || nReaders > 0)
	{
		++WaitingWriters;
		pthread_mutex_lock(&canWrite);
		// canWrite.wait(lck);
		--WaitingWriters;
	}
	nWriters = 1;
}
/**
* endwrite function will begin the monitor for the reading.
* 	begin
busy := false;
if OKtoread.queue then OKtoread.signal
else OKtowrite.signal;
end endwrite;
*/
void endwrite(){
	nWriters = 0;
	if (WaitingReaders)
	{
		pthread_mutex_unlock(&canRead);
		// canRead.notify_one();
	}
	else
	{
		 pthread_mutex_unlock(&canWrite);
		// canWrite.notify_one();
	}
}
/**
* Reader function will output value to console.
*/
void *readerFunc(void *arg)
{
	int readDelay = *((int*)arg);
	for (int i = 0; i < readDelay; i++){}
	startread(rrCount);
	rrCount++;
	cout << "Current integer value: " << itgr << endl;
	endread();
	return arg;
}

/**
* Writer function will increment value.
*/
void *writerFunc(void *arg)
{
	int writeDelay = *((int*)arg);
	for (int i = 0; i < writeDelay; i++){}
	startwrite(wwCount);
	wwCount++;
	itgr++;
	endwrite();
	return arg;
}

int main(int argc, char* argv[]) {
	/**
	* 'r' number of readers argv[1]
	* 'w' number of writers argv[2]
	* 'R' delay for a reader to restart argv[3]
	* 'W' delay for a writer to restart argv[4]
	*/
	// cout << "argc = " << argc << endl; 
	// for(int i = 0; i < argc; i++) 
	// cout << "argv[" << i << "] = " << argv[i] << endl;
	// get_wall_clock(&x,&y);
	// cout << x << endl;
	// cout << y << endl;
	int readers = atoi(argv[1]);
	int writers = atoi(argv[2]);
	int readDelay = atoi(argv[3]);
	int writeDelay = atoi(argv[4]);
	vector<pthread_t> rtid(readers);
	vector<pthread_t> wtid(writers);
	for (int i = 0; i < readers; i++)
	{
		pthread_create(&rtid[i], NULL, readerFunc, (void *)&readDelay);
	}
	for (int i = 0; i < writers; i++)
	{
		pthread_create(&wtid[i], NULL, writerFunc, (void *)&writeDelay);
	}
	for (int t = 0; t < writers; t++)
	{
		pthread_join(wtid[t], NULL);
	}
	for (int t = 0; t < readers; t++)
	{
		pthread_join(rtid[t], NULL);
	}
	char anyChar;
	cin >> anyChar;
}
