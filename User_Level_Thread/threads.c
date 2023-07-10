//User level threads library
// INCLUDES
#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "threads.h"

// DEFINES
#define SECOND 1000000
#define MAX_NO_OF_THREADS 1000 /* in any state */
#define STACK_SIZE 10000 //4096
#define TIME_QUANTUM 1


//////////////////////////////////////////////////////
/// Convert seconds to milliseconds
#define SEC_TO_MS(sec) ((sec)*1000)
/// Convert seconds to microseconds
#define SEC_TO_US(sec) ((sec)*1000000)
/// Convert seconds to nanoseconds
#define SEC_TO_NS(sec) ((sec)*1000000000)

/// Convert nanoseconds to seconds
#define NS_TO_SEC(ns)   ((ns)/1000000000)
/// Convert nanoseconds to milliseconds
#define NS_TO_MS(ns)    ((ns)/1000000)
/// Convert nanoseconds to microseconds
#define NS_TO_US(ns)    ((ns)/1000)
//////////////////////////////////////////////////////


// Black box code
////////////////////////////////////////////////////////////////////////
#ifdef __x86_64__
// code for 64 bit Intel arch

//typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

//A translation required when using an address of a variable
//Use this as a black box in your code.
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#else
// code for 32 bit Intel arch

//typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

//A translation required when using an address of a variable
//Use this as a black box in your code.
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#endif
////////////////////////////////////////////////////////////////////////

// Global variables
thread_queue_t *thread_list;     /* the list of all threads */
thread_queue_t *ready_list;      /* the list of all ready threads */
thread_t *current;               /* the current running thread */
int next_thread = 0;             /* Used for assigning IDs */
int scheduling_type;             /* Scheduling type 0 = RR, 1 = LOT */
int clean = 0;                   /* if in cleanup, exit out of dispatch */
unsigned start_time = 0;         /* Time a thread is started */

extern void thread_enqueue(thread_t*, thread_queue_t*);
// enqueue t to back of queue


long long int global_work=0;
long long int tot_context_switch_tim=0;
long long int context_switch_cnt=0;
long long int n_thread;

extern thread_t* scheduler();
//implementation of thread scheduler, RR and LOT

int CreateThread(void (*f) (void), int priority)
{
    // Return -1 if fail
    if (thread_list->size + 1 > MAX_NO_OF_THREADS)
    {
        printf("Too many threads\n");
        return -1;
    }
    thread_t *thread = malloc(sizeof(thread_t));

    thread->status = malloc(sizeof(status_t));
    thread->stack = malloc(STACK_SIZE);
    thread->status->id = next_thread;
    thread->priority = priority;
    next_thread++;
    thread->status->state = READY;

    thread->sp = (address_t)thread->stack + STACK_SIZE - sizeof(address_t);
    thread->pc = (address_t)f;
    sigsetjmp(thread->jbuf, 1);
    (thread->jbuf->__jmpbuf)[JB_SP] = translate_address(thread->sp);
    (thread->jbuf->__jmpbuf)[JB_PC] = translate_address(thread->pc);
    sigemptyset(&thread->jbuf->__saved_mask);

    thread_enqueue(thread, ready_list);
    thread_enqueue(thread, thread_list);
    return thread->status->id;
}

void InsertWrapper(thread_t *t, thread_queue_t *q)
{
    if (scheduling_type == 0) // RR
    {
        thread_enqueue(t, q); // insert the context-switched-out thread at the tail of the ready queue
    }
    else
    {
        //InsertAtHead(t, q);
        // simply inserts the context-switched-out thread at the head of the ready queue
        return;
    }
}


unsigned long micros()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    unsigned long us = SEC_TO_US((unsigned long)ts.tv_sec) + NS_TO_US((unsigned long)ts.tv_nsec);
    return us;
}

unsigned long millis()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    unsigned long ms = SEC_TO_MS((unsigned long)ts.tv_sec) + NS_TO_MS((unsigned long)ts.tv_nsec);
    return ms;
}

unsigned long nanos()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    unsigned long ns = SEC_TO_NS((unsigned long)ts.tv_sec) + (unsigned long)ts.tv_nsec;
    return ns;
}

// Signal handler
void Dispatch(int sig)
{
    if (clean == 1)
    {
        return;
    }
    unsigned time_delta = GetCurrentTime() - start_time;// will give you the amount of time the thread ran
                                                        // before being interrupted (time quantum)
    status_t *s = current->status;
    s->total_exec_time += time_delta;


	struct timespec beg;
  	struct timespec curr;

  // clock_gettime is used to retrieve the time of the specified clock clk_id
  // it is stored in struct begin here
  // CLOCK_MONOTONIC: Clock that cannot be set and represents monotonic time 
  // since some unspecified starting point. 
    


    // Save state of current thread
    int ret = sigsetjmp(current->jbuf, 1);
    if (ret == 1)
    {
        return;
    }

    // Itterate through all threads list and deal with them.

	//clock_gettime(CLOCK_MONOTONIC, &beg);
	//unsigned t1 = GetCurrentTime();
	struct timeval t1,t2;
	gettimeofday(&t1,NULL);
	unsigned long t11 = nanos();//1000000 * t1.tv_sec + t1.tv_usec;

    thread_node_t *node = thread_list->head;
    while(node != NULL)
    {
        switch(node->thread->status->state)
        {
            // Do nothing
        case READY:
            break;
        case RUNNING:
            break;
        case SUSPENDED:
            break;
        case SLEEPING:
            if (GetCurrentTime() >= node->thread->status->wake_time)
            {
                printf("wake time\n");
                thread_enqueue(node->thread, ready_list);
                node->thread->status->state = READY;
            }
            break;
        case FINISHED:
            RemoveFromList(node->thread->status->id, ready_list);
            break;
        }
        node = node->next;
    }
    // Iterate through ready_queue to update wait times
    thread_node_t *ready = ready_list->head;
    while(ready != NULL)
    {
        ready->thread->status->total_wait_time += time_delta;
        ready = ready->next;
    }

    // Schedule new thread
    if (current->status->state == RUNNING)
    {
        InsertWrapper(current, ready_list);
        current->status->state = READY;
    }
	//printf("Hi\n");

    thread_t *next = GetNextThread();
        if(next==NULL)
	{
		CleanUp();
	}

    current = next;
    status_t *stat = next->status;
    stat->state = RUNNING;
    stat->no_of_bursts++;
    stat->avg_exec_time = (stat->total_exec_time / stat->no_of_bursts);
    stat->avg_wait_time = (stat->total_wait_time / (stat->no_of_bursts + 1)); // + 1 b/c num_wait = num_run + 1
    start_time = GetCurrentTime();
    start_timer();

	//clock_gettime(CLOCK_MONOTONIC, &curr);
	gettimeofday(&t2,NULL);
	unsigned long t22 = nanos();//1000000 * t2.tv_sec + t2.tv_usec;
        
	if(scheduling_type!=2)
	{
		printf("context switch time: %lu \n",(t22-t11));
		tot_context_switch_tim += (t22-t11);
		context_switch_cnt++;
	}

    siglongjmp(next->jbuf, 1);


    //clock_gettime(CLOCK_MONOTONIC, &curr);
    //printf("context switch time: %lld \n",(curr.tv_sec - begin.tv_sec));
}

void Go()
{
    thread_t *t = GetNextThread();
    current = t;
    t->status->state = RUNNING;
    start_timer();
    start_time = GetCurrentTime();
    siglongjmp(t->jbuf, 1);
    while(1);
}

int GetMyId()
{
    return current->status->id;
}

thread_t *GetThread(int thread_id)
{
    if (thread_list->head == NULL)
    {
        return NULL;
    }

    thread_node_t *node = thread_list->head;
    while(node != NULL && node->thread->status->id != thread_id)
    {
        node = node->next;
    }
    if (node == NULL)
    {
        return NULL;
    }
    return node->thread;
}

int DeleteThread(int thread_id)
{
    int currentId = GetMyId();
    thread_t *t = GetThread(thread_id); // Returns thread structure by ID
    if (t == NULL)
    {
        return -1;
    }

    t->status->state = FINISHED;
    if (thread_id == currentId)
    {
	unsigned long t11 = nanos();
        YieldCPU();
	//printf("Hi\n");
	unsigned long t22 = nanos();
	//if(scheduling_type==2)printf("context switch time: %lu \n",(t22-t11));
    }
    return 0;
}

int RemoveFromList(int thread_id, thread_queue_t *q)
{
    thread_node_t *node = q->head;
    thread_node_t *prev_node = NULL;
    // Find node with corresponding thread_id
    if (!node){
        return -1;
    }
    while(node->next != NULL && node->thread->status->id != thread_id)
    {
        prev_node = node;
        node = node->next;
    }
    // If it is not found return -1
    if(node->thread->status->id != thread_id)
    {
        return -1;
    }
    // Head
    if (node == q->head)
    {
        // If the head is to be deleted.
        q->head = node->next;
    }
    // Tail
    else if (node->next == NULL)
    {
        prev_node->next = NULL;
        q->tail = prev_node;
    }

    // Otherwise in middle / end of list
    else
    {
        prev_node->next = node->next;
    }
    free(node);
    (q->size) --;
    return 0;
}

void YieldCPU()
{
	//printf("Hi\n");
    	raise(SIGVTALRM);
	//printf("Hi\n");	
}

int GetStatus(int thread_id, status_t *status)
{
    thread_t *t = GetThread(thread_id);
    if (t == NULL)
        return -1;
    status->id = t->status->id;
    status->state = t->status->state;
    status->no_of_bursts = t->status->no_of_bursts;
    status->total_exec_time = t->status->total_exec_time;
    status->total_sleep_time = t->status->total_sleep_time;
    status->total_wait_time = t->status->total_wait_time;
    status->avg_exec_time = t->status->avg_exec_time;
    status->avg_wait_time = t->status->avg_wait_time;
    return t->status->id;
}

int SuspendThread(int thread_id)
{
    thread_t *t = GetThread(thread_id);
    if (t == NULL)
        return -1;
    t->status->state = SUSPENDED;
    RemoveFromList(thread_id, ready_list);
    if (current->status->id == thread_id)
        YieldCPU();
    return thread_id;
}

int ResumeThread(int thread_id)
{
    thread_t *t = GetThread(thread_id);
    if (t == NULL)
        return -1;
    if (t->status->state != SUSPENDED)
        return thread_id;
    thread_enqueue(t, ready_list);
    t->status->state = READY;
    return thread_id;
}

unsigned GetCurrentTime()
{
    clock_t time = clock();
    unsigned millis = ((double)(time) / CLOCKS_PER_SEC) * 1000;
    return millis;
}

void SleepThread(int sec)
{
    current->status->state = SLEEPING;
    current->status->wake_time = (GetCurrentTime() + (sec * 1000));
    current->status->total_sleep_time += (sec * 1000);
    RemoveFromList(current->status->id, ready_list);
    YieldCPU();
}

void setup(int schedule)
{
    srand(time(NULL));
    scheduling_type = schedule; //RR=0, LOT == 1, FCFS == 2
    ready_list = malloc(sizeof(thread_queue_t));
    ready_list->head = ready_list->tail = NULL;
    ready_list->size = 0;

    thread_list = malloc(sizeof(thread_queue_t));
    thread_list->head = ready_list->tail = NULL;
    thread_list->size = 0;

    current = NULL;

    signal(SIGVTALRM, Dispatch);
}


void start_timer()
{
    struct itimerval tv;
    tv.it_value.tv_sec = TIME_QUANTUM;     //time of first timer
    tv.it_value.tv_usec = 0;               //time of first timer
    tv.it_interval.tv_sec = 0;             //time of all timers but the first one
    tv.it_interval.tv_usec = 0;            //time of all timers but the first one
    setitimer(ITIMER_VIRTUAL, &tv, NULL);
}


//Get the head of thread queue
thread_t *GetNextThread()
{
	return scheduler();
}

/*Print the status of all threads and kill all threads. 
In other words, capture the last status of all threads.*/

void CleanUp()
{
	
	printf("Global work: %lld \n",global_work);
	printf("tot context switch time: %lld \n",tot_context_switch_tim);
	
	FILE *filePointer ; 

	filePointer = fopen("/home/yuvraj/Desktop/pa2-sms821/Plotting_Script/myFile1", "a");
	tot_context_switch_tim/=context_switch_cnt;
	fprintf(filePointer, "%lld",global_work);
        fputs("\n", filePointer);
	fclose(filePointer);
	


    clean = 1;
    // Print contents of status struct
    thread_node_t *node = thread_list->head;
    while(NULL != node)
    {
        thread_t *t = node->thread;
        status_t *s = t->status;
        printf("thread %d info:\n", s->id);
        switch(s->state)
        {
            case(RUNNING):
                printf("thread state = RUNNING\n");
                break;
            case(READY):
                printf("thread state = READY\n");
                break;
            case(SLEEPING):
                printf("thread state = SLEEPING\n");
                break;
            case(SUSPENDED):
                printf("thread state = SUSPENDED\n");
                break;
            case(FINISHED):
                printf("thread state = FINISHED\n");
                break;
        }
        printf("num_runs = %d\ntotal_exec_time = %d\n"
               "total_sleep_time = %d\ntotal_wait_time = %d\navg_exec_time = %d\navg_wait_time = %d\n", s->no_of_bursts, s->total_exec_time, s->total_sleep_time, s->total_wait_time, s->avg_exec_time, s->avg_wait_time);
        node = node->next;
    }
    // delete all threads
    // free up readyQ
    node = ready_list->head;
    thread_node_t *next;
    while(NULL != node)
    {
        next = node->next;
        RemoveFromList(node->thread->status->id, ready_list);
        node = next;
    }
    free(ready_list);
    //free up threads
    node = thread_list->head;
    while(NULL != node)
    {
        next = node->next;
        thread_t *t = node->thread;
        RemoveFromList(node->thread->status->id, thread_list);
        free(t->stack);
        free(t->status);
        free(t);
        node = next;
    }
    free(thread_list);
    // exit
    exit(0);
}
