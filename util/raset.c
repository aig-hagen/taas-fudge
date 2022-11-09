/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : raset.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : Implementation of a "random access set", i.e. a set data structure,
               that has O(1) runtime for checking whether an element is in the set,
               adding/deleting an element, and selecting some element with uniform
               probability.
 ============================================================================
 */

struct RaSet{
  // bitset indicating whether an element is in the set
  struct BitSet* elements;
  // the number of elements
  int number_of_elements;
  // the maximum number of elements
  int max_number_of_elements;
  // the first "number_elements" of the following array are the elements in the set
  int* elements_arr;
  // for each possible element the following array gives the index of that element
  // in the elements_arr array
  int* elements_arr_inverted;
};

// initialises and returns a new empty raset with the given maximal number
// of elements
struct RaSet* raset__init_empty(int max_number_of_elements){
  struct RaSet* set = (struct RaSet*) malloc(sizeof(struct RaSet));
  set->max_number_of_elements = max_number_of_elements;
  set->number_of_elements = 0;
  set->elements = (struct BitSet*) malloc(sizeof(struct BitSet));
  bitset__init(set->elements,set->max_number_of_elements);
  bitset__unsetAll(set->elements);
  set->elements_arr = (int*) malloc(set->max_number_of_elements * sizeof(int));
  set->elements_arr_inverted = (int*) malloc(set->max_number_of_elements * sizeof(int));
  return set;
}

// resets the set (removes all elements)
void raset__reset(struct RaSet* set){
  set->number_of_elements = 0;
  bitset__unsetAll(set->elements);
}

// return TRUE iff the element is contained in this set
int raset__contains(struct RaSet* set, int element){
  return bitset__get(set->elements,element);
}

// adds the element to the set (if it is not already included);
// returns TRUE if the set has been modified, FALSE otherwise
int raset__add(struct RaSet* set, int element){
  if(raset__contains(set,element))
    return FALSE;
  bitset__set(set->elements,element);
  set->elements_arr[set->number_of_elements] = element;
  set->elements_arr_inverted[element] = set->number_of_elements;
  set->number_of_elements++;
  return TRUE;
}

// removes the element from this set (if it is contained);
// returns TRUE if the set has been modified, FALSE otherwise;
int raset__remove(struct RaSet* set, int element){
  if(!raset__contains(set,element))
    return FALSE;
  bitset__unset(set->elements,element);
  set->number_of_elements--;
  set->elements_arr[set->elements_arr_inverted[element]] = set->elements_arr[set->number_of_elements];
  set->elements_arr_inverted[set->elements_arr[set->elements_arr_inverted[element]]] = set->elements_arr_inverted[element];
  return TRUE;
}

// returns a random element from this set
int raset__random_element(struct RaSet* set){
    return set->elements_arr[rand() % set->number_of_elements];
}

// returns a random element from (this set MINUS the given set)
// if there is no such element, -1 is returned
int raset__random_element_with_skip(struct RaSet* set, struct BitSet* ignore){
  int* arr = (int*) malloc(set->number_of_elements * sizeof(int));
  int idx = 0;
  for(int i = 0; i < set->number_of_elements; i++)
    if(!bitset__get(ignore,set->elements_arr[i]))
      arr[idx++] = set->elements_arr[i];
  if(idx == 0)
    return -1;
  int result = arr[rand() % idx];
  free(arr);
  return result;
}

// returns the element at index position idx
int raset__get(struct RaSet* set, int idx){
  return set->elements_arr[idx];
}

// prints the set using the given map to strings
void raset__print(struct RaSet* set, char** ids2arguments){
  printf("[");
  char isFirst = TRUE;
  for(int i = 0; i < set->number_of_elements; i++){
    if(isFirst)
      isFirst = FALSE;
    else printf(",");
    printf("%s", ids2arguments[set->elements_arr[i]]);
  }
  printf("]\n");
}

// prints the set using the given map to strings in ICCMA23 style
void raset__print_i23(struct RaSet* set, char** ids2arguments){
  printf("w ");
  char isFirst = TRUE;
  for(int i = 0; i < set->number_of_elements; i++)
    printf("%s ", ids2arguments[set->elements_arr[i]]);
  printf("\n");
}

// frees the set
void raset__destroy(struct RaSet* set){
  bitset__destroy(set->elements);
  free(set->elements_arr);
  free(set->elements_arr_inverted);
  free(set);
}
/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
