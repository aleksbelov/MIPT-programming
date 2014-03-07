#ifndef BTLIB_H
#define BTLIB_H

#include <stdlib.h>
#include <stdio.h>

struct tree_t;
struct node_t;
struct iterator_t;

struct tree_t* init_tree();

int it_init(struct iterator_t*, struct tree_t* );


int insert_item(struct tree_t*, int);
int delete_tree(struct tree_t*);
int dump_tree(struct tree_t*);
//int iterator_foo(struct tree_t*, int(*)(int, int), int);

#endif
