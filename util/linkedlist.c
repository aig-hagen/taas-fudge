/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : linkedlist.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : A simple linked list implementation.
 ============================================================================
 */

/**
 * A simple linked list.
 */
struct LinkedList{
	struct LinkedListNode* root;
	struct LinkedListNode* tail;
	int length;
};

/**
 * A node of a linked list
 */
struct LinkedListNode{
	void* data;
	struct LinkedListNode* next;
};

/** Inits an empty linked list */
void llist__init(struct LinkedList* list){
	list->root = NULL;
	list->tail = NULL;
	list->length = 0;
}

/** Adds a new value to the end of the list. */
void llist__add(struct LinkedList* list, void* data){
	list->length++;
	if(list->length == 1){
		list->root = malloc(sizeof(struct LinkedListNode));
		list->tail = list->root;
		list->root->data = data;
		list->root->next = NULL;
	}else{
		list->tail->next = malloc(sizeof(struct LinkedListNode));
		list->tail->next->data = data;
		list->tail->next->next = NULL;
		list->tail = list->tail->next;
	}
}

/** Adds a new value to the beginning of the list. */
void llist__push(struct LinkedList* list, void* data){
	list->length++;
	if(list->length == 1){
		list->root = malloc(sizeof(struct LinkedListNode));
		list->tail = list->root;
		list->root->data = data;
		list->root->next = NULL;
	}else{
		struct LinkedListNode* newRoot = malloc(sizeof(struct LinkedListNode));
		newRoot->next = list->root;
		newRoot->data = data;
		list->root = newRoot;
	}
}

// returns the idx-element of this list (if it exists)
// or NULL instead
void* llist__get(struct LinkedList* list, int idx){
	if(idx+1 > list->length)
		return NULL;
	int i = 0;
	struct LinkedListNode* current = list->root;
	while(i++ < idx)
		current = current->next;
	return current->data;
}

// returns the idx-element of this list, skipping elements from
// the given BitSet or NULL instead
void* llist__get_with_skip(struct LinkedList* list, int idx, struct BitSet* ignore){
	int i = 0;
	struct LinkedListNode* current = list->root;
	while(i++ < idx)
		current = current->next;
	int idx2 = 0;
	while(bitset__get(ignore,*(int*)current->data)){
		if(idx2 >= list->length)
			return NULL;
		if(current->next == NULL)
			current = list->root;
		else current = current->next;
		idx2++;
	}
	return current->data;
}

// returns the value of the root and removes it from the list
void* llist__pop(struct LinkedList* list){
	if(list->length <= 0)
		return NULL;
	void* data = list->root->data;
	struct LinkedListNode* oldRoot = list->root;
	list->root = list->root->next;
	free(oldRoot);
	list->length--;
	if(list->length == 0)
		list->tail = NULL;
	return data;
}

int llist__contains(struct LinkedList* list, void* data){
	for(struct LinkedListNode* node = list->root; node != NULL; node = node->next)
		if(node->data == data)
			return TRUE;
	return FALSE;
}

int llist__contains_int(struct LinkedList* list, int val){
	for(struct LinkedListNode* node = list->root; node != NULL; node = node->next)
		if(*(int*)node->data == val)
			return TRUE;
	return FALSE;
}

void llist__destroy(struct LinkedList* list){
	struct LinkedListNode* current = list->root;
	struct LinkedListNode* next;
	while(current != NULL){
		next = current->next;
		free(current->data);
		free(current);
		current = next;
	}
  free(list);
}

void llist__destroy_without_listanddata(struct LinkedList* list){
	struct LinkedListNode* current = list->root;
	struct LinkedListNode* next;
	while(current != NULL){
		next = current->next;
		free(current);
		current = next;
	}
}

void llist__destroy_without_data(struct LinkedList* list){
	struct LinkedListNode* current = list->root;
	struct LinkedListNode* next;
	while(current != NULL){
		next = current->next;
		free(current);
		current = next;
	}
  free(list);
}
/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
