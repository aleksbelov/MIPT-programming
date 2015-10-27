#ifndef BTLIB_H
#define BTLIB_H

#include <stdlib.h>
#include <stdio.h>

enum {
	LEFT = 0, 
	RIGHT = 1,
	MAX_DIM
};

typedef struct tree_t
	tree_t;
typedef struct node_t
	node_t;
typedef struct foreach_t
	foreach_t;
typedef node_t* iterator_t;

struct tree_t* init_tree(int data);
int insert_item(tree_t* tree, int data, iterator_t* it, int child_side);
int delete_item(tree_t* tree, iterator_t*);
void delete_tree(tree_t*);
int dump_tree(tree_t*);

// Iterator for each
foreach_t* foreach_init(tree_t*);
int foreach_isEnd(foreach_t*);
void foreach_next(foreach_t*);
int foreach_current(foreach_t*);
int foreach_delete(foreach_t*);


//Iterator, which can turn left, right and up on tree
iterator_t* it_init(tree_t*);
int it_left(iterator_t*);
int it_right(iterator_t*);
int it_up(iterator_t*);
int it_current(iterator_t*);
void it_delete(iterator_t*);

//int iterator_foo(struct tree_t*, int(*)(int, int), int);

#endif
