//
// Created by lemse on 09/09/2024.
//

#ifndef OS_ASSIGN_1_MAIN_H
#define OS_ASSIGN_1_MAIN_H

typedef struct int_llist intNode;
struct int_llist
{
    int value;
    intNode *next;
};

int add_int(struct int_llist **collection, int count);
int remove_last(struct int_llist **collection);
int free_list(struct int_llist **collection);
int print_list(struct int_llist *collection);

#endif //OS_ASSIGN_1_MAIN_H
