#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAXLENGTH 10

/* reference for linked list struct and corresponding push/print functions: http://www.mahonri.info/SO/23279119_LinkedList_101.c*/

// struct for keeping track of input data
typedef struct {
    char name[MAXLENGTH];
    int runtime;
    float probability;
}input_data; 

// struct for linked list -> possibly ready queue? 
typedef struct {
    input_data data;
    struct linkedlist *next;
}linkedlist;


// TODO: create linked list I/O queue 
    // the next request to execute is the first one in the queue.
    // The amount of time needed to service this request is to be generated randomly and be an integer between 1 and 30 inclusive.
    // Once the I/O request is started, the entire time needed to service it is dedicated to that request. 
    // So, when dispatching the next I/O request, you only need decide how long it will take to complete. 
    // When the request completes, the job moves to the end of the ready queue


// TODO: create a structure to record information about the CPU

// TODO: create a structure to record information about the I/O device.

// CPU dispatch routine
// TODO: figure out parameters and complete 
void dispatch(const char *policy, linkedlist *head){
    linkedlist *current = head;
    (void) srandom(12345);
    printf("random num: %d\n", srandom); 
    // 1. determine whether process is to block for I/O (and is to be transferred to the I/O queue):
        // To do this, generate a random number between 0 and 1.
        // Then compare it to the probability that the process will block
        // If the number you generated is less than the input probability, the process blocks; otherwise not.
}

// TODO: figure out parameters and complete
void FCFS (bool doesBlock) {
    // if the process does not block, it will run until it finishes
    // If the process is to block, you must then decide how long it will 
    // run before blocking (again, using a random number generator).
}

// function for pushing input data into a linked list
void push (linkedlist **head, input_data data){
    linkedlist *newNode = NULL;
    newNode = malloc(sizeof(*newNode));
    if (!newNode) {
        fprintf(stderr, "Malloc error");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;

    if(*head == NULL)
         *head = newNode;
    //Otherwise, find the last node and add the newNode
    else {
        linkedlist * current = *head;
        
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newNode;
    }
    
}

// function for printing linked list (debugging purposes)
int PrintList(linkedlist *head) {
   int rCode=0;
   linkedlist *cur = head;
   int nodeCnt=0;

   while(cur) {
      ++nodeCnt;
      printf("%s %d %.2f\n", cur->data.name, cur->data.runtime, cur->data.probability);
      cur=cur->next;
    }
    printf("%d nodes printed.\n", nodeCnt);
    return(rCode);
   }



int main( int argc, char *argv[] ) {
    // check that the arguments are right 
    if (argc != 3) {
        fprintf(stderr, "Usage: <scheduling policy> <file>\n");
        exit(EXIT_FAILURE);
    } 

    // open file
    FILE* fp = fopen(argv[2], "r");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open file %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    // initialiaze temp data 
    input_data temp_data = { .name = "" };    
    // initialize linked list
    linkedlist *head = NULL;

    // read file 
    while (fscanf (fp, "%s %d %f", temp_data.name, &temp_data.runtime, &temp_data.probability) == 3) {
        // convert probabilty to string so that we can check if there is more than 2 decimal places
        char buf[10];
        gcvt(temp_data.probability, 4, buf);
        
        // error checking 
        if (strlen(temp_data.name) > 10) {
            fprintf(stderr, "Name '%s' must be no more than 10 characters\n",temp_data.name);
            exit(EXIT_FAILURE);
        } else if (temp_data.runtime < 1) {
            fprintf(stderr, "The run time must be an integer 1 or more.\n");
            exit(EXIT_FAILURE);
        } else if (temp_data.probability <= 0 || temp_data.probability >= 1 || strlen(buf) > 4) {
            fprintf(stderr, "The probability must be a decimal number between 0 and 1 with 2 decimal places.\n");
            exit(EXIT_FAILURE);
        }
        // allocate actual node
        input_data *data_ptr = malloc (sizeof *data_ptr); 

        // make sure it works
        if (!data_ptr) { 
             fprintf(stderr, "Malloc error");
            break;
        }

        // assign temporary data to the actual node
        *data_ptr = temp_data; 

        // add to linked list 
        push(&head, *data_ptr);


        //printf ("%s %d %.2f\n", data_ptr->name, data_ptr->runtime, data_ptr->probability);

    }
    // close file if not stdin
    if (fp != stdin) fclose (fp); 

    // run the CPU first and then the IO within each clock cycle
    // call CPU dispatch routine
    dispatch(argv[1], head);
                                   
    PrintList(head);
    return 0;
}