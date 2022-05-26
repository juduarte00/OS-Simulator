#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLENGTH 10


/* reference for linked list struct and corresponding push/print functions: http://www.mahonri.info/SO/23279119_LinkedList_101.c*/

// struct for keeping track of input data
typedef struct {
    char name[MAXLENGTH];
    int runtime;
    float probability;
}input_data; 

// struct for linked list
typedef struct {
    input_data data;
    struct linkedlist *next;
}linkedlist;

// function for pushing input data into a linked list
void push (linkedlist **head, input_data data){
    linkedlist *newNode = NULL;
    // allocate memory for new node
    newNode = malloc(sizeof(*newNode));
    if (!newNode) {
        fprintf(stderr, "Malloc error");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = *head; 
    *head = newNode; 
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
                                   
    PrintList(head);
    return 0;
}