/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : graph.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : Utility functions on graphs.
 ============================================================================
 */

// this data structure contains a subset of all odd
// cycles of some given AAF (at most one odd cycle for each argument);
// it is constructed using a modified depth-first search from each argument
struct OddCycleCollection{
  // an array of linked lists (=sequences of nodes)
  struct LinkedList** oddCycles;
  // for quick checks whether an argument is part of an odd cycle
  struct BitSet* arguments_in_odd_cycles;
  // for each cycle the arguments of that cycle
  struct BitSet** arguments_in_individual_odd_cycles;
  // for each odd cycle the arguments attacking that cycle
  struct RaSet** arguments_attacking_cycle;
  // an array to provide pointers to all arguments
  int* arguments;
  // the number of odd cycles
  int number_of_odd_cycles;
};

// prints one cycle
void occ__print_cycle(struct LinkedList* cycle, struct AAF* aaf){
  printf("[");
  int isFirst = TRUE;
  for(struct LinkedListNode* node = cycle->root; node != NULL; node = node->next){
    if(isFirst)
      isFirst = FALSE;
    else printf(",");
    printf("%s", aaf->ids2arguments[*(int*)node->data]);
  }
  printf("]");
}

// prints an odd cycle collection
void occ__print(struct OddCycleCollection* occ, struct AAF* aaf){
  printf("[");
  int isFirst = TRUE;
  for(int i = 0; i < occ->number_of_odd_cycles;i++){
    if(isFirst)
      isFirst = FALSE;
    else printf(",");
    occ__print_cycle(occ->oddCycles[i],aaf);
  }
  printf("]");
}

// adds the given cycle to the collection
// returns TRUE if this cycle is not attacked.
int occ__add_cycle(struct OddCycleCollection* occ, struct LinkedList* cycle, struct AAF* aaf){
  occ->number_of_odd_cycles++;
  occ->oddCycles = realloc(occ->oddCycles, occ->number_of_odd_cycles * sizeof(struct LinkedList*));
  occ->arguments_in_individual_odd_cycles = realloc(occ->arguments_in_individual_odd_cycles, occ->number_of_odd_cycles * sizeof(struct BitSet*));
  occ->arguments_attacking_cycle = realloc(occ->arguments_attacking_cycle, occ->number_of_odd_cycles * sizeof(struct RaSet*));
  occ->arguments_attacking_cycle[occ->number_of_odd_cycles-1] = raset__init_empty(aaf->number_of_arguments);
  occ->oddCycles[occ->number_of_odd_cycles-1] = cycle;
  occ->arguments_in_individual_odd_cycles[occ->number_of_odd_cycles-1] = malloc(sizeof(struct BitSet));
  bitset__init(occ->arguments_in_individual_odd_cycles[occ->number_of_odd_cycles-1], aaf->number_of_arguments);
  bitset__unsetAll(occ->arguments_in_individual_odd_cycles[occ->number_of_odd_cycles-1]);
  int previousArgument = *(int*)cycle->tail->data;
  for(struct LinkedListNode* node = cycle->root; node != NULL; node = node->next){
    bitset__set(occ->arguments_in_odd_cycles,*(int*)node->data);
    bitset__set(occ->arguments_in_individual_odd_cycles[occ->number_of_odd_cycles-1],*(int*)node->data);
    for(GSList* node2 = aaf->parents[*(int*)node->data]; node2 != NULL; node2 = node2->next)
      if( *(int*)node2->data != previousArgument)
        raset__add(occ->arguments_attacking_cycle[occ->number_of_odd_cycles-1], *(int*)node2->data);
    previousArgument = *(int*)node->data;
  }
  // if we found an odd cycle without any attacker, we can stop right now
  if(occ->arguments_attacking_cycle[occ->number_of_odd_cycles-1]->number_of_elements == 0)
    return TRUE;
  return FALSE;
}

// constructs an OddCycleCollection for the given AAF
struct OddCycleCollection* occ__init(struct AAF* aaf){
  struct OddCycleCollection* occ = malloc(sizeof(struct OddCycleCollection));
  occ->arguments_in_odd_cycles = malloc(sizeof(struct BitSet*));
  bitset__init(occ->arguments_in_odd_cycles,aaf->number_of_arguments);
  bitset__unsetAll(occ->arguments_in_odd_cycles);
  occ->number_of_odd_cycles = 0;
  occ->oddCycles = realloc(0,1);
  occ->arguments_in_individual_odd_cycles = realloc(0,1);
  occ->arguments_attacking_cycle = realloc(0,1);
  // for providing pointers to all arguments
  occ->arguments = malloc(aaf->number_of_arguments * sizeof(int));
  for(int i = 0; i < aaf->number_of_arguments; i++)
    occ->arguments[i] = i;
  // The stack of the depth-first search (a stack of paths)
  struct LinkedList *stack;
  struct LinkedList *path, *new_path;
  // the following data structures are needed to organise the DFS
  // "visited" stores for each arguments whether it has been already visited during the DFS;
  // "pathToStart" stores for visited argument, whether we already found a path back to the
  //    starting argument
  // "oddPathToStart" stores for each visited argument where we found a path to the starting argument,
  //    whether this path has odd length
  // "argToStart" stores for each visited argument where we found a path to the starting argument,
  //    the argument which comes next in that path.
  struct BitSet *visited = malloc(sizeof(struct BitSet));
  struct BitSet *pathToStart = malloc(sizeof(struct BitSet));
  struct BitSet *oddPathToStart = malloc(sizeof(struct BitSet));
  bitset__init(visited,aaf->number_of_arguments);
  bitset__init(pathToStart,aaf->number_of_arguments);
  bitset__init(oddPathToStart,aaf->number_of_arguments);
  int *argToStart = malloc(aaf->number_of_arguments * sizeof(int));
  int unattacked_odd_cycle_found = FALSE;
  for(int i = 0; i < aaf->number_of_arguments; i++){
    // if we already have an odd cycle for that argument,
    // we skip it
    if(bitset__get(occ->arguments_in_odd_cycles,i))
      continue;
    // initialise data structures
    stack = malloc(sizeof(struct LinkedList));
    llist__init(stack);
    path = malloc(sizeof(struct LinkedList));
    llist__init(path);
    llist__add(path,&occ->arguments[i]);
    llist__push(stack,path);
    memset(argToStart,0,aaf->number_of_arguments * sizeof(int));
    bitset__unsetAll(visited);
    bitset__unsetAll(pathToStart);
    bitset__unsetAll(oddPathToStart);
    bitset__set(visited,i);
    int cycleFound = FALSE;
    while(stack->length>0){
      path = (struct LinkedList*) llist__pop(stack);
      for(GSList* node = aaf->children[*(int*)path->tail->data]; node != NULL; node = node->next){
        if(bitset__get(visited,*(int*)node->data)){
          // node already visited
          if(*(int*)node->data == i){
            // node is the starting node -> we have found a cycle (but we ignore loops)
            if(path->length % 2 == 1 && path->length > 1){
              // if cycle is odd we are finished
              unattacked_odd_cycle_found = occ__add_cycle(occ,path,aaf);
              cycleFound = TRUE;
              break;
            }else{
              // cycle is of even length
              // now we can update some information in pathToStart, oddPathToStart,
              // and argToStart
              // We start from the second node in the path
              int isOdd = FALSE;
              for(struct LinkedListNode* node2 = path->root->next; node2 != NULL; node2 = node2->next){
                bitset__set(pathToStart,*(int*)node2->data);
                if(node2->next != NULL)
                  argToStart[*(int*)node2->data] = *(int*)node2->next->data;
                else
                  argToStart[*(int*)node2->data] = i;
                if(isOdd)
                  bitset__set(oddPathToStart,*(int*)node2->data);
                isOdd = !isOdd;
              }
            }
          }else{
            // node has been visited before (but it's not the starting node)
            // check whether the node is not already in the current path
            // and whether we can construct an odd cycle now
            if(bitset__get(pathToStart,*(int*)node->data) && !llist__contains_int(path,*(int*)node->data)){
                // there is a path to the starting node
                if((bitset__get(oddPathToStart,*(int*)node->data) && path->length % 2 == 1) ||
                      (!bitset__get(oddPathToStart,*(int*)node->data) && path->length % 2 == 0)){
                  // either one of two cases:
                  // 1.) we can reach the starting node from node via an odd number of nodes AND
                  //      we can reach node from the starting node via an even number of nodes, or
                  // 2.) we can reach the starting node from node via an even number of nodes AND
                  //      we can reach node from the starting node via an odd number of nodes.
                  // In both cases we found an odd cycle
                  // Now we just have to add the missing nodes to our current path
                  int next_node = *(int*)node->data;
                  while(next_node != i){
                    llist__add(path,&occ->arguments[next_node]);
                    next_node = argToStart[next_node];
                  }
                  occ__add_cycle(occ,path,aaf);
                  cycleFound = TRUE;
                  break;
                }else{
                  // we just found another even cycle
                  // now we can update some information in pathToStart, oddPathToStart,
                  // and argToStart
                  // We start from the second node in the path
                  int isOdd = FALSE;
                  for(struct LinkedListNode* node2 = path->root->next; node2 != NULL; node2 = node2->next){
                    bitset__set(pathToStart,*(int*)node2->data);
                    if(node2->next != NULL)
                      argToStart[*(int*)node2->data] = *(int*)node2->next->data;
                    else
                      argToStart[*(int*)node2->data] = i;
                    if(isOdd)
                      bitset__set(oddPathToStart,*(int*)node2->data);
                    isOdd = !isOdd;
                  }
                }
            }
            // if there is no path to the starting node there is nothing to do
          }
        }else{
          // node not visited before -> extend path and put on stack
          bitset__set(visited,*(int*)node->data);
          new_path = malloc(sizeof(struct LinkedList));
          llist__init(new_path);
          for(struct LinkedListNode* node2 = path->root; node2 != NULL; node2 = node2->next)
            llist__add(new_path,node2->data);
          llist__add(new_path,node->data);
          llist__push(stack, new_path);
        }
      }
      if(cycleFound)
        break;
      else
        // free some space
        llist__destroy_without_data(path);
    }
    // free some space
    while(stack->length > 0)
      llist__destroy_without_data(llist__pop(stack));
    llist__destroy_without_data(stack);
    if(unattacked_odd_cycle_found)
      break;
  }
  // free some variables
  bitset__destroy(visited);
  bitset__destroy(pathToStart);
  bitset__destroy(oddPathToStart);
  free(argToStart);
  if(unattacked_odd_cycle_found)
    return NULL;
  return occ;
}

// returns TRUE iff the given node is contained in some
// odd cycle from occ.
int occ__contains(struct OddCycleCollection* occ, int node){
  return bitset__get(occ->arguments_in_odd_cycles, node);
}

// assuming that node is contained in an odd cycle, this function returns
// the set representation of all attackers of that cycle.
struct RaSet* occ__get_attackers(struct OddCycleCollection* occ, int node){
  for(int i = 0; i < occ->number_of_odd_cycles; i++)
    if(bitset__get(occ->arguments_in_individual_odd_cycles[i], node))
      return occ->arguments_attacking_cycle[i];
  return NULL;
}
// frees memory of an occ
void occ__destroy(struct OddCycleCollection* occ){
  for(int i = 0; i < occ->number_of_odd_cycles; i++)
    llist__destroy_without_listanddata(occ->oddCycles[i]);
  free(occ->arguments);
  free(occ->oddCycles);
  free(occ);
}

// computes the set of strongly connected components using Tarjan's algorithm
// (internal recursive sub function)
int __scc__compute_strongly_connected_components(int idx, int v,struct LinkedList* stack,struct LinkedList* sccs,struct AAF *aaf,int* index,int* lowlink, int* all_arguments){
  index[v] = idx;
	lowlink[v] = idx;
	idx++;
  llist__push(stack,&all_arguments[v]);
  for(GSList* w = aaf->children[v]; w != NULL; w = w->next){
    if(index[*(int*)w->data] == -1){
	     idx = __scc__compute_strongly_connected_components(idx,*(int*)w->data,stack,sccs,aaf,index,lowlink,all_arguments);
    	 lowlink[v] = lowlink[v] > lowlink[*(int*)w->data] ? lowlink[*(int*)w->data] : lowlink[v];
	  }else if(llist__contains_int(stack,*(int*)w->data)){
      lowlink[v] = lowlink[v] > index[*(int*)w->data] ? index[*(int*)w->data] : lowlink[v];
	 	}
	}
  if(lowlink[v] == index[v]){
    struct LinkedList* scc = malloc(sizeof(struct LinkedList));
    llist__init(scc);
    int* w;
    do{
       w = llist__pop(stack);
       llist__add(scc,w);
    }while( v != *w);
    llist__add(sccs,scc);
	}
	return idx;
}

// computes the set of strongly connected components using Tarjan's algorithm
struct LinkedList* scc__compute_strongly_connected_components(struct AAF *aaf, int* all_arguments){
  struct LinkedList* sccs = malloc(sizeof(struct LinkedList));
  llist__init(sccs);
  int idx = 0;
  struct LinkedList* stack = malloc(sizeof(struct LinkedList));
  llist__init(stack);
  int* index = malloc(aaf->number_of_arguments * sizeof(int));
  int* lowlink = malloc(aaf->number_of_arguments * sizeof(int));
    for(int i = 0; i < aaf->number_of_arguments; i++){
    index[i] = -1;
    lowlink [i] = -1;
  }
  for(int i = 0; i < aaf->number_of_arguments; i++)
    if(index[i] == -1)
      idx = __scc__compute_strongly_connected_components(idx,i,stack,sccs,aaf,index,lowlink,all_arguments);
  return sccs;
}

// print the set of strongly connected components
void __scc_print(struct AAF *aaf, struct LinkedList* sccs){
  for(struct LinkedListNode* scc = sccs->root; scc != NULL; scc = scc->next){
    printf("<");
    char isFirst = TRUE;
    for(struct LinkedListNode* v = (*(struct LinkedList*)scc->data).root; v != NULL; v = v->next){
      if(isFirst){
        isFirst = FALSE;
        printf("%s", aaf->ids2arguments[*(int*)v->data]);
      }else printf(",%s", aaf->ids2arguments[*(int*)v->data]);
    }
    printf(">\n");
  }
}
/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
