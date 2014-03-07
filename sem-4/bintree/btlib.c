#include "btlib.h"

typedef struct node_t {
	int data;
	struct node_t* left;
	struct node_t* right;
} node_t;

typedef struct tree_t {
	struct node_t* head ;
	int nnodes;
} tree_t;

typedef struct stack {
	void* arr;
	int sptr;
} stack;

typedef struct iterator_t {
	struct node_t* node;
	struct stack* st;
	int sp;
} iterator_t;

//######################################################################
// Stack

int STACK_SIZE = 256;

int st_init(stack* st){
	st->arr = calloc(STACK_SIZE, sizeof(node_t*));
	st->sptr = 0;
	return 0;
}

// End Stack
//######################################################################


//######################################################################
// Iterator

int it_init(iterator_t* it, tree_t* tree){
	if (it == NULL || tree == NULL)
		return -1;
	it->node = tree->head;
	st_init(it->st);
	return 0;
}

int it_isEnd(iterator_t* it){
	if (it->node == NULL)
		return 1;
	else
		return 0;
}

int it_next(iterator_t* it){
	if (it->node->left != NULL){
		it->node = it->node->left;
		return 0;
	}
	if (it->node->right != NULL){
		it->node = it->node->right;
		return 0;
	}
	
	return 0;
}

// End Iterator
//######################################################################


void printtabs(int);

int iterator_foo_node(node_t*, int(*)(int, int), int);

tree_t* init_tree(){
	tree_t* tree_ptr = (tree_t*) calloc(1, sizeof(tree_t));
	
	tree_ptr->nnodes = 0;
	tree_ptr->head = NULL;
	return tree_ptr;
}

node_t* creat_and_init_node(int data){
	node_t* tmpptr = (node_t*) malloc(sizeof(node_t));
	if (tmpptr == 0)
		return NULL;
	tmpptr->data = data;
	tmpptr->left = NULL;
	tmpptr->right = NULL;
	return tmpptr;
}


int insert_item(tree_t* tree, int data){
	if (tree == NULL)
		return -1;
		
	if (tree->nnodes == 0){
		tree->nnodes++;
		tree->head = creat_and_init_node(data);
		if (tree->head == NULL)
			return -1;
	}
	else {
		node_t* current_node = tree->head;
		while (1){
			if (data >= current_node->data){
				if (current_node->right != NULL)
					current_node = current_node->right;
				else {
					current_node->right = creat_and_init_node(data);
					break;
				}
			}
			else {
				if (current_node->left != NULL)
					current_node = current_node->right;
				else {
					current_node->left = creat_and_init_node(data);
					break;
				}
			}
		}
	}
	return 0;
}


int delete_node(node_t* node){
	if (node == NULL)
		return -1;
	if (node->left != NULL)
		delete_node(node->left);
	if (node->right != NULL)
		delete_node(node->right);
	node->data = 0;
	free(node);
	return 0;
}

int delete_tree(tree_t* tree){
	if (tree == 0)
		return -1;
	tree->nnodes = 0;
	delete_node(tree->head);
	free(tree);
	return 0;
}


const char* mytab = "    ";
void printtabs(int n){
	int i;
	for (i = 0; i < n; i++)
		printf("%s!", mytab);
}

int dump_node(node_t* node){
	if (node == NULL)
		return -1;
		
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

int iterator_foo(tree_t* tree, int (*user_foo)(int, int), int user_data ){
	
	if (tree == NULL || tree->head == NULL)
		return -1;
	return iterator_foo_node(tree->head, user_foo, user_data);
}

int iterator_foo_node(node_t* node, int (*user_foo)(int, int), int user_data){
	//static int i = 0;
	//i++;
	//printf("iterator i = %i\n  node data = %i\n", i, node->data);
	if (node == NULL)
		return -1;
	if (node->left != NULL){
		int rez = iterator_foo_node(node->left, user_foo, user_data);
		//printf("node data = %i rez_left = %i\n",node->data, rez);
		if (rez != 0)
			return rez;
	}
	if (node->right != NULL){
		int rez = iterator_foo_node(node->right, user_foo, user_data);
		//printf("node data = %i rez_right = %i\n",node->data, rez);
		if (rez != 0)
			return rez;
	}
	return user_foo(node->data, user_data);
}
