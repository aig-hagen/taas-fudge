/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : bitset.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : A simple bitset implementation.
 ============================================================================
 */
#define WORD_BITS (8 * sizeof(unsigned int))

/* A bit set structure */
struct BitSet{
	unsigned int* data;
	// number of bits
	int length;
	// number of ints
	int num_elements;
};

/** Init bit set. */
void bitset__init(struct BitSet* bitset, int length){
	bitset->num_elements = ((length/WORD_BITS)+1);
	bitset->data = (unsigned int*) malloc(bitset->num_elements * sizeof(unsigned int));
  bitset->length = length;
}

/**
 * Randomly sets the bits in this bitset
 */
void bitset__randomize(struct BitSet* bitset){
	for(int i = 0; i < bitset->num_elements; i++)
		bitset->data[i] = rand();
}

/** Clones the first bitset */
void bitset__clone(struct BitSet* bitset1, struct BitSet* bitset2){
	bitset2->num_elements = bitset1->num_elements;
	bitset2->length = bitset1->length;
	bitset2->data = (unsigned int*) malloc(bitset2->num_elements * sizeof(unsigned int));
	memcpy(bitset2->data,bitset1->data,bitset2->num_elements*sizeof(unsigned int));
}

/** set a value in a bitset **/
void bitset__set(struct BitSet* bitset, size_t idx) {
	bitset->data[idx / WORD_BITS] |= (1 << (idx % WORD_BITS));
}

/** sets all bits */
void bitset__setAll(struct BitSet* bitset){
	memset(bitset->data, ~0, bitset->num_elements * sizeof(unsigned int));
}

/** unset a value in a bitset **/
void bitset__unset(struct BitSet* bitset, size_t idx) {
	bitset->data[idx / WORD_BITS] &= ~(1 << (idx % WORD_BITS));
}

/** unsets all bits */
void bitset__unsetAll(struct BitSet* bitset){
	memset(bitset->data, 0, bitset->num_elements * sizeof(unsigned int));
}

/** get value of bit in bitset **/
int bitset__get(struct BitSet* bitset, size_t idx){
	return ((bitset->data[idx / WORD_BITS]) & (1<<(idx % WORD_BITS)));
}

/** Returns the index of the next set bit starting from given idx (it may
 * return idx itself), or -1 if no more set bit found. */
int bitset__next_set_bit(struct BitSet* bitset, int idx){
	//TODO make this more efficient?
	int i,j;
	int elem = idx/WORD_BITS;
	if(bitset->data[elem] != 0){
		for(j = idx; bitset__get(bitset,j) == 0 && j < (elem+1)*WORD_BITS && j < bitset->length; j++ );
		if(j >= bitset->length) return -1;
		if(j < (elem+1)*WORD_BITS){
			return j;
		}
	}
	if(elem+1 >= bitset->num_elements) return -1;
	for(i = elem+1; i < bitset->num_elements && bitset->data[i] ==0; i++);
	if(i >= bitset->num_elements) return -1;
	for(j = i * WORD_BITS; bitset__get(bitset,j) == 0 && j < bitset->length; j++ );
	if(j >= bitset->length) return -1;
	return j;
}

/** resizes the bit set */
void bitset__resize(struct BitSet* bitset, int new_length, int value){
	bitset->num_elements = ((new_length/WORD_BITS)+1);
	bitset->data = (unsigned int*) realloc(bitset->data, bitset->num_elements * sizeof(unsigned int));
	int old_length = bitset->length;
	bitset->length = new_length;
	// set values to default value
	// TODO make this more efficient
	if(value)
		for(int i = old_length; i < new_length; i++)
			bitset__set(bitset, i);
	else
		for(int i = old_length; i < new_length; i++)
			bitset__unset(bitset, i);
}

/** free allocated memory */
void bitset__destroy(struct BitSet* bitset){
	free(bitset->data);
	free(bitset);
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
