#include "btlib.h"
#include <stdlib.h>
#include <time.h>

int main(){
	
	tree_t * tree = init_tree(100);
	iterator_t* it = it_init(NULL);
	it = it_init(tree);
	it_left(it);
	it_right(it);
	
	it_current(NULL);
	it_up(NULL);
	it_right(NULL);
	it_left(NULL);
	
	
	insert_item(NULL, 0, it, RIGHT);
	insert_item(tree, 200, it, 10);
	
	insert_item(tree, 30, it, LEFT);
	insert_item(tree, 40, it, RIGHT);
	insert_item(tree, 20, it, LEFT);
	
	it_right(it);
	it_left(it);
	insert_item(tree, 100500, it, RIGHT);
	insert_item(tree, 10, it, RIGHT);
	insert_item(tree, 25, it, LEFT);
	it_right(it);
	dump_tree(NULL);
	dump_tree(tree);
	
	foreach_t* iter = foreach_init(NULL);
	for (iter = foreach_init(tree); !foreach_isEnd(iter); foreach_next(iter)){
		//printf("%i/%i:%i\n",iter->i, iter->nnodes, foreach_current(iter));
		printf("%i\n",foreach_current(iter));
	}
	printf("\n");
	foreach_delete(iter);
	
	delete_item(tree, NULL);
	
	printf("current = %i\n", it_current(it));
	it_up(it);
	it_left(it);
	printf("current = %i\n", it_current(it));
	dump_tree(tree);
	delete_item(tree, it);
	printf("current = %i\n", it_current(it));
	dump_tree(tree);
	it_right(it);
	delete_item(tree, it);
	dump_tree(tree);
	
	for (iter = foreach_init(tree); !foreach_isEnd(iter); foreach_next(iter)){
		//printf("%i/%i:%i\n",iter->i, iter->nnodes, foreach_current(iter));
		printf("%i\n",foreach_current(iter));
	}
	printf("\n");
		
	delete_tree(NULL);
	delete_tree(tree);
	
	it_delete(it);
	it_delete(NULL);
	
	foreach_delete(NULL);
	foreach_delete(iter);
	return 0;
}
