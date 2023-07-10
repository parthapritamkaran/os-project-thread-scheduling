// Project 2: User level thread library
// Test file
#include "threads.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <time.h>


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


int scheduling_type;
extern long long int global_work;
extern long long int tot_context_switch_tim;
extern long long int n_thread;

void counter()
{
  long tid;
  int id = GetMyId();
  int temp;
  // id = syscall(SYS_gettid);
  //int *ar = arr;

  // struct timespec declared in <sys/time.h> and is used to get the elapsed time
  struct timespec begin;
  struct timespec current;

  // clock_gettime is used to retrieve the time of the specified clock clk_id
  // it is stored in struct begin here
  // CLOCK_MONOTONIC: Clock that cannot be set and represents monotonic time 
  // since some unspecified starting point. 
  clock_gettime(CLOCK_MONOTONIC, &begin);

	unsigned long t11 = nanos();
        
 
  long long int rf = 0,i=0;
	while (1)
	{
		rf++;
		i++;

		clock_gettime(CLOCK_MONOTONIC, &current);
		if (i % 10000000 == 0)
        	{
            		id = GetMyId();
            		printf("counter_thread tid: %d \n", id);
            		//j++;
        	}

		// if more than 10 seconds elapse between the current time and start time,
		// break out of the while loop
		if (((double)current.tv_sec - (double)begin.tv_sec) >= 2.0)
		{
			break;
		}
	}

	// the threads have temp_id between 0 and 99
	printf("10 secs done\n");

	temp = (int)id%100;
	//printf("work = %lld\n", rf);
	global_work += rf;
	//printf("time = %lf \n", ((double)current.tv_sec - (double)begin.tv_sec));
	//ar[temp] = rf;
	DeleteThread(GetMyId());

	unsigned long t22 = nanos();
	//if(scheduling_type==2)printf("context switch time: %lu \n",(t22-t11));
   	//pthread_exit(NULL);
}

void sleeping()
{
  long tid;
  int id = GetMyId();;
  //id = syscall(SYS_gettid);
  //int *ar = arr;
  
  int temp;

  struct timespec begin;
  struct timespec current;
  clock_gettime(CLOCK_MONOTONIC, &begin);

  long long int rf = 0;
  while (1)
  {
      rf++;
      clock_gettime(CLOCK_MONOTONIC, &current);
  
      	if((((long long)current.tv_sec - (long long)begin.tv_sec) == 5.0))// && (((long long)current.tv_sec - (long long)begin.tv_sec) <= 5.5))
      	{
		printf("ddddddddddddddddddddddddddddddddddddddddddddddddddddddGoing to sleep\n");
		//sleep(1);
		SleepThread(2);
	}

      if (((long long)current.tv_sec - (long long)begin.tv_sec) >= 10.0)
      {
        break;
      }
  }
    
   // the threads have temp_id between 0 and 99
	printf("10 secs done\n");

	temp = (int)id%100;
	//printf("work = %lld\n", rf);
	global_work += rf;
	//printf("time = %lf \n", ((double)current.tv_sec - (double)begin.tv_sec));
	//ar[temp] = rf;
	DeleteThread(GetMyId());
   	//pthread_exit(NULL);
}

void simple_thread()
{
    int i = 0;
    int j = 0;
    while(1)
    {
        if (i % 100000000 == 0)
        {
            int id = GetMyId();
            printf("simple_thread tid: %d j = %d\n", id, j);
            j++;
        }
        if (j == 10)
        {
            printf("DELETING MYSELF tid: %d\n", GetMyId());
            printf("Size of ready list: %d\n", ready_list->size);
            // Changes the state of the running thread to FINISHED and yields the CPU
            DeleteThread(GetMyId());
        }
        i++;
    }

}
void clean_up_thread()
{
    int i = 0;
    int j = 0;
    while(1)
    {
        if (i % 100000000 == 0)
        {
            int id = GetMyId();
            printf("simple_thread tid: %d j = %d\n", id, j);
            j++;
        }
        if (j == 10)
        {
            printf("CLEANUP()\n");
            CleanUp();
        }
        i++;
    }

}
void sleep_test()
{
    int i = 0;
    int j = 0;
    int sleep = 0;
    while(1)
    {
        if (sleep == 1)
        {
            printf("WAKING UP\n");
            sleep = 0;
        }
        if (i % 100000000 == 0)
        {
            int id = GetMyId();
            printf("sleep_thread tid: %d j = %d\n", id, j);
            j++;
        }
        if (j == 5)
        {
            j++;
            printf("SLEEP FOR 20 SECONDS (tid: %d)\n", GetMyId());
            sleep = 1;
            SleepThread(20);
        }
        i++;
    }
}

void suspend_test()
{
    int i = 0;
    int j = 0;
    while(1)
    {
        if (i % 100000000 == 0)
        {
            int id = GetMyId();
            printf("suspend_test tid: %d j = %d\n", id, j);
            j++;
        }
        if (j == 10)
        {
            j++;
            printf("SUSPEND THREAD 0\n");
            SuspendThread(0);
        }
        if (j == 20)
        {
            j++;
            printf("RESUME THREAD 0\n");
            ResumeThread(0);
        }
        i++;
    }
}

void yield_test()
{
    int i = 0;
    int j = 0;
    while(1)
    {
        if (i % 100000000 == 0)
        {
            int id = GetMyId();
            printf("yield_test tid: %d j = %d\n", id, j);
            j++;
        }
        if (j % 5 == 0)
        {
            j++;
            printf("YIELDING CPU tid: %d\n", GetMyId());
            YieldCPU();
        }
        i++;
    }
}

void delete_test()
{
    int i = 0;
    int j = 0;
    while(1)
    {
        if (i % 100000000 == 0)
        {
            int id = GetMyId();
            printf("delete_test tid: %d j = %d\n", id, j);
            j++;
        }
        if (j % 25 == 0)
        {
            j++;
            printf("DELETING MYSELF tid: %d\n", GetMyId());
            DeleteThread(GetMyId());
        }
        i++;
    }
}

void delete_other_test()
{
    int i = 0;
    int j = 0;
    while(1)
    {
        if (i % 100000000 == 0)
        {
            int id = GetMyId();
            printf("delete_other_test tid: %d j = %d\n", id, j);
            j++;
        }
        if (j == 20)
        {
            j++;
            printf("DELETING THREAD 1\n");
            DeleteThread(1);
        }
        i++;
    }
}

void create_test()
{
    int i = 0;
    int j = 0;
    while(1)
    {
        if (i % 100000000 == 0)
        {
            int id = GetMyId();
            printf("create_test tid: %d j = %d\n", id, j);
            j++;
        }
        if (j == 30)
        {
            j++;
            printf("CREATING A NEW THREAD\n");
            CreateThread(simple_thread, 1); 
        }
        i++;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s [RR] | [LOT] | [FCFS] \n", argv[0]);
        return 1;
    }
    else if (strcmp(argv[1], "RR") == 0)
    {
	scheduling_type=0;
        setup(0); // RR
    }
    else if (strcmp(argv[1], "LOT") == 0)
    {
	scheduling_type=1;
        setup(1); // LOT
    }
    else if (strcmp(argv[1], "FCFS") == 0)
    {
	scheduling_type=2;
        setup(2); // FCFS
    }	
    else
    {
        return 1;
    }
	global_work = 0;
	tot_context_switch_tim = 0;
	n_thread = 60;
	//CreateThread(simple_thread, 1);
	//CreateThread(simple_thread, 1);
//CreateThread(simple_thread, 1);
//CreateThread(simple_thread, 1);
	//CreateThread(counter, 1);
	//CreateThread(counter, 1);
	for(int i=0;i<n_thread;i++)CreateThread(counter, 1);
	//CreateThread(sleeping, 1);
    //CreateThread(simple_thread, 2);        // 0
    //CreateThread(simple_thread, 1);        // 1 
    //CreateThread(simple_thread, 3);        // 0
    //CreateThread(simple_thread, 4);        // 1 
    //CreateThread(simple_thread, 5);        // 0
    //CreateThread(simple_thread, 1);        // 1 
    //CreateThread(clean_up_thread, 1);        // 2

    Go();

    return 0;
}
