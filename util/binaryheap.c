/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : binaryheap.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : A binary heap implementation.
 ============================================================================
 */

 /**
 * A binary heap implementation for
 * priority queues.
 *
 */
struct BinaryHeap {
	/** the actual heap represented as an array (with keys as elements) */
	int** data;
  /** the values of the keys in the heap (used as sorting criterion) */
  int* values;
  /** Points each key to its index in data. */
  int* inv_index;
	/** the allocated length of the array  */
	int maxlength;
  /** the number of elements in the heap */
  int length;
  /** Lists which elements are present in the heap. */
	struct BitSet* elements;
};

/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Creates a new heap with the given maximum length.
 * @param length
 */
void binaryheap__init(struct BinaryHeap* heap, int maxlength){
  heap->data = malloc(maxlength * sizeof(int*));
  heap->values = malloc(maxlength * sizeof(int));
  heap->inv_index = malloc(maxlength * sizeof(int));
  heap->maxlength = maxlength;
  heap->length = 0;
  heap->elements = malloc(sizeof(struct BitSet));
	bitset__init(heap->elements, maxlength);
	bitset__unsetAll(heap->elements);
}

/* ============================================================================================================== */
/* ============================================================================================================== */
void __binaryheap__prettyprint_tree(struct BinaryHeap* heap, int node, int indent){
  if(node < heap->length){
      for(int i = 0; i < indent; i++)
    		printf("   ");
      printf("%d=%d\n", *heap->data[node], heap->values[*heap->data[node]]);
      __binaryheap__prettyprint_tree(heap, 2*node+1, indent+1);
      __binaryheap__prettyprint_tree(heap, 2*node+2, indent+1);
  }
}

void binaryheap__prettyprint(struct BinaryHeap* heap){
	printf("Number of elements: %d (%d)\n", heap->length,heap->maxlength);
	printf("Tree: \n");
	__binaryheap__prettyprint_tree(heap, 0, 0);
}
/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Check whether the given argument is contained in this heap.
 * @param arg some argument
 * @return zero iff it is not contained, otherwise one.
 */
int binaryheap__contains(struct BinaryHeap* heap, int* arg){
	return bitset__get(heap->elements,*arg) != 0 ? 1 : 0;
}

/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Returns the value of the minimal element.
 * @return
 */
int binaryheap__value_of_minimum(struct BinaryHeap* heap){
  return heap->values[*heap->data[0]];
}

/* ============================================================================================================== */
/* ============================================================================================================== */

/** swaps two elements in an array */
void swap(int** arr, int idx1, int idx2){
  int* tmp;
  tmp = arr[idx1];
  arr[idx1] = arr[idx2];
  arr[idx2] = tmp;
}

/**
 * Sifts up the element at index idx
 */
void __binaryheap_siftup(struct BinaryHeap* heap, int idx){
  int parent_idx;
  int* tmp;
  while(idx > 0){
    parent_idx = (idx-1)/2;
    if(heap->values[*heap->data[parent_idx]] >= heap->values[*heap->data[idx]]){
      swap(heap->data,parent_idx,idx);
      heap->inv_index[*heap->data[parent_idx]] = parent_idx;
      heap->inv_index[*heap->data[idx]] = idx;
      idx = parent_idx;
    }else break;
  }
}

/**
 * Adds the argument to this heap and repairs it.
 * @param arg some argument
 * @param value some value
 */
void binaryheap__insert(struct BinaryHeap* heap, int* arg, int value){
	// add to indices
	heap->values[*arg] = value;
	bitset__set(heap->elements, *arg);
	// add to actual heap
  int idx = heap->length++;
  heap->data[idx] = arg;
  heap->inv_index[*arg] = idx;
  __binaryheap_siftup(heap,idx);
}

/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Removes and returns the minimal value of this heap,
 * repairs the heap afterwards.
 * @return the minimal value
 */
int* binaryheap__extract_minimum(struct BinaryHeap* heap){
	//binaryheap__prettyprint(heap);
	int* arg = heap->data[0];
	bitset__unset(heap->elements, *arg);
	heap->length--;
	if(heap->length > 0){
    heap->data[0] = heap->data[heap->length];
    heap->inv_index[*heap->data[0]] = 0;
    int current = 0;
    while(current < heap->length){
      int childLeft = 2*current + 1;
			int childRight = 2*current + 2;
			if(childLeft > heap->length-1)
				break;
			if(childRight < heap->length){
				if(heap->values[*heap->data[current]] <= heap->values[*heap->data[childLeft]] && heap->values[*heap->data[current]] <= heap->values[*heap->data[childRight]])
					break;
				else if(heap->values[*heap->data[childLeft]] < heap->values[*heap->data[childRight]]){
					swap(heap->data,current,childLeft);
					heap->inv_index[*heap->data[current]] = current;
					heap->inv_index[*heap->data[childLeft]] = childLeft;
					current = childLeft;
				}else{
					swap(heap->data,current,childRight);
					heap->inv_index[*heap->data[current]] = current;
					heap->inv_index[*heap->data[childRight]] = childRight;
					current = childRight;
				}
			}else if(heap->values[*heap->data[current]] <= heap->values[*heap->data[childLeft]])
				break;
			else{
				swap(heap->data,current,childLeft);
				heap->inv_index[*heap->data[current]] = current;
				heap->inv_index[*heap->data[childLeft]] = childLeft;
				current = childLeft;
			}
    }
	}
	return arg;
}

/* ============================================================================================================== */
/* ============================================================================================================== */
/**
 * Update the value of the given argument by subtracting a value.
 * @param arg
 */
void binaryheap__decrease_value(struct BinaryHeap* heap, int* arg, int amount){
	heap->values[*arg] = heap->values[*arg] - amount;
  __binaryheap_siftup(heap,heap->inv_index[*arg]);
}

void binaryheap__decrement_value(struct BinaryHeap* heap, int *arg){
	binaryheap__decrease_value(heap, arg, 1);
}
/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Removes the given argument from the heap
 * @param arg some argument
 */
void binaryheap__remove(struct BinaryHeap* heap, int* arg){
	binaryheap__decrease_value(heap,arg, heap->values[*arg]+1-binaryheap__value_of_minimum(heap));
	binaryheap__extract_minimum(heap);
}

/* ============================================================================================================== */
/* ============================================================================================================== */

/**
 * Update the value of the given argument to the given value
 * @param arg some argument
 */
void binaryheap__update(struct BinaryHeap* heap, int* arg, int value){
	if(!binaryheap__contains(heap,arg)){
		binaryheap__insert(heap,arg,value);
		return;
	}
	if(heap->values[*arg] == value)
		return;
	if(heap->values[*arg] > value){
		binaryheap__decrease_value(heap,arg,heap->values[*arg]-value);
		return;
	}
	binaryheap__remove(heap,arg);
	binaryheap__insert(heap,arg,value);
}

/* ============================================================================================================== */
/* ============================================================================================================== */

void binaryheap__destroy(struct BinaryHeap* heap){
	free(heap->values);
  free(heap->data);
	bitset__destroy(heap->elements);
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
	int ACTIONS = 10000;

	struct BinaryHeap* heap = malloc(sizeof(struct BinaryHeap));
	int numbers[NUM];
	int deleted_numbers[NUM];
	binaryheap__init(heap,NUM);
	for (int i = 0; i < NUM; i++){
		numbers[i] = rand() % NUM;
		deleted_numbers[i] = 0;
		int* arg = malloc(sizeof(int));
		*arg = i;
		binaryheap__insert(heap,arg,numbers[i]);
		//printf("Added %i with value %i\n", *arg, numbers[i]);
	}
  //binaryheap__prettyprint(heap);
	printf("-----------\n");
  //exit(0);
	for(int i = 0; i < ACTIONS; i++){
		int arg = rand() % NUM;
		int val = rand() % NUM;
		binaryheap__decrease_value(heap, &arg ,val);
		numbers[arg] -= val;
		//printf("Changed %i to value %i\n", arg, numbers[arg]);
	}
	//printf("-----------\n");
	for(int i = 0; i < NUM; i++){
		int value = binaryheap__value_of_minimum(heap);
		int* arg = binaryheap__extract_minimum(heap);

		int arg_true = minimumFromArray(numbers,deleted_numbers,NUM);
		int value_true = numbers[arg_true];
		deleted_numbers[arg_true] = 1;

		if(value == value_true && *arg == arg_true){
			//printf("YEAH! Extracted %i with value %i\n", *arg, value);
		}else if(value == value_true){
			//printf("ALMOST! Extracted %i with value %i (expected %i)\n", *arg, value, arg_true);
		}else{
			printf("NO! Extracted %i with value %i\n", *arg, value);
		}
	}
	binaryheap__destroy(heap);
}
******************************/
/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
