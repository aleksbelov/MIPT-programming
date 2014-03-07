#include "btlib.h"
#include <stdlib.h>
int my_foo(int elem, int try){
	if (elem != try)
		return 0;
	else
		return 1;
}


int main(){
	struct tree_t *tree = init_tree();
	insert_item(tree, 5);
	insert_item(tree, 4);
	insert_item(tree, 8);
	dump_tree(tree);
	//int iffind = iterator_foo(tree, my_foo, 4);
	//printf("find: %i\n", iffind);
	delete_tree(tree);
	return 0;
}
