/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : fibheap.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : A Fibonacci heap implementation.
 ============================================================================
 */

 /**
 * A Fibonacci heap implementation for
 * priority queues.
 *
 */
struct FibonacciHeap {
	/** Maps arguments to their values. */
	int* values;
	/** The tree containing the minimum value in its root */
	struct FibonacciTree* minimumTree;
	/** Lists which elements are present in the heap. */
	struct BitSet* elements;
	/** The current number of elements */
	int number_of_elements;
	/** Maps each element to its node */
	struct FibonacciNode** elements2nodes;
	/** for consolidation (only temporarily used) */
	struct FibonacciTree** degree_arr;
	/** for consolidation (only temporarily used) */
	int arr_length;
};

struct FibonacciTree {
	/** The previous tree (all trees of a heap are circularly linked.*/
	struct FibonacciTree* previousTree;
	/** The previous tree (all trees of a heap are circularly linked.*/
	struct FibonacciTree* nextTree;
	/** The root node of this tree */
	struct FibonacciNode* root;
};

struct FibonacciNode {
	/** the actual argument */
	int* argument;
	/** the parent of this node */
	struct FibonacciNode* parent;
	/** the children of this node */
	struct FibonacciNode** children;
	/** number of children */
	int number_of_children;
	/** whether this node is marked */
	int marked;
	/** The tree where this node belongs to
	  * (only used and valid if the node is a root) */
	struct FibonacciTree* tree;
};


/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Creates a new heap with the given maximum length.
 * @param length
 */
void fibheap__init(struct FibonacciHeap* heap, int maxlength){
	heap->values = malloc(maxlength * sizeof(int));
	heap->minimumTree = NULL;
	heap->elements2nodes = malloc(maxlength * sizeof(struct FibonacciNode*));
	heap->elements = malloc(sizeof(struct BitSet));
	bitset__init(heap->elements, maxlength);
	bitset__unsetAll(heap->elements);
	heap->number_of_elements = 0;
	heap->arr_length = (int) floor(log2((double)maxlength))+2;
	heap->degree_arr = malloc(heap->arr_length*sizeof(struct FibonacciTree*));
}
/* ============================================================================================================== */
/* ============================================================================================================== */
void __fibheap__prettyprint_tree(struct FibonacciNode* node, int indent){
	for(int i = 0; i < indent; i++)
		printf("   ");
	printf("--> %d (%d,%d)\n", *node->argument, node->marked, node->number_of_children);
	for(int i = 0; i < node->number_of_children; i++)
		__fibheap__prettyprint_tree(node->children[i], indent+1);
}

void fibheap__prettyprint(struct FibonacciHeap* heap){
	printf("Number of elements: %d\n", heap->number_of_elements);
	printf("Elements with values: \n");
	for(int idx = bitset__next_set_bit(heap->elements,0); idx != -1 ; idx = bitset__next_set_bit(heap->elements, idx+1))
		printf("  %d -> %d\n", idx, heap->values[idx]);
	printf("All trees (starting with minimum tree): \n");
	struct FibonacciTree* current = heap->minimumTree;
	do{
		printf("   %p (%p, %p)\n",current, current->previousTree, current->nextTree);
		__fibheap__prettyprint_tree(current->root, 1);
		current = current->nextTree;
	}while(current != heap->minimumTree);
}
/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Check whether the given argument is contained in this heap.
 * @param arg some argument
 * @return zero iff it is not contained, otherwise one.
 */
int fibheap__contains(struct FibonacciHeap* heap, int* arg){
	int r = bitset__get(heap->elements,*arg) != 0 ? 1 : 0;
	return r;
}

/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Returns the value of the minimal element.
 * @return
 */
int fibheap__value_of_minimum(struct FibonacciHeap* heap){
	return heap->values[*heap->minimumTree->root->argument];
}

/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Adds the argument to this heap and repairs it.
 * @param arg some argument
 * @param value some value
 */
void fibheap__insert(struct FibonacciHeap* heap, int* arg, int value){
	// add to indices
	heap->values[*arg] = value;
	bitset__set(heap->elements, *arg);
	// create new tree
	struct FibonacciTree* tree = malloc(sizeof(struct FibonacciTree));
	tree->root = malloc(sizeof(struct FibonacciNode));
	tree->root->argument = arg;
	tree->root->marked = 0;
	tree->root->number_of_children = 0;
	tree->root->parent = NULL;
	tree->root->tree = tree;
	// add to index elements2nodes
	heap->elements2nodes[*arg] = tree->root;
	// insert into heap
	heap->number_of_elements++;
	if(heap->number_of_elements == 1){
		heap->minimumTree = tree;
		tree->nextTree = tree;
		tree->previousTree=  tree;
	}else{
		tree->nextTree = heap->minimumTree->nextTree;
		tree->previousTree = heap->minimumTree;
		heap->minimumTree->nextTree->previousTree = tree;
		heap->minimumTree->nextTree = tree;
		if(heap->values[*heap->minimumTree->root->argument] >= value)
			heap->minimumTree = tree;
	}
}

/* ============================================================================================================== */
/* ============================================================================================================== */
/** Compactifies the heap and repairs the link to minimum tree*/
void __fibheap__consolidate(struct FibonacciHeap* heap){
	for(int i = 0; i < heap->arr_length; i++)
		heap->degree_arr[i] = NULL;
	heap->degree_arr[heap->minimumTree->root->number_of_children] = heap->minimumTree;
	struct FibonacciTree* current = heap->minimumTree->nextTree;
	heap->minimumTree->previousTree->nextTree = NULL;
	while(current != NULL){
		int degree = current->root->number_of_children;
		if(heap->degree_arr[degree] == NULL){
			heap->degree_arr[degree] = current;
			current = current->nextTree;
		}else{
			struct FibonacciTree* other = heap->degree_arr[degree];
			heap->degree_arr[degree] = NULL;
			if(heap->values[*other->root->argument] > heap->values[*current->root->argument]){
				current->root->number_of_children++;
				if(degree == 0)
					current->root->children = malloc(sizeof(struct FibonacciNode*));
				else
					current->root->children = realloc(current->root->children, current->root->number_of_children * sizeof(struct FibonacciNode*));
				current->root->children[current->root->number_of_children-1] = other->root;
				other->root->parent = current->root;
				free(other);
			}else{
				other->root->number_of_children++;
				if(degree == 0)
					other->root->children = malloc(sizeof(struct FibonacciNode*));
				else
					other->root->children = realloc(other->root->children, other->root->number_of_children * sizeof(struct FibonacciNode*));
				other->root->children[other->root->number_of_children-1] = current->root;
				current->root->parent = other->root;
				other->nextTree = current->nextTree;
				free(current);
				current = other;
			}
		}
	}
	heap->minimumTree = NULL;
	struct FibonacciTree* prev = NULL;
	struct FibonacciTree* first = NULL;
	for(int i = 0; i < heap->arr_length; i++){
		if(heap->degree_arr[i] != NULL){
			if(prev != NULL){
				prev->nextTree = heap->degree_arr[i];
				heap->degree_arr[i]->previousTree = prev;
			}else first = heap->degree_arr[i];
			prev = heap->degree_arr[i];
			if(heap->minimumTree == NULL)
				heap->minimumTree = heap->degree_arr[i];
			else if(heap->values[*heap->degree_arr[i]->root->argument] < heap->values[*heap->minimumTree->root->argument])
				heap->minimumTree = heap->degree_arr[i];
		}
	}
	prev->nextTree = first;
	first->previousTree= prev;	;
}


/**
 * Removes and returns the minimal value of this heap,
 * repairs the heap afterwards.
 * @return the minimal value
 */
int* fibheap__extract_minimum(struct FibonacciHeap* heap){
	int* arg = heap->minimumTree->root->argument;
	bitset__unset(heap->elements, *arg);
	heap->number_of_elements--;
	if(heap->number_of_elements == 0){
		free(heap->minimumTree->root);
		free(heap->minimumTree);
	}else{
		if(heap->minimumTree->root->number_of_children == 0){
			struct FibonacciTree* old = heap->minimumTree;
			old->previousTree->nextTree = old->nextTree;
			old->nextTree->previousTree = old->previousTree;
			heap->minimumTree = old->nextTree;
			free(old->root);
			free(old);
		}else{
			struct FibonacciTree* new_tree;
			for(int i = 0; i < heap->minimumTree->root->number_of_children; i++){
				new_tree = malloc(sizeof(struct FibonacciTree));
				new_tree->root = heap->minimumTree->root->children[i];
				new_tree->root->tree = new_tree;
				new_tree->root->parent = NULL;
				new_tree->nextTree = heap->minimumTree->nextTree;
				new_tree->previousTree = heap->minimumTree;
				heap->minimumTree->nextTree->previousTree = new_tree;
				heap->minimumTree->nextTree = new_tree;
			}
			heap->minimumTree->nextTree->previousTree = heap->minimumTree->previousTree;
			heap->minimumTree->previousTree->nextTree = heap->minimumTree->nextTree;
			free(heap->minimumTree->root);
			free(heap->minimumTree);
			heap->minimumTree = new_tree;
		}
		//only consolidate if there are at least two trees
		if(heap->minimumTree != heap->minimumTree->nextTree)
			__fibheap__consolidate(heap);
	}
	return arg;
}

/* ============================================================================================================== */
/* ============================================================================================================== */
/**
 * Update the value of the given argument by subtracting a value.
 * @param arg
 */
void fibheap__decrease_value(struct FibonacciHeap* heap, int* arg, int amount){
	heap->values[*arg] = heap->values[*arg] - amount;
	if(heap->elements2nodes[*arg]->parent == NULL){
		if(heap->values[*arg] < heap->values[*heap->minimumTree->root->argument]){
			heap->minimumTree = heap->elements2nodes[*arg]->tree;
		}
		return;
	}
	if(heap->values[*arg] >= heap->values[*heap->elements2nodes[*arg]->parent->argument])
		return;
	struct FibonacciNode* current = heap->elements2nodes[*arg];
	struct FibonacciTree* new_tree = malloc(sizeof(struct FibonacciTree));
	new_tree->root = current;
	new_tree->root->tree = new_tree;
	current = current->parent;
	new_tree->root->parent = NULL;
	current->number_of_children--;
	for(int i = 0; i < current->number_of_children+1; i++){
		if(current->children[i] == new_tree->root){
			current->children[i] = current->children[current->number_of_children];
			break;
		}
	}
	new_tree->previousTree = heap->minimumTree;
	new_tree->nextTree = heap->minimumTree->nextTree;
	heap->minimumTree->nextTree = new_tree;
	new_tree->nextTree->previousTree = new_tree;
	if(heap->values[*new_tree->root->argument] < heap->values[*heap->minimumTree->root->argument]){
		heap->minimumTree = new_tree;
	}
	new_tree->root->marked = 0;
	while(current->parent != NULL && current->marked != 0){
		new_tree = malloc(sizeof(struct FibonacciTree));
		new_tree->root = current;
		new_tree->root->tree = new_tree;
		current = current->parent;
		new_tree->root->parent = NULL;
		current->number_of_children--;
		for(int i = 0; i < current->number_of_children+1; i++){
			if(current->children[i] == new_tree->root){
				current->children[i] = current->children[current->number_of_children];
				break;
			}
		}
		new_tree->previousTree = heap->minimumTree;
		new_tree->nextTree = heap->minimumTree->nextTree;
		heap->minimumTree->nextTree = new_tree;
		new_tree->nextTree->previousTree = new_tree;
		new_tree->root->marked = 0;
	}
	if(current->parent != NULL)
		current->marked = 1;
}

void fibheap__decrement_value(struct FibonacciHeap* heap, int *arg){
	fibheap__decrease_value(heap, arg, 1);
}
/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Removes the given argument from the heap
 * @param arg some argument
 */
void fibheap__remove(struct FibonacciHeap* heap, int* arg){
	fibheap__decrease_value(heap,arg, heap->values[*arg]+1-fibheap__value_of_minimum(heap));
	fibheap__extract_minimum(heap);
}

/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Update the value of the given argument to the given value
 * @param arg some argument
 */
void fibheap__update(struct FibonacciHeap* heap, int* arg, int value){
	if(!fibheap__contains(heap,arg)){
		fibheap__insert(heap,arg,value);
		return;
	}
	if(heap->values[*arg] == value)
		return;
	if(heap->values[*arg] > value){
		fibheap__decrease_value(heap,arg,heap->values[*arg]-value);
		return;
	}
	fibheap__remove(heap,arg);
	fibheap__insert(heap,arg,value);
}

/* ============================================================================================================== */
/* ============================================================================================================== */
void __fibheap__destroy_nodes(struct FibonacciNode* node){
	for(int i = 0; i < node->number_of_children; i++)
		__fibheap__destroy_nodes(node->children[i]);
	free(node);
}

void fibheap__destroy(struct FibonacciHeap* heap){
	free(heap->values);
	bitset__destroy(heap->elements);
	free(heap->elements2nodes);
	if(heap->number_of_elements == 0)
		return;
	struct FibonacciTree* current = heap->minimumTree;
	struct FibonacciTree* current2;
	do{
		__fibheap__destroy_nodes(current->root);
		current2 = current->nextTree;
		free(current);
		current = current2;
	}while(current != heap->minimumTree);
	free(heap->degree_arr);
	free(heap);
}
/* ============================================================================================================== */
/* ============================================================================================================== */


/** FOR TESTING
#include <time.h>

int minimumFromArray(int arr[], int deleted_numbers[], int len){
	int min = 1410065408;
	int min_idx = -1;
	for(int i = 0; i < len; i++){
		if(!deleted_numbers[i] && arr[i] < min){
			min = arr[i];
			min_idx = i;
		}
	}
	return min_idx;
}

int main(int argc, char *argv[]){
	srand((unsigned int)time(NULL));
	int NUM = 10000;
	int ACTIONS = 1000000;

	struct FibonacciHeap heap;
	int numbers[NUM];
	int deleted_numbers[NUM];
	fibheap__init(&heap,NUM);
	for (int i = 0; i < NUM; i++){
		numbers[i] = rand() % NUM;
		deleted_numbers[i] = 0;
		int* arg = malloc(sizeof(int));
		*arg = i;
		fibheap__insert(&heap,arg,numbers[i]);
		//printf("Added %i with value %i\n", *arg, numbers[i]);
	}
	printf("-----------\n");
	for(int i = 0; i < ACTIONS; i++){
		int arg = rand() % NUM;
		int val = rand() % NUM;
		fibheap__decrease_value(&heap, arg ,val);
		numbers[arg] -= val;
		//printf("Changed %i to value %i\n", arg, numbers[arg]);
	}
	//printf("-----------\n");
	for(int i = 0; i < NUM; i++){
		int value = fibheap__value_of_minimum(&heap);
		int arg = fibheap__extract_minimum(&heap);

		int arg_true = minimumFromArray(numbers,deleted_numbers,NUM);
		int value_true = numbers[arg_true];
		deleted_numbers[arg_true] = 1;

		if(value == value_true && arg == arg_true){
			//printf("YEAH! Extracted %i with value %i\n", arg, value);
		}else if(value == value_true){
			//printf("ALMOST! Extracted %i with value %i\n", arg, value);
		}else{
			printf("NO! Extracted %i with value %i\n", arg, value);
		}
	}
	fibheap__destroy(&heap);
}
 ******************************/
/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
