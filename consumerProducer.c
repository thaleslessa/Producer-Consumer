//********************************************************************
//
// Thales Lessa
// 
// The Prime Number Producer - Consumer Problem
// with Faulty Producer Threads
//
//********************************************************************

#include <ctype.h>
#include <unistd.h> /* Symbolic Constants */
#include <sys/types.h> /* Primitive System Data Types */
#include <errno.h> /* Errors */
#include <stdio.h> /* Input/Output */
#include <stdlib.h> /* General Utilities */
#include <pthread.h> /* POSIX Threads */
#include <string.h> /* String handling */
#include <malloc.h> /* For memory management */
#include <semaphore.h> /* Semaphores */
#include <stdbool.h> /* Boolean operators */

/* prototype for producer thread routine */
void producer_function ( void *ptr );

/* prototype for faulty producer thread routine */
void faulty_producer_function ( void *ptr );

/* prototype for consumer thread routine */
void consumer_function ( void *ptr );

/* prototype for prime checking routine */
bool isPrime ( int nbr );

/* Opaque buffer element type.  This would is defined by the application. */
typedef struct { int value; } ElemType;

/* Circular buffer object */
typedef struct {
    int         size;   /* maximum number of elements           */
    int         start;  /* index of oldest element              */
    int         end;    /* index at which to write new element  */
    ElemType   *elems;  /* vector of elements                   */
} CircularBuffer;

/* Initialization of Circular Buffer */
void cbInit(CircularBuffer *cb, int size) {
    cb->size  = size + 1; /* include empty elem */
    cb->start = 0;
    cb->end   = 0;
    cb->elems = (ElemType *)calloc(cb->size, sizeof(ElemType));
}

/* Write an element */
void cbWrite(CircularBuffer *cb, ElemType *elem) {
    cb->elems[cb->end] = *elem;
    cb->end = (cb->end + 1) % cb->size;
}
 
/* Read oldest element */
void cbRead(CircularBuffer *cb, ElemType *elem) {
    *elem = cb->elems[cb->start];
    cb->start = (cb->start + 1) % cb->size;
}

void cbFree(CircularBuffer *cb) {
    free(cb->elems); /* OK if null */ }

/* struct to hold data to be passed to a thread
this shows how multiple data items can be passed to a thread */
typedef struct str_thdata
{
int thread_no;
int times;
int number;
} thdata;

//Global variables
volatile int running_producers;		//Counter of running producers, when reaches 0, program can stop executing and show statistics
pthread_mutex_t mutex;			//mutex lock to be acquired by threads to do their critical sections
sem_t full;				//full semaphore
sem_t empty;				//empty semaphore
CircularBuffer buffer;			//buffer struct
int count, timesFull, timesEmpty, nPrimeFound, totalproduce, totalFproduce, totalconsume;	//counters for #of items in buffer, times it was full, times it was
												//empty, non-primes found, total items produced, total faulty items
												//produced and total numbers consumed
thdata dataarray1 [100];	//array of producer data
thdata dataarray2 [100];	//array of faulty producer data
thdata dataarray3 [100];	//array of consumer data

//****************************************************************************************************************************
//
// Main Function
//
// The function reads command line arguments, creates threads that will write and read to buffer, handling the producer-consumer problem
// It doesn't join the threads because that would generate a core dump, since the threads finish executing when they finish their job
//
// Return Value
// -------------------
// int
//
// Function Parameters
// -------------------
// argc			int		value		Number of arguments coming from the command line
// argv			char**		reference	Pointer to the array of strings that are arguments in the command line
//
// Local Variables
// -------------------
// producerThread		pthread_t		Thread to do the producer thread function
// faulty_producerThread	pthread_t		Thread to do the faulty producer thread function
// consumerThread		pthread_t		Thread to do the consumer thread function
//
// array1			pthread_t[]		Array of producer Threads
// array2			pthread_t[]		Array of faulty producer Threads
// array3			pthread_t[]		Array of consumer Threads
// itemsNum			int			Number of items for producers to produce
// length			int			Length of buffer
// consumerNum			int			Number of consumer threads
// producerNum			int			Number of producer threads
// faultyNum			int			Number of faulty producer threads
// opt				int			Integer with command line arguments, is -1 when there are no more arguments
// i				int			Loop iteration variables
// testBuffer
//
//****************************************************************************************************************************

int main(int argc, char *argv[])
{

count = 0;
timesFull = 0;
timesEmpty = 0;
nPrimeFound = 0;

pthread_t producerThread; /* producer thread variable */
pthread_t faulty_producerThread; /* faulty producer thread variable */
pthread_t consumerThread; /* consumer thread variable */

pthread_t array1 [100];
pthread_t array2 [100];
pthread_t array3 [100];

int itemsNum=0, length=0, consumerNum=0, producerNum=0, faultyNum=0, opt;   
int i;

    while ((opt = getopt(argc, argv, "f:p:c:l:n:")) != -1) {
        switch (opt) {
        case 'n':
            itemsNum = atoi(optarg);
            break;
        case 'l':
            length = atoi(optarg);
            break;
	case 'c':
            consumerNum = atoi(optarg);
            break;
	case 'p':
            producerNum = atoi(optarg);
            break;
	case 'f':
            faultyNum = atoi(optarg);
            break;
        default: /* '?' */
            fprintf(stderr, "Usage: %s [-n] Number of items per producer thread, [-l] Length of the buffer, [-c] Number of consumer threads, [-p] Number of producer threads, [-f] Number of faulty producer threads\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

running_producers = producerNum + faultyNum;

if (length == 0){
printf ("Buffer length is 0, program cannot run\n");
return 0;
}

if (consumerNum == 0){
printf ("Program cannot run without consumers\n");
return 0;
}

if (running_producers == 0){
printf ("Program cannot run without producers\n");
return 0;
}

printf ("Starting threads ... \n\n\n");

pthread_mutex_init( &mutex, NULL );
sem_init( &full, 0, 0 );
sem_init( &empty, 0, length );

cbInit(&buffer, length);

//creating producer threads
for(i = 0 ; i < producerNum ; i++){

/* initialize data to pass to the thread */
dataarray1[i].thread_no = i+1;
dataarray1[i].times = itemsNum;
dataarray1[i].number = 0;

/* create thread */
pthread_create (&producerThread, NULL, (void *) &producer_function, (void *) &dataarray1[i]);

array1[i] = producerThread;

}

//creating faulty producer threads
for(i = 0 ; i < faultyNum ; i++){

/* initialize data to pass to the thread */
dataarray2[i].thread_no = i+1;
dataarray2[i].times = itemsNum;
dataarray2[i].number = 0;

/* create thread */
pthread_create (&faulty_producerThread, NULL, (void *) &faulty_producer_function, (void *) &dataarray2[i]);

array2[i] = faulty_producerThread;

}

//creating consumer threads
for(i = 0 ; i < consumerNum ; i++){

/* initialize data to pass to the thread */
dataarray3[i].thread_no = i+1;
dataarray3[i].number = 0;

/* create thread */
pthread_create (&consumerThread, NULL, (void *) &consumer_function, (void *) &dataarray3[i]);

array3[i] = consumerThread;

}

/* While there are producers running, wait for them to finish executing */
  while (running_producers > 0)
  {
     sleep(1);
  }

   printf("\nPRODUCER / CONSUMER SIMULATION COMPLETE\n");   
   printf("=======================================");
   printf("\nItems per producer = \t\t\t%d \nLength of buffer = \t\t\t%d \nNumber of consumer threads = \t\t%d \nNumber of producer threads = \t\t%d \nNumber of faulty producer threads = \t%d \n\n", itemsNum, length, consumerNum, producerNum, faultyNum);

printf("Total Number of Items Produced: \t%d\n", totalproduce);
for (i = 0; i < producerNum; i++)
{
printf("Thread %d: \t\t\t\t%d\n", dataarray1[i].thread_no, dataarray1[i].number);
}
printf("\n");

printf("Total Number of Faulty Items Produced: \t%d\n", totalFproduce);
for (i = 0; i < faultyNum; i++)
{
printf("Thread %d: \t\t\t\t%d\n", dataarray2[i].thread_no, dataarray2[i].number);
}
printf("\n");

printf("Total Number of Items Consumed: \t%d\n", totalconsume);
for (i = 0; i < consumerNum; i++)
{
printf("Thread %d: \t\t\t\t%d\n", dataarray3[i].thread_no, dataarray3[i].number);
}
printf("\n");

printf("Total Number of Times Buffer was Full: \t\t%d \nTotal Number of Times Buffer was Empty: \t%d \nTotal Number of Non-primes detected: \t\t%d\n\n", timesFull, timesEmpty, nPrimeFound);


    cbFree(&buffer);		//free the memory used for the buffer

    return 0;
}

//********************************************************************
//
// Producer Thread Function
//
// This function checks if the buffer is full, if so, it waits. When
// the buffer is not full, the thread waits for a mutex lock. When it
// gets it, it writes a prime value from the buffer.
//
// Return Value
// ------------
// void
//
// Function Parameters
// -------------------
// ptr		void*		reference	thdata cast into void * containing information passed to thread. Contains thread number,
//						times (number of items to produce) and number of thread reads/writes, which is updated constantly
//
// Local Variables
// ---------------
// elem		ElemType	Contains the value of the current element in the buffer (first, the one to be added to buffer, and later for traversing)
// traverse	int		Loop iteration variable for traversing the buffer when printing it
// data		thdata		Contains thdata coming from main method
// limit	int		Number of items thread has to produce
// i		int		Loop iteration variable
// writecount	int		Contains the number of writes this thread has made
// rdm		int		Random number generated, is checked many times until is prime
//
//*******************************************************************

void producer_function ( void *ptr )
{
pthread_mutex_lock( &mutex );
thdata *data1;
data1 = (thdata *) ptr; /* type cast to a pointer to thdata */
pthread_mutex_unlock( &mutex );

int limit = data1->times;
int i, rdm, traverse, writecount = 0;

for (i = 0; i < limit; i++){

rdm = rand() % 9997 + 2;

while (!(isPrime (rdm))){
rdm = rand() % 9997 + 2;
}

ElemType elem = {rdm};

sem_wait( &empty );			//waits if buffer is full
pthread_mutex_lock( &mutex );		//acquires mutex lock

cbWrite(&buffer, &elem);
count++;
writecount++;
totalproduce++;

/* do the work */
printf("Producer %d writes %d. Buffer contains %d/%d items. ", data1->thread_no, rdm, count, (buffer.size -1));

printf("Elements in buffer are: [ ");

traverse = buffer.start;

while (traverse != buffer.end){
elem = buffer.elems[traverse];
printf("%d ", elem.value);

traverse = (traverse + 1) % buffer.size;
}

printf("] ");

if (count == (buffer.size-1)){
timesFull++;
printf("** FULL **");
}

printf("\n\n");

pthread_mutex_unlock( &mutex );
sem_post( &full );

}

dataarray1[data1->thread_no-1].number = writecount;

running_producers--;

pthread_exit(0); /* exit */
}

//********************************************************************
//
// Faulty Producer Thread Function
//
// This function checks if the buffer is full, if so, it waits. When
// the buffer is not full, the thread waits for a mutex lock. When it
// gets it, it writes an even value from the buffer.
//
// Return Value
// ------------
// void
//
// Function Parameters
// -------------------
// ptr		void*		reference	thdata cast into void * containing information passed to thread. Contains thread number,
//						times (number of items to produce) and number of thread reads/writes, which is updated constantly
//
// Local Variables
// ---------------
// elem		ElemType	Contains the value of the current element in the buffer (first, the one to be added to buffer, and later for traversing)
// traverse	int		Loop iteration variable for traversing the buffer when printing it
// data		thdata		Contains thdata coming from main method
// limit	int		Number of items thread has to produce
// i		int		Loop iteration variable
// writecount	int		Contains the number of writes this thread has made
// rdm		int		Random number generated, is checked many times until is even
//
//*******************************************************************

void faulty_producer_function ( void *ptr )
{

thdata *data;
data = (thdata *) ptr; /* type cast to a pointer to thdata */

int limit = data->times;
int i, rdm, traverse, writecount = 0;
for (i = 0; i < limit; i++){

rdm = rand() % 9997 + 2;

while(rdm % 2 != 0){
rdm = rand() % 9997 + 2;
}

ElemType elem = {rdm};

sem_wait( &empty );		//checks if buffer is full
pthread_mutex_lock( &mutex );	//acquires lock

/* do the work */
cbWrite(&buffer, &elem);

//increments all counters
count++;
writecount++;
totalFproduce++;

printf("Pr*d*c*r %d writes %d. Buffer contains %d/%d items. ", data->thread_no, rdm, count, (buffer.size -1));

printf("Elements in buffer are: [ ");

traverse = buffer.start;

while (traverse != buffer.end){
elem = buffer.elems[traverse];
printf("%d ", elem.value);

traverse = (traverse + 1) % buffer.size;
}

printf("] ");

if (count == (buffer.size-1)){
timesFull++;
printf("** FULL **");
}

printf("\n\n");

pthread_mutex_unlock( &mutex );	//gives away mutex lock
sem_post( &full );		//tells other threads an item has been added to buffer (is not empty and could be full)

}
running_producers--;		//decrements producers counter

dataarray2[data->thread_no-1].number = writecount;

pthread_exit(0); /* exit */
}

//********************************************************************
//
// Consumer Thread Function
//
// This function checks if the buffer is empty, if so, it waits. When
// the buffer is not empty, the thread waits for a mutex lock. When it
// gets it, it reads a value from the buffer and removes it. If the
// number is not prime, it shows a NOT PRIME message. It also prompts
// EMPTY if the buffer is empty after it removes the value from the buffer
//
// Return Value
// ------------
// void
//
// Function Parameters
// -------------------
// ptr		void*		reference	thdata cast into void * containing information passed to thread. Contains thread number,
//						times (unused in this function) and number of thread reads/writes, which is updated constantly
//
// Local Variables
// ---------------
// elem		ElemType	Contains the value of the current element in the buffer (is used only in the traverse loop)
// elem2	ElemType	Contains the value of the element that is to be read and removed from the buffer
// traverse	int		Loop iteration variable for traversing the buffer when printing it
// prime	bool		Boolean that says if random number generated is prime or not
// data		thdata		Contains thdata coming from main method
//
//*******************************************************************

void consumer_function ( void *ptr1 )
{

ElemType elem, elem2;
int traverse;
bool prime;
thdata *data2;
data2 = (thdata *) ptr1; /* type cast to a pointer to thdata */

while (1)
{

sem_wait( &full );			//checks if buffer is empty
pthread_mutex_lock( &mutex );		//gets for a lock

/* do the work */
cbRead(&buffer, &elem2);

//increment counters
count--;
totalconsume++;
dataarray3[data2->thread_no-1].number++;

printf("Consumer %d reads %d. Buffer contains %d/%d items. ", data2->thread_no, elem2.value, count, (buffer.size -1));

printf("Elements in buffer are: [ ");

traverse = buffer.start;

while (traverse != buffer.end){
elem = buffer.elems[traverse];
printf("%d ", elem.value);

traverse = (traverse + 1) % buffer.size;	//make sure it loops back to beginning of buffer
}

printf("] ");

prime = isPrime (elem2.value);

if (!prime){
printf("NOT PRIME ");
nPrimeFound++;
}

if (count == 0){
timesEmpty++;
printf("** EMPTY **");
}

printf("\n\n");

pthread_mutex_unlock( &mutex );		//gives mutex lock away
sem_post( &empty );			//says that there is one less item in buffer (is not full, and could be empty)
}

pthread_exit(0); /* exit */
}

//****************************************************************************************************************************
//
// Is Prime Function
//
// This function checks if the incoming number is prime or not. Returns
// true if it is prime or false otherwise.
//
// Return Value
// -------------------
// bool			true/false if prime
//
// Function Parameters
// -------------------
// nbr		int		value		Integer sent from producer thread to be checked if is prime or not	
//
// Local Variables
// -------------------
// i		int		Loop Iteration Variable
// number	int		Integer passed from the main function, contains the number to be determined if is prime or not
//
//****************************************************************************************************************************

bool isPrime ( int nbr )
{	
	int i;
	int number = nbr;

	if (number <= 1) return false;
	
	for (i=2; i*i<=number; i++) {
        if (number % i == 0) return false;
	}	
	return true;
}
