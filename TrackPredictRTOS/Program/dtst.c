#include <stdio.h>
#include <stdlib.h>
#define data_count 5


typedef struct n{
    int x;
    int y;
    double time;
    struct n * next;
    struct n * prev;
}node;

//node * head = NULL;

node * add(node* head, int x, int y, double time){
    if(head == NULL){
        head = (node*)malloc(sizeof(node));
        head->x = x;
        head->y = y;
        head->time = 0;
        head->next = head;
        head->prev = head;
        return head;
    }
    node *temp_last = head->prev;
    
    temp_last->next = NULL;
    temp_last->next = (node*)malloc(sizeof(node));
    temp_last->next->next = head;
    head->prev = temp_last->next;
    temp_last->next->prev = temp_last;
    head->prev->prev = temp_last;
    
    temp_last = temp_last->next;
    temp_last->x = x;
    temp_last->y = y;
    temp_last->time = time;
    return head;
    
}

void show(node *head){
    if(head == NULL){
        printf("no element in structure!");
        return;
    }
    else{
        node * temp = head;
        int index = 1;
        while(temp->next != head){
            printf("%d: %d, %d, %f\n", index, temp->x, temp->y, temp->time);
            temp = temp->next;
            index++;
        }
        printf("%d: %d, %d, %f\n", index, temp->x, temp->y, temp->time);
        return;
    }
}

void free_dtst(node *head) {
    if (head == NULL) {
        printf("Data structure is NULL. Nothing freed!\n");
        return;
    }

    node *temp = head;
    node *nextNode;
    
    do {
        nextNode = temp->next;
        free(temp);
        temp = nextNode;
    } while (temp != head);

    printf("Memory cleaned! Exiting...\n");
}
