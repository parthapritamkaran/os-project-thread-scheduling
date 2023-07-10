#include "threads.h"
thread_t* roundRobin(thread_node_t *node);
thread_t* lottery();
extern int scheduling_type;
extern thread_queue_t *ready_list;

void InsertAtHead(thread_t *t, thread_queue_t *q) {
	thread_node_t *node = malloc(sizeof(thread_node_t));
	node->thread = t;
	node->next = q->head;
	q->head = node;
	if(q->tail == NULL)
		q->tail = node;
	q->size++;
}

void thread_enqueue(thread_t *t, thread_queue_t *q){
	thread_node_t *node = malloc(sizeof(thread_node_t));
	node->thread = t;
	node->next = NULL;
	if (q->tail != NULL)
		q->tail->next = node;
	
	else 
		q->head = node;
	q->tail = node;
	q->size++;
}

thread_t* scheduler() {
	//int number, priority = 0;
	thread_node_t *node;
	if(ready_list->size == 0)
		return NULL;
	switch(scheduling_type)	{
		case 0:          // Round Robin
			//Put your code here
            return roundRobin(node);
		case 1:          // Lottery
			//Put your code here
            return lottery();
		case 2:          //First come first serve
			return ready_list->head->thread;
		default:
			return NULL;
	}
}

// Implementation of Lottery Scheduler Function
// thread_t* lottery(thread_node_t* node) {
thread_t* lottery() {

    thread_node_t* nodeNow;
    thread_node_t* nodeNext;
    thread_t* savedThread;
    int wtNow, wtNext, wtSum, threadNum;

    // Sort the ready queue in increasing order of priority
    nodeNow = ready_list->head;
    nodeNext = ready_list->head->next;
    wtSum = nodeNow->thread->priority;
    while (nodeNext != NULL) {
        wtNow = nodeNow->thread->priority;
        wtNext = nodeNext->thread->priority;
        wtSum += wtNext;
        if (wtNow > wtNext) {
            // Save thread corresponding to nextNode
            savedThread = nodeNext->thread;

            // Delete the nodeNext from the ready queue
            nodeNext = nodeNext->next;
            RemoveFromList(savedThread->status->id, ready_list);

            // Add the removed node to the head of the ready queue
            InsertAtHead(savedThread, ready_list);
        }
        else {
            nodeNow = nodeNow->next;
            nodeNext = nodeNext->next;
        }
    }

    // Generate a random between 1 and wtSum
    threadNum = rand() % wtSum + 1;
    printf("Random number generated is  %d\n", threadNum);
    
    // Again, traverse through the sorted ready list to 
    // determine the interval in which threadNum lies.
    nodeNow = ready_list->head;
    while (nodeNow != NULL) {
        wtNow = nodeNow->thread->priority;
        if (threadNum <= wtNow) {
            printf("Thread scheduled is     %d\n\n", nodeNow->thread->status->id);
            return nodeNow->thread;
        }
        else 
            threadNum -= wtNow;
        nodeNow = nodeNow->next;
    }
    return NULL;
}

// Implementation of Round Robin Scheduler function
thread_t* roundRobin(thread_node_t *node) {
    thread_t* currentThread;
    node = ready_list->head;
    currentThread = node->thread;
    // Remove this thread from the ready queue
    RemoveFromList(currentThread->status->id, ready_list);
    return currentThread;
}

//
    //return NULL;
    
    // Determine the interval to which 
    /*
    // Iterate through the ready list and collect thread weights
    node = thread_list->head;
    while (node != NULL) {
        weights[i] = node->thread->priority;
        wtSum += weights[i];
        node = node->next;
        i++;
    }

    // Generate a random number between 1 and wtSum;
    select = rand() % wtSum + 1;

    // Determine the interval in which select lies
    // i contains the number of threads in the ready queue
    for (j = 0; j < i; j++) {

    }
    */
