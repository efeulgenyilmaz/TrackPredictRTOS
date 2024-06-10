//data structures
#ifndef dtst_h
#define dtst_h

typedef struct n{
    int x;
    int y;
    double time;
    struct n * next;
    struct n * prev;
}node;

node * add(node* head, int x, int y, double time);
void show(node *head);
void free_dtst(node *head);

#endif
