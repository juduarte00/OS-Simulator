#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


// GLOBAL CONSTANTS
#define MAXLENGTH 10
#define IO_MAX_TIME 30
#define MIN_TIME 1
#define QUANTUM 5


// STRUCTURES
typedef struct {
    int cputime;
    int dispatches;
    int timesblocked;
    int iotime;
} PROC_STATS;

typedef struct {
    int busy;
    int idle;
    int dispatches;
    int processes;
} CPU_STATS;

typedef struct {
    int busy;
    int idle;
    int calls;
} IO_STATS;

typedef struct
{
    char name[MAXLENGTH];
    int runtime;
    float probability;
    int remainingtime;
    int iotime;
    PROC_STATS stats;
} PROC;

// typedef struct
// {
//     PROC data;
//     struct QUEUE *next;
// } QUEUE;

struct node {
    PROC* value;
    struct node* next;
};

struct queue {
    int count;
    struct node* head;
    struct node* back;
};

typedef struct queue* QUEUE;

// GLOBAL VARIABLES
QUEUE ready = NULL;
QUEUE io = NULL;
QUEUE q_cpu = NULL;
QUEUE q_io = NULL;

PROC * proc = NULL;
PROC * iodev = NULL;

CPU_STATS cpu_stats = {0,0,0,0};
IO_STATS io_stats = {0,0,0};
int CLOCK = 0;
char* PROC_HEADERS[] = {"Name", "CPU Time", "When Done", "# Dispatches", "Blocked for I/O", "I/O Time"};

bool debug = false;
bool debug_random = false;

// QUEUE FUNCTIONS
QUEUE queue_create(void);
int queue_destroy(QUEUE queue);
int queue_enqueue(QUEUE queue, void *data);
int queue_dequeue(QUEUE queue, void **data);
int queue_delete(QUEUE queue, void *data);
typedef int (*queue_func_t)(QUEUE queue, void *data, void *arg);
int queue_iterate(QUEUE queue, queue_func_t func, void *arg, void **data);
int queue_length(QUEUE queue);
void movetocpu(PROC *p);                            // implemented
void movetoio(PROC *p);                             // implemented

// RUN FUNCTIONS
void runio();                                       // partly implemented *tested only with runfcfs()
void runfcfs();                                     // implemented
void runrr();
void run(char *flag);                               // implemented

// INPUT FUNCTIONS
void rfile(char *fname);                            // implemented
void rflag(char *flag);

// STATISTIC FUNCTIONS
void print_proc_stats(char *name, PROC_STATS stat); // implemented
void print_cpu_stats();                             // implemented
void print_io_stats();                              // implemented

// HELPER FUNCTIONS
bool max(int a, int b);                             // implemented
int min(int a, int b);
void initQueues(void);



int main(int argc, char *argv[])
{
    // enable print debug statements
    debug = false;
    debug_random = false;

    // random seed
    (void)srandom(12345);

    // initialize queues
    initQueues();

    // check that the arguments are right
    if (argc != 3)
    {
        fprintf(stderr, "Usage: <scheduling policy> <file>\n");
        exit(EXIT_FAILURE);
    }

    // check inputs then open the file and load the process queue
    rflag(argv[1]);
    rfile(argv[2]);

    // run the sim
    
    run(argv[1]);

    return 0;
}


// QUEUE FUNCTIONS

QUEUE queue_create(void) {
    // Initialize queue by setting up the head/tail and start counting number elements
    struct queue* newQueue = (struct queue*)malloc(sizeof(struct queue));
    // if(newQueue == NULL)
    // return NULL;
    newQueue->head = NULL;
    newQueue->back = NULL;
    newQueue->count = 0;
    return newQueue;
}
int queue_destroy(QUEUE queue) {
    // Confirm whether the queue is empty before deletion
    if (queue->head == NULL && queue->back == NULL) {
        free(queue);
        return 0;
    }
    return -1;
}
int queue_enqueue(QUEUE queue, void *data) {
    struct node* newNode;
    PROC* newVal;
    if(queue == NULL || data == NULL)
        return -1;

    newVal= data;
    newNode = (struct node*)malloc(sizeof(struct node));
    if(newNode == NULL)
        return -1;
    newNode->next = NULL;
    newNode->value = newVal;
    // If the queue is empty, the enqueued value becomes the head
    if(queue->head == NULL) {
        queue->head = newNode;
        queue->back = newNode;
        queue->count++;
    } else { // Else, the new value is added to the end of the queue to implement FIFO
        queue->back->next = newNode;
        queue->back = queue->back->next;
        queue->count++;
    }
    //printf("enqueue count: %d\n", queue->count);
    return 0;
}
int queue_dequeue(QUEUE queue, void **data) {
    if(queue == NULL || queue->count == 0 || data == NULL)
        return -1;
    // If there is only one element in the queue, then dequeue it and ensure bothhead and back are null
    if(queue->head == queue->back) {
        *data = queue->head->value;
        queue->head = NULL;
        queue->back = NULL;
        queue->count--;
    }
    else { // Else, take remove the value of the head and shift the head to the next node
        *data = queue->head->value;
        queue->head = queue->head->next;
        queue->count--;
    }
    return 0;
}
int queue_delete(QUEUE queue, void* data) {
    if(queue == NULL || data == NULL)
        return -1;
    struct node* currentNode = queue->head;
    // If the beginning of the queue is the item to delete, reassign head
    if(currentNode->value == data) {
        queue->head = queue->head->next;
        free(currentNode);
        queue->count--;
        return 0;
    }
    else { // Else check whether the next nodes match
        while(currentNode->next != NULL) {
            if(currentNode->next->value == data) {
                currentNode->next = currentNode->next->next;
                queue->count--;
                currentNode = NULL;
                free(currentNode);
                return 0;
            } else {
                // Check whether to keep going through the queue
                if(currentNode->next->next != NULL) 
                    currentNode = currentNode->next;
                else 
                    break;
            }
        }
    }
    currentNode = NULL;
    free(currentNode);
    return -1;
}
int queue_iterate(QUEUE queue, queue_func_t func, void* arg, void** data) {
    if(queue == NULL || func == NULL || queue->head == NULL)
        return -1;
    struct node* currentNode = queue->head;
    struct node* nextNode = queue->head->next;
    // Run the function on the head; if a 1 is returned (TRUE), then data should not be null
    // Store returned value in data
    if(func(queue, currentNode->value, arg)) {
        if(data != NULL) {
            *data = currentNode->value;
            //printf("data: ", data->value.runtime);
            return 0;
        }
    }
    // Check if there is more than one node present in your queue 
    if(nextNode == NULL) 
            return 0;
        
    currentNode = nextNode;
    nextNode = nextNode->next;
    while (currentNode != NULL) {
        if (func(queue, currentNode->value, arg)) {
            if (data != NULL) {
            *data = currentNode->value;
            return 0;
            }
        }
    // Reassign current node if it was deleted
    if(currentNode->value == NULL) {
        currentNode = nextNode;
        nextNode = nextNode->next;
    }
    currentNode = currentNode->next;
    if(nextNode->next != NULL)
        nextNode = nextNode->next;
    }
    return 0;
}

int queue_length(QUEUE queue) {
    if (queue == NULL)
        return -1;
    return queue->count;
}

int printqueue(QUEUE queue){
    if (queue->head == NULL) {
        if (debug) printf("\tqueue is empty\n"); 
        return -1;
    }
    struct node* currentNode = queue->head;
    struct node* nextNode = queue->head->next;
    for (int i = 0; i < queue->count; i++){
        if (debug) printf("\tNAME: %s RT: %d PROB: %.2f REMAINING: %d\n", currentNode->value->name, currentNode->value->runtime, currentNode->value->probability, currentNode->value->remainingtime);
        if (i != queue->count - 1) {
            currentNode = nextNode;
            nextNode = nextNode->next;
        }
    
    }
    return 0; 
}


void movetocpu(PROC *p){
    if (debug)  printf("MOVING PROCESSS %s TO CPU\n", p->name);
    p->stats.dispatches++;
    cpu_stats.dispatches++;
    queue_enqueue(q_cpu, p);
    queue_delete(ready, p);
}

void movetoio(PROC *p){
    if (debug) printf("MOVING PROCESSS %s TO IO\n", p->name);
    p->stats.timesblocked++;
    io_stats.calls++;
    queue_enqueue(q_io, p);
    queue_delete(q_cpu, p);
}


// RUN FUNCTIONS


void runio(){
    
    if (!iodev && !q_io){
        if (debug) printf("VIEW: both I/O device and I/O queue empty; leaving function for I/O queue\n");
        return;
    }

    if (iodev){
        if (iodev->iotime == 0){
            if (debug) printf("PROCESS %s FINISHED AT %d\n", iodev->name, CLOCK);
            queue_delete(q_io, iodev);
            iodev = NULL;
            //queue_enqueue(ready, iodev);
        } else {
            iodev->iotime--;
            
        }
        if (debug) printf("\tIO PROCESS NAME: %s, IO TIME REMAINING: %d\n", iodev->name, iodev->iotime);
        io_stats.busy++;

    }
}

void runfcfs() {
    if (debug) printf("RUN FCFS ____________\n");
    if (proc){
        if (proc->remainingtime > 0) proc->remainingtime--;
        if (proc->remainingtime  == 0){
            if (debug) printf("PROCESS %s FINISHED AT %d\n", proc->name, CLOCK);
            printf("%-10s %6d     %6d    %6d    %6d    %6d\n", proc->name, 
                        proc->stats.cputime, CLOCK, proc->stats.dispatches, proc->stats.timesblocked, proc->stats.iotime);
            queue_delete(q_cpu, proc);
            proc = NULL;
            return;
            // q_cpu = (*q_cpu)->head->next;
        }
        if (debug) printf("\tIN FCFS PROCESS NAME: %s, TIME REMAINING: %d\n", proc->name, proc->remainingtime);
    }
}
void runrr() {

    if (debug) printf("RUN FCFS ____________\n");
    if (proc){
        if (proc->remainingtime > 0) proc->remainingtime--;
        if (proc->remainingtime  == 0){
            if (debug) printf("PROCESS %s FINISHED AT %d\n", proc->name, CLOCK);

            



            printf("%-10s %6d     %6d    %6d    %6d    %6d\n", proc->name, 
                        proc->stats.cputime, CLOCK, proc->stats.dispatches, proc->stats.timesblocked, proc->stats.iotime);
            
            queue_delete(q_cpu, proc);
            proc = NULL;
            
            return;
            // q_cpu = (*q_cpu)->head->next;
        }
        if (debug) printf("\tIN FCFS PROCESS NAME: %s, TIME REMAINING: %d\n", proc->name, proc->remainingtime);
    }
    }


// void runrr(){
//     QUEUE * q = &ready;

//     while (q){
//         q->data.stats.dispatches = q->data.stats.dispatches + 1;
//         //printf("\nRR QUEUE:\n");
//         //printqueue(q);
//         //printf("PROCESS NAME: %s\n", q->data.name);
//         bool blocked = false;
//         int blockedtime = 0;
//         int duration = QUANTUM;
//         // check if runtime is greater than 2 and determines if process should block
//         if (q->data.runtime > 2){
//             blocked = ((float)rand()/RAND_MAX < q->data.probability);
//         }

//         // set duration to the min between duration and runtime
//         if (max(duration, q->data.runtime)) duration = q->data.runtime;

//         // update relevant variables with duration
//         CLOCK += duration;
//         q->data.runtime = q->data.runtime - duration;
//         while (duration >= 0) duration--;      

//         // check if runtime is done, then print stats
//         if (q->data.runtime == 0){
//             print_proc_stats(q->data.name, q->data.stats);
//         } 

//         // if there is io blocking, move to io and call the routine
//         if (blocked){
//             q->data.stats.timesblocked = q->data.stats.timesblocked + 1;
//             movetoio(&q->data);
//             runio();
//         }

//         // if the runtime isn't finished and the process isn't blocked, move to end of queue
//         if (q->data.runtime != 0 && !blocked) movetocpu(&q->data);

//         q = q->next;
//     }
// }

void run(char *flag){
    printf("Processes:\n\n");
    printf("   name     CPU time  when done  cpu disp  i/o disp  i/o time\n");
    if (debug) {
        for (int i = 0; i < 6; i++){
            printf("%s\t", PROC_HEADERS[i]);
        }
        printf("\n");
    }

    bool blocked = false;
    int blockedtime = 0;
    int ioservice = 0;
    int prevtime = 0;
    if (strcmp(flag, "-f") == 0) {
        while (q_cpu->head != NULL || q_io->head != NULL || ready->head != NULL){
            CLOCK++;
            if (debug) printf("===TICK %d===\n", CLOCK);
            if (debug) printf("printing ready queue: \n");
            printqueue(ready);
            if (debug) printf("printing io queue: \n");
            printqueue(q_io);
            if (debug) printf("\n");
            if (debug) printf("RUN READY QUEUE --------------------------------------------\n");
            bool firstload = false;
            bool skipIO = false;
            // RUN READY QUEUE
             if (ready->head == NULL && q_cpu->head == NULL) {
                 if (debug) printf("VIEW: both ready queue and cpu queue empty; leaving function for cpu\n");
            } else {
                if (!proc) {
                     
                    firstload = true;
                    if (ready->head != NULL) {
                        proc = ready->head->value;
                        if (proc->remainingtime == 0)
                            firstload = false;
                        movetocpu(proc);
                        
                    } else {
                        if (q_io->head != NULL) {
                            proc = q_io->head->value;
                            movetocpu(proc);
                        }

                    }
                    if (proc->remainingtime >= 2){
                        float r = random();
                        if (debug_random) printf("%d: %d, f\n", CLOCK, (int)r);
                        float blockprob = r/RAND_MAX;
                        if (debug) printf("block probability value used: %f\n", blockprob);
                        blocked = (blockprob < proc->probability);
                        if (blocked) {
                            int ran = random();
                            if (debug_random) printf("%d: %d, %d, i\n", CLOCK, ran, proc->remainingtime);
                            blockedtime = ran % proc->remainingtime + MIN_TIME;
                        }
                        else blockedtime = 0;
                        blockedtime = proc->remainingtime - blockedtime + 1;
                        if (debug) printf("block time value used: %d\n", blockedtime);
                        
                    }
                } 
                
            }
            
            if (firstload == false){
                runfcfs();
            }
            

            bool sametick = false;
            if (proc) {
                
                if (proc->remainingtime == blockedtime && blocked) {
                    if (debug) printf("PROCESS BLOCKING\n");
                    proc->remainingtime--;
                    movetoio(proc);
                    proc = NULL;
                    sametick = true; 
                    //q_cpu = (*q_cpu)->head->next;
                }
            }

            // RUN IO QUEUE
             if (debug) printf("RUN I/O QUEUE --------------------------------------------\n");
    
            if ((q_io->head != NULL) && !iodev) {
                if (sametick == false || firstload == true) {
                    iodev = q_io->head->value;
                    if (iodev->remainingtime == 0) {
                        ioservice = 1;
                        iodev->stats.iotime += ioservice;
                        if (debug) printf("remaining time is 0 so io service time = %d \n", ioservice);
                    } else {
                        if (debug) printf("remaining run time %d > 30 \n", iodev->remainingtime);
                        //ioservice = (random() % (IO_MAX_TIME - iodev->remainingtime + 1)) + iodev->runtime;
                        int r = random();
                        if (debug_random) printf("%d: %d, i\n", CLOCK, r);
                        ioservice = (r % (IO_MAX_TIME)) + 1;
                        iodev->stats.iotime += ioservice;
                        if (sametick && proc) {
                            ioservice--;
                            io_stats.busy++;
                        }
                        else if (sametick) skipIO = true;
                        if (debug) printf("io service time: %d \n", ioservice);
                    }
                    // ioservice = (random() % (IO_MAX_TIME - MIN_TIME + 1)) + MIN_TIME;
                    // ioservice = (random() % (IO_MAX_TIME - MIN_TIME + 1)) + MIN_TIME;
                    // printf("io service time: %d \n", ioservice);
                    prevtime = iodev->remainingtime;
                    iodev->iotime = ioservice;
                    // prevtime = iodev->runtime;
                    // iodev->runtime = ioservice;
                }
            }
            if (!iodev) io_stats.idle++;


            // if (iodev) {
            //     if (iodev->remainingtime == 0) { 
            //     ioservice = 1;
            //     printf("remaining time is 0 so io service time = %d \n", ioservice);
            //     iodev->iotime = ioservice;
            //     }
            // }

            if ((iodev) && !skipIO)   
                runio();

            if (iodev) {
                if (iodev->iotime == 0){
                    // iodev->iotime = prevtime;
                    // // move back to ready queue
                    queue_enqueue(ready, iodev);
                    queue_delete(q_io, iodev); 
                    iodev = NULL;
                    //q_io = q_io->head->next;
                }
            }

            // break;


        }
    }
    else {
        while (q_cpu->head != NULL || q_io->head != NULL || ready->head != NULL){
            CLOCK++;
            if (debug) printf("===TICK %d===\n", CLOCK);
            if (debug) printf("printing ready queue: \n");
            printqueue(ready);
            if (debug) printf("printing io queue: \n");
            printqueue(q_io);
            if (debug) printf("\n");
            if (debug) printf("RUN READY QUEUE --------------------------------------------\n");
            bool firstload = false;
            bool skipIO = false;
            // RUN READY QUEUE
             if (ready->head == NULL && q_cpu->head == NULL) {
                 
                 if (debug) printf("VIEW: both ready queue and cpu queue empty; leaving function for cpu\n");
            } else {
                if (!proc) {
                     
                    firstload = true;
                    if (ready->head != NULL) {
                        proc = ready->head->value;
                        if (proc->remainingtime == 0)
                            firstload = false;
                        movetocpu(proc);
                        
                    } else {
                        if (q_io->head != NULL) {
                            proc = q_io->head->value;
                            movetocpu(proc);
                        }

                    }
                    
                    if (proc->remainingtime >= 2){
                       
                        float r = random();
                        if (debug_random) printf("%d: %d, f\n", CLOCK, (int)r);
                        float blockprob = r/RAND_MAX;
                        if (debug) printf("block probability value used: %f\n", blockprob);
                        blocked = (blockprob < proc->probability);
                        
                            int ran = random();
                            if (debug_random) printf("%d: %d, %d, i\n", CLOCK, ran, proc->remainingtime);
                            int min_time = min(proc->remainingtime, QUANTUM);
                            blockedtime = ran % min_time + MIN_TIME;

                            
                        
                        blockedtime = proc->remainingtime - blockedtime + 1;
                        if (debug) printf("block time value used: %d\n", blockedtime);
                        
                    }

                    if (proc->remainingtime < QUANTUM && proc->remainingtime == 1) {
                        proc->stats.dispatches++;
                        cpu_stats.dispatches++;
                    }

                }
                 
                
            }
            
            if (firstload == false){
                runrr();
            }
            

            bool sametick = false;
            if (proc) {
                
                if (proc->remainingtime == blockedtime) {
                    if (debug) printf("PROCESS BLOCKING\n");
                    proc->remainingtime--;
                    movetoio(proc);
                    proc = NULL;
                    sametick = true; 
                    //q_cpu = (*q_cpu)->head->next;
                }
            }

            // RUN IO QUEUE
             if (debug) printf("RUN I/O QUEUE --------------------------------------------\n");
    
            if ((q_io->head != NULL) && !iodev) {
                if (sametick == false || firstload == true) {
                    iodev = q_io->head->value;
                    if (iodev->remainingtime == 0) {
                        ioservice = 1;
                        iodev->stats.iotime += ioservice;
                        if (debug) printf("remaining time is 0 so io service time = %d \n", ioservice);
                    } else {
                        if (debug) printf("remaining run time %d > 30 \n", iodev->remainingtime);
                        //ioservice = (random() % (IO_MAX_TIME - iodev->remainingtime + 1)) + iodev->runtime;
                        int r = random();
                        if (debug_random) printf("%d: %d, i\n", CLOCK, r);
                        ioservice = (r % (IO_MAX_TIME)) + 1;
                        iodev->stats.iotime += ioservice;
                        if (sametick && proc) {
                            ioservice--;
                            io_stats.busy++;
                        }
                        else if (sametick && !proc) skipIO = false;
                        if (debug) printf("io service time: %d \n", ioservice);
                    }
                    // ioservice = (random() % (IO_MAX_TIME - MIN_TIME + 1)) + MIN_TIME;
                    // ioservice = (random() % (IO_MAX_TIME - MIN_TIME + 1)) + MIN_TIME;
                    // printf("io service time: %d \n", ioservice);
                    prevtime = iodev->remainingtime;
                    iodev->iotime = ioservice;
                    // prevtime = iodev->runtime;
                    // iodev->runtime = ioservice;
                }
            }
            if (!iodev) io_stats.idle++;


            // if (iodev) {
            //     if (iodev->remainingtime == 0) { 
            //     ioservice = 1;
            //     printf("remaining time is 0 so io service time = %d \n", ioservice);
            //     iodev->iotime = ioservice;
            //     }
            // }
     
            if ((iodev != NULL) && !skipIO)   
                runio();

            if (iodev) {
                if (iodev->iotime == 0){
                    // iodev->iotime = prevtime;
                    // // move back to ready queue
                    queue_enqueue(ready, iodev);
                    queue_delete(q_io, iodev); 
                    iodev = NULL;
                    //q_io = q_io->head->next;
                }
            }

            // break;


        }
    }
    
    // else runrr();

    printf("\nSystem:\n");
    printf("The wall clock time at which the simulation finished: %d\n", CLOCK);

    printf("\nCPU:\n");
    print_cpu_stats();

    printf("\nI/O device:\n");
    print_io_stats();
    
}


// FILE FUNCTIONS
void rfile(char *fname)
{
    // open file
    FILE *fp = fopen(fname, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "%s: No such file or directory\n", fname);
        exit(EXIT_FAILURE);
    }

    PROC tmp_proc = {.name = ""};  

    int count = 0;
    
    int ret = fscanf(fp, "%s %d %f", tmp_proc.name, &tmp_proc.runtime, &tmp_proc.probability);

    while (ret >= 0)
    {

        // error checking
        if (ret < 3){
            fprintf(stderr, "Malformed line %s(%d)\n", fname, count+1);
            exit(EXIT_FAILURE);
        }
        if (strlen(tmp_proc.name) > 10)
        {
            fprintf(stderr, "name is too long %s(%d)\n", fname, count+1);
            exit(EXIT_FAILURE);
        }
        else if (tmp_proc.runtime < 1)
        {
            fprintf(stderr, "runtime is not positive integer %s(%d)\n", fname, count+1);
            exit(EXIT_FAILURE);
        }
        else if (tmp_proc.probability <= 0 || tmp_proc.probability >= 1)
        {
            fprintf(stderr, "probability < 0 or > 1 %s(%d)\n", fname, count+1);
            exit(EXIT_FAILURE);
        }

        tmp_proc.stats.cputime = tmp_proc.runtime;
        cpu_stats.busy += tmp_proc.runtime;
        tmp_proc.remainingtime = tmp_proc.runtime;

        // allocate actual node
        PROC *data_ptr = malloc(sizeof (*data_ptr));

        // make sure it works
        if (!data_ptr)
        {
            fprintf(stderr, "Malloc error");
            break;
        }

        // assign temporary data to the actual node
        *data_ptr = tmp_proc;

        // add to ready queue
        queue_enqueue(ready, data_ptr);
        count++;
        //printf ("%s %d %.2f\n", data_ptr->name, data_ptr->runtime, data_ptr->probability);

        ret = fscanf(fp, "%s %d %f", tmp_proc.name, &tmp_proc.runtime, &tmp_proc.probability);
    }
    cpu_stats.processes = count;
    if (fp != stdin) fclose (fp); 
    if (debug) printf("printing from read: \n");
    if (debug) printqueue(ready);
    // printf("what\n "); 
    // printqueue(io);
}

void rflag(char *flag){
    if ((strcmp(flag, "-f") != 0) && (strcmp(flag, "-r") != 0)){
        fprintf(stderr, "Usage: ./prsim [-r | -f] file\n");
        exit(EXIT_FAILURE);
    }
}


// STATISTIC FUNCTIONS
void print_proc_stats(char *name, PROC_STATS stats){
    printf("%*s", (int)-strlen(PROC_HEADERS[0]), name);
    if (strlen(name) < 8) printf("\t");
    printf("%*d\t", (int)strlen(PROC_HEADERS[1]), stats.cputime);
    printf("%*d\t", (int)strlen(PROC_HEADERS[2]), CLOCK);
    printf("%*d\t", (int)strlen(PROC_HEADERS[3]), stats.dispatches);
    printf("%*d\t", (int)strlen(PROC_HEADERS[4]), stats.timesblocked);
    printf("%*d\t", (int)strlen(PROC_HEADERS[5]), stats.iotime);
    printf("\n\n");
    
    cpu_stats.dispatches += stats.dispatches;
    cpu_stats.busy += stats.cputime;
    cpu_stats.idle += stats.iotime;

    io_stats.busy += stats.iotime;
    io_stats.idle += stats.cputime;
}

void print_cpu_stats(){
    printf("Total time spent busy: %d\n", cpu_stats.busy);
    printf("Total time spent idle: %d\n", CLOCK - cpu_stats.busy);
    printf("CPU utilization: %.2f\n", ((float)cpu_stats.busy/(float)CLOCK));
    printf("Number of dispatches: %d\n", cpu_stats.dispatches);
    printf("Overall throughput: %.2f\n", ((float)cpu_stats.processes)/((float)CLOCK));
}

void print_io_stats(){
    printf("Total time spent busy: %d\n", io_stats.busy);
    printf("Total time spent idle: %d\n", CLOCK - io_stats.busy);
    printf("I/O utilization: %.2f\n", ((float)io_stats.busy/(float)CLOCK));
    printf("Number of dispatches: %d\n", io_stats.calls);
    printf("Overall throughput: %.2f\n", ((float)cpu_stats.processes/(io_stats.idle + io_stats.busy)));
}


// HELPER FUNCTIONS
bool max(int a, int b){
    return (a > b) ? true : false;
}

int min(int a, int b){
    return (a > b) ? b : a;
}

void initQueues(void){
    ready = queue_create();
    //io = queue_create();
    q_cpu = queue_create();
    q_io = queue_create();
    //q_cpu = &ready;
    //q_io = &io;
}