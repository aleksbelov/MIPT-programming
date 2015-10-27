#include "btlib.h"

typedef struct node_t {
	int data;
	struct node_t* left;
	struct node_t* right;
	struct node_t* parent;
	int passed;
} node_t;

typedef struct tree_t {
	struct node_t* head;
	int nnodes;
} tree_t;

typedef struct foreach_t {
	struct node_t* node;
	int nnodes;
	int i;
} foreach_t;

typedef node_t* iterator_t;

//######################################################################
// Iterator

int init_pass_field(node_t*);

foreach_t* foreach_init(tree_t *tree){
	if (tree == NULL)
		return NULL;
	
	foreach_t *it = (foreach_t*) calloc(1, sizeof(foreach_t));
	if (it == NULL)
		return NULL;
	it->node = tree->head;
	it->nnodes = tree->nnodes;
	it->i = 0;
	if (tree->head != NULL)
		init_pass_field(tree->head);
	return it;
}

int init_pass_field(node_t* node){
	node->passed = 0;
	if (node->left != NULL)
		init_pass_field(node->left);
	if (node->right != NULL)
		init_pass_field(node->right);
	return 0;
}

void foreach_next(foreach_t* it){
	if (it->node == NULL)
		return;
	if ((it->node->left != NULL) && (!it->node->left->passed))
		it->node = it->node->left;
	else if ((it->node->right != NULL) && (!it->node->right->passed))
		it->node = it->node->right;
	else {
		it->node->passed = 1;
		//printf("%p ", it->node->parent);
		it->node = it->node->parent;
		foreach_next(it);
	}
	it->i++;
}

int foreach_current(foreach_t* it){
	//if (it == NULL || it->node == NULL)
	//	return -1;
	return it->node->data;
}

int foreach_isEnd(foreach_t* it){
	if (it->i < it->nnodes)
		return 0;
	else
		return 1;
}

int foreach_delete(foreach_t *it){
	free(it);
	return 0;
}

//---------------

iterator_t* it_init(tree_t* tree){
	if (tree == NULL)
		return NULL;
	
	iterator_t* it = (iterator_t*) calloc(1, sizeof(iterator_t));
	if (it == NULL)
		return NULL;
	*it = tree->head;
	return it;
}

int it_left(iterator_t* it){
	if (it == NULL || *it == NULL)
		return -1;
	if ((*it)->left != NULL)
		*it = (*it)->left;
	return 0;
}

int it_right(iterator_t* it){
	if (it == NULL || *it == NULL)
		return -1;
	if ((*it)->right != NULL)
		*it = (*it)->right;
	return 0;
}

int it_up(iterator_t* it){
	if (it == NULL || *it == NULL)
		return -1;
	*it = (*it)->parent;
	return 0;
}

int it_current(iterator_t* it){
	if (it == NULL || *it == NULL)
		return -1;
	return (*it)->data;
}

void it_delete(iterator_t* it){
	free(it);
}


// End Iterator
//######################################################################

node_t* creat_and_init_node(int data, node_t* parent){
	node_t* tmpptr = (node_t*) malloc(sizeof(node_t));
	if (tmpptr == NULL)
		return NULL;
	tmpptr->passed = 0;
	tmpptr->data = data;
	tmpptr->left = NULL;
	tmpptr->right = NULL;
	tmpptr->parent = parent;
	return tmpptr;
}

tree_t* init_tree(int data){
	tree_t* tree_ptr = (tree_t*) calloc(1, sizeof(tree_t));
	if (tree_ptr == NULL)
		return NULL;
	
	tree_ptr->nnodes = 1;
	tree_ptr->head = creat_and_init_node(data, NULL);
	return tree_ptr;
}

int delete_node(node_t* node){
	int del_nodes = 0;
	if (node != NULL){
		if (node->left != NULL)
			del_nodes += delete_node(node->left);
		if (node->right != NULL)
			del_nodes += delete_node(node->right);
	}
	free(node);
	return del_nodes + 1;
}


int insert_item(tree_t* tree, int data, iterator_t* it, int child_side){
	if (tree == NULL || it == NULL || *it == NULL)
		return -1;
		
	if (child_side == LEFT){
		if ((*it)->left != NULL)
			tree->nnodes -= delete_node((*it)->left);
		(*it)->left = creat_and_init_node(data, *it);
	}
	else if (child_side == RIGHT){
		if ((*it)->right != NULL)
			tree->nnodes -= delete_node((*it)->right);
		(*it)->right = creat_and_init_node(data, *it);
	}
	else return -2;
	
	tree->nnodes++;
	return 0;
}


int delete_item(tree_t* tree, iterator_t* it){
	if (it == NULL || *it == NULL)
		return -1;
	if ((*it)->parent->left == (*it)){
		it_up(it);
		tree->nnodes -= delete_node((*it)->left);
		(*it)->left = NULL;
	}
	else{
		it_up(it);
		tree->nnodes -= delete_node((*it)->right);
		(*it)->right = NULL;
	}
	return 0;
}

void delete_tree(tree_t* tree){
	if (tree != NULL){
		tree->nnodes = 0;
		delete_node(tree->head);
	}
	free(tree);
}



int dump_node(node_t* node){		
	printf("(%i| ", node->data);
	if (node->left == NULL)
		printf("* ");
	else {
		dump_node(node->left);
	}
	if (node->right == NULL)
		printf("*");
	else
		dump_node(node->right);
	printf(")");
	return 0;
}

int dump_tree(tree_t* tree){
	if (tree == NULL)
		return -1;
	
	printf(	"*** Tree ***\n"
			"nnodes = %i\n", tree->nnodes);
	dump_node(tree->head);
	printf("\n* End tree *\n");
	return 0;
}

