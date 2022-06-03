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
    PROC_STATS stats;
} PROC;

typedef struct
{
    PROC data;
    struct QUEUE *next;
} QUEUE;


// GLOBAL VARIABLES
QUEUE ready = {NULL, NULL};
QUEUE io = {NULL, NULL};
QUEUE * q_cpu = &ready;
QUEUE * q_io = &io;

PROC * proc = NULL;
PROC * iodev = NULL;

CPU_STATS cpu_stats = {0,0,0,0};
IO_STATS io_stats = {0,0,0};
int CLOCK = 0;
char* PROC_HEADERS[] = {"Name", "CPU Time", "When Done", "# Dispatches", "Blocked for I/O", "I/O Time"};


// QUEUE FUNCTIONS
bool empty(QUEUE *q);                               // implemented
void addtoqueue(QUEUE *q, PROC *p);                 // implemented
void printqueue(QUEUE *q);                          // implemented
void movetocpu(PROC *p);                            // implemented
void movetoio(PROC *p);                             // implemented

// RUN FUNCTIONS
void runio();                                       // partly implemented *tested only with runfcfs()
void runfcfs();                                     // implemented
void runrr();
void run(char *flag);                               // implemented

// FILE FUNCTIONS
void rfile(char *fname);                            // implemented

// STATISTIC FUNCTIONS
void print_proc_stats(char *name, PROC_STATS stat); // implemented
void print_cpu_stats();                             // implemented
void print_io_stats();                              // implemented

// HELPER FUNCTIONS
bool max(int a, int b);                             // implemented

int main(int argc, char *argv[])
{
    // random seed
    (void)srandom(12345);

    // check that the arguments are right
    if (argc != 3)
    {
        fprintf(stderr, "Usage: <scheduling policy> <file>\n");
        exit(EXIT_FAILURE);
    }

    // open the file and load the process queue
    rfile("file1");

    // run the sim
    run("-f");

    return 0;
}


// QUEUE FUNCTIONS
bool empty(QUEUE *q){
    // determines if queue is empty by checking the head's name
    return (strcmp(q->data.name, "") == 0);
}

void addtoqueue(QUEUE *q, PROC *p){
    // if queue is empty, init the head
    if (empty(q)){
        q->data = *p;
        q->next = NULL;
    }
    else {
    // else, iterate to the end of the queue and add a new node
        QUEUE *t;
        t = (QUEUE *)malloc(sizeof(QUEUE));
        t->data = *p;
        t->next = NULL;
        QUEUE * curr = q;
        
        while (q->next){
            q = q->next;
        }
        q->next = t;
    }
}

void printqueue(QUEUE *q){
    while (q){
        printf("\tNAME: %s RT: %d PROB: %.2f\n", q->data.name, q->data.runtime, q->data.probability);
        q = q->next;
    }
}

void movetocpu(PROC *p){
    addtoqueue(&ready, p);
}

void movetoio(PROC *p){
    addtoqueue(&io, p);
}


// RUN FUNCTIONS
/*
void runio(){
    // determine pause and update relevant data entries
    int pause = (random() % IO_MAX_TIME + MIN_TIME) - 1;
    io.data.stats.iotime = io.data.stats.iotime + pause;
    CLOCK += pause;
    io_stats.calls++;
    //printf("\nIO SERVICE: %d seconds\n", pause);  
    io.data.stats.dispatches = io.data.stats.dispatches + 1;

    // check if process needs to continue running then move to cpu
    movetocpu(&io.data);

    // check if there is an io queue
    if (io.next){
        QUEUE * t = io.next;
        io.data = t->data;
        io.next = t->next;
    }
    else {
        QUEUE p = {NULL, NULL};
        io = p;
    }
    //printf("IO QUEUE:\n");
    //printqueue(&io);
}
*/

void runio(){
        if (iodev){
            printf("\tIO PROCESS NAME: %s, TIME REMAINING: %d\n", iodev->name, iodev->runtime);
            if (iodev->runtime == 0){
                printf("PROCESS %s FINISHED AT %d\n", iodev->name, CLOCK);
            }
        else iodev->runtime--;
    }
}

/*
void runfcfs() {
    QUEUE * q = &ready;

    while (q){
        q->data.stats.dispatches = q->data.stats.dispatches + 1;
        //printf("\nFCFS QUEUE:\n");
        //printf("PROCESS NAME: %s\n", q->data.name);


        bool blocked = false;
        int blockedtime = 0;

        // check if runtime is greater than 2 and determines if process should block
        if (q->data.runtime > 2){
            blocked = ((float)random()/RAND_MAX < q->data.probability);
        }

        // determine blocked time if process blocked
        if (blocked){
            blockedtime = random() % q->data.runtime + MIN_TIME;
            //printf("PROCESS BLOCKED, BT = %d\n", blockedtime);
        }

        int duration = q->data.runtime - blockedtime;
        CLOCK += duration;
        q->data.runtime = q->data.runtime - duration;
        //printf("RUN FOR %d SECONDS\n", duration);
        while (duration >= 0) duration--;

        
        // if process is done, print stats
        if (q->data.runtime <= 1){
            print_proc_stats(q->data.name, q->data.stats);
            q = q->next;
            continue;
        }

        // if process blocked, move to io and run io
        if (blocked) {
            q->data.stats.timesblocked = q->data.stats.timesblocked + 1;
            movetoio(&q->data);
            runio();
        }

        // continue iterating
        q = q->next;
        
    }
}
*/
void runfcfs() {
    if (proc){
        printf("\t***IN FCFS PROCESS NAME: %s, TIME REMAINING: %d\n", proc->name, proc->runtime);
        if (proc->runtime-1 == 0){
            printf("PROCESS %s FINISHED AT %d\n", proc->name, CLOCK);
            proc = NULL;
            q_cpu = q_cpu->next;
        }
        else proc->runtime--;
    }
}

void runrr(){
    QUEUE * q = &ready;

    while (q){
        q->data.stats.dispatches = q->data.stats.dispatches + 1;
        //printf("\nRR QUEUE:\n");
        //printqueue(q);
        //printf("PROCESS NAME: %s\n", q->data.name);
        bool blocked = false;
        int blockedtime = 0;
        int duration = QUANTUM;
        // check if runtime is greater than 2 and determines if process should block
        if (q->data.runtime > 2){
            blocked = ((float)rand()/RAND_MAX < q->data.probability);
        }

        // set duration to the min between duration and runtime
        if (max(duration, q->data.runtime)) duration = q->data.runtime;

        // update relevant variables with duration
        CLOCK += duration;
        q->data.runtime = q->data.runtime - duration;
        while (duration >= 0) duration--;      

        // check if runtime is done, then print stats
        if (q->data.runtime == 0){
            print_proc_stats(q->data.name, q->data.stats);
        } 

        // if there is io blocking, move to io and call the routine
        if (blocked){
            q->data.stats.timesblocked = q->data.stats.timesblocked + 1;
            movetoio(&q->data);
            runio();
        }

        // if the runtime isn't finished and the process isn't blocked, move to end of queue
        if (q->data.runtime != 0 && !blocked) movetocpu(&q->data);

        q = q->next;
    }
}

void run(char *flag){
    printf("Processes:\n");
    for (int i = 0; i < 6; i++){
        printf("%s\t", PROC_HEADERS[i]);
    }
    printf("\n");


    bool blocked = false;
    int blockedtime = 0;
    int ioservice = 0;
    int prevtime = 0;
    if (strcmp(flag, "-f") == 0) {
        // tik 1 - 
        //start loading at tick 2 
        while (q_cpu || q_io){
            CLOCK++;
            
            printf("===TICK %d===\n", CLOCK);
            printf("printing ready queue: \n");
            printqueue(&ready);
            printf("\n");
            printf("printing io queue: \n");
            printqueue(&io);

            if (q_cpu && !proc) {
                proc = &q_cpu->data;
                if (proc) {
                    if (proc->runtime > 2){
                        float blockprob = (float)random()/RAND_MAX;
                        blocked = (blockprob < proc->probability);
                        blockedtime = random() % proc->runtime + MIN_TIME;
                        blockedtime = proc->runtime - blockedtime + 1;
                    }
                }
            }
            
            runfcfs();
            if (proc) {
                if (proc->runtime == blockedtime && blocked) {
                    printf("PROCESS BLOCKING\n");
                    movetoio(proc);
                    proc = NULL;
                    q_cpu = q_cpu->next;
                }
            }
            
            if (q_io && !iodev) {
                iodev = &q_io->data;
                ioservice = (random() % IO_MAX_TIME + MIN_TIME) - 1;
                prevtime = iodev->runtime;
                iodev->runtime = ioservice;
            }

            runio();

            if (iodev) {
                if (iodev->runtime == 0){
                    iodev->runtime = prevtime;
                    movetocpu(iodev);
                    iodev = NULL;
                    q_io = q_io->next;
                }
            }
            //break;
        }


    }
    /*
    else runrr();

    printf("\nSystem:\n");
    printf("The wall clock time at which the simulation finished: %d\n", CLOCK);

    printf("\nCPU:\n");
    print_cpu_stats();

    printf("\nIO:\n");
    print_io_stats();
    */
}


// FILE FUNCTIONS
void rfile(char *fname)
{
    // open file
    FILE *fp = fopen(fname, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Cannot open file %s\n", fname);
        exit(EXIT_FAILURE);
    }

    PROC tmp_proc = {.name = ""};  

    int count = 0;
    while (fscanf(fp, "%s %d %f", tmp_proc.name, &tmp_proc.runtime, &tmp_proc.probability) == 3)
    {
        // convert probabilty to string so that we can check if there is more than 2 decimal places
        char buf[10];
        gcvt(tmp_proc.probability, 4, buf);

        // error checking
        if (strlen(tmp_proc.name) > 10)
        {
            fprintf(stderr, "Name '%s' must be no more than 10 characters\n", tmp_proc.name);
            exit(EXIT_FAILURE);
        }
        else if (tmp_proc.runtime < 1)
        {
            fprintf(stderr, "The run time must be an integer 1 or more.\n");
            exit(EXIT_FAILURE);
        }
        else if (tmp_proc.probability <= 0 || tmp_proc.probability >= 1 || strlen(buf) > 4)
        {
            fprintf(stderr, "The probability must be a decimal number between 0 and 1 with 2 decimal places.\n");
            exit(EXIT_FAILURE);
        }

        tmp_proc.stats.cputime = tmp_proc.runtime;

        // allocate actual node
        PROC *data_ptr = malloc(sizeof *data_ptr);

        // make sure it works
        if (!data_ptr)
        {
            fprintf(stderr, "Malloc error");
            break;
        }

        // assign temporary data to the actual node
        *data_ptr = tmp_proc;

        // add to linked list
        addtoqueue(&ready, data_ptr);
        count++;
        // printf ("%s %d %.2f\n", data_ptr->name, data_ptr->runtime, data_ptr->probability);
    }
    cpu_stats.processes = count;
    if (fp != stdin) fclose (fp); 

    // printqueue(&ready);
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
    printf("Total time spent idle: %d\n", cpu_stats.idle);
    printf("CPU utilization: %.2f\n", ((float)cpu_stats.busy/(float)CLOCK));
    printf("Number of Dispatches: %d\n", cpu_stats.dispatches);
    printf("Overall throughput: %.2f\n", ((float)cpu_stats.processes)/((float)CLOCK));
}

void print_io_stats(){
    printf("Total time spent busy: %d\n", io_stats.busy);
    printf("Total time spent idle: %d\n", io_stats.idle);
    printf("I/O device utilization: %.2f\n", ((float)io_stats.busy/(float)CLOCK));
    printf("Number of times I/O was started: %d\n", io_stats.calls);
    printf("Overall throughput %.2f\n", ((float)io_stats.calls/(float)CLOCK));
}


// HELPER FUNCTIONS
bool max(int a, int b){
    return (a > b) ? true : false;
}