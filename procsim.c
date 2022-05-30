#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAXLENGTH 10

/*
PROC *cpu; // points to process on cpu
PROC *iodev; // points to process on io device
QUEUE ready = { NULL, NULL }; // ready queue
QUEUE io = { NULL, NULL }; // i/o queue
PROC *palloc(char *nm, int rt, float pr);
void addtoqueue(QUEUE *q, PROC *p);
void movetocpu(PROC *p);
void movetoiodev(PROC *p);
void runfcfs(void);
void runio(void);
void run();
void rfile(char *fname, FILE *fp);
*/

typedef struct
{
    char name[MAXLENGTH];
    int runtime;
    float probability;
    bool blocked;
    int blockruntime;
} PROC;

typedef struct
{
    PROC data;
    struct QUEUE *next;
} QUEUE;

QUEUE ready = {NULL, NULL};
QUEUE io = {NULL, NULL};

bool empty(QUEUE *q);                   // implemented
void addtoqueue(QUEUE *q, PROC *p);     // implemented
void printqueue(QUEUE *q);              // implemented
void movetocpu(PROC *p);                // implemented
void movetoio(PROC *p);                 // implemented
void runfcfs();
void runio();
void run();
void rfile(char *fname);                // implemented

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
    rfile(argv[2]);

    return 0;
}

bool empty(QUEUE *q){
    return (strcmp(q->data.name, "") == 0);
}

void addtoqueue(QUEUE *q, PROC *p){
    QUEUE *t;
    t = (QUEUE *)malloc(sizeof(QUEUE));
    t->data = *p;

    if (empty(q)){
        q->data = *p;
        q->next = NULL;
    }
    else {
        QUEUE * curr = q;
        while (curr->next){
            curr = curr->next;
        }
        curr->next = t;
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
        // printf ("%s %d %.2f\n", data_ptr->name, data_ptr->runtime, data_ptr->probability);
    }

    if (fp != stdin) fclose (fp); 

    //printqueue(&ready);

    movetoio(&ready.data);
    QUEUE * t = ready.next;
    movetoio(&t->data);

    printqueue(&io);
}


