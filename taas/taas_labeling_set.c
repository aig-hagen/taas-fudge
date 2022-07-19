/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : taas_labeling_set.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : LabelingSet is a binary decision diagram for representing sets of
               labelings/extensions.
 ============================================================================
*/

#define YES_NODE 1
#define NO_NODE 2
#define ARG_NODE 3

/**
 * A single node in the binary decision diagram representation
 * of a set of labelings
 */
struct LabelingSetNode{
  // Links to the part where the argument of this node is IN
  struct LabelingSetNode* inArc;
  // Links to the part where the argument of this node is OUT/UNDECIDED
  struct LabelingSetNode* outArc;
  // the argument of this node
  int argument;
  // the type of this node (YES_NODE, NO_NODE, or ARG_NODE)
  char type;
};

/**
 * A set of labelings represented as a binary decision
 * diagram (BDD)
 */
struct LabelingSet{
  // the root of the BDD
  struct LabelingSetNode* root;
  // special nodes
  struct LabelingSetNode* yesNode;
  struct LabelingSetNode* noNode;
};

/**
 * Initialises an empty labeling set
 */
struct LabelingSet* taas__labset_init_empty(){
  struct LabelingSet* labset = malloc(sizeof(struct LabelingSet));
  labset->noNode = malloc(sizeof(struct LabelingSetNode));
  labset->noNode->type = NO_NODE;
  labset->yesNode = malloc(sizeof(struct LabelingSetNode));
  labset->yesNode->type = YES_NODE;
  labset->root = labset->noNode;
  return labset;
}

/**
 * Checks whether there is a labeling L in this set such that the set of
 * all arguments labelled IN by L is a superset of the arguments labelled IN in
 * lab.
 */
char taas__labset_subcontains_rec(struct LabelingSet* labset, struct Labeling* lab, struct LabelingSetNode* current){
  if(current == labset->yesNode)
    return TRUE;
  if(current == labset->noNode)
    return FALSE;
  if(bitset__get(lab->in,current->argument))
    return taas__labset_subcontains_rec(labset,lab,current->inArc);
  return taas__labset_subcontains_rec(labset,lab,current->inArc) || taas__labset_subcontains_rec(labset,lab,current->outArc);
}
char taas__labset_subcontains(struct LabelingSet* labset, struct Labeling* lab){
  return taas__labset_subcontains_rec(labset,lab,labset->root);
}

/**
 * Adds the given labeling to this set.
 */
void taas__labset_add(struct LabelingSet* labset, struct Labeling* lab){
  if(labset->root == labset->noNode){
    // special case: labset is empty
    labset->root = malloc(sizeof(struct LabelingSetNode));
    struct LabelingSetNode* parent = NULL;
    struct LabelingSetNode* current = labset->root;
    for(int arg = 0; arg < lab->in->length; arg++){
      current->argument = arg;
      current->type = ARG_NODE;
      parent = current;
      if(bitset__get(lab->in,arg)){
        current->outArc = labset->noNode;
        current->inArc = malloc(sizeof(struct LabelingSetNode));
        current = current->inArc;
      }else{
        current->inArc = labset->noNode;
        current->outArc = malloc(sizeof(struct LabelingSetNode));
        current = current->outArc;
      }
    }
    // take back last step and assign yesNode
    if(parent->inArc == labset->noNode){
      free(parent->outArc);
      parent->outArc = labset->yesNode;
    }else{
      free(parent->inArc);
      parent->inArc = labset->yesNode;
    }
  }else{
    // general case, already an element in the set
    struct LabelingSetNode* parent = NULL;
    struct LabelingSetNode* current = labset->root;
    do{
      parent = current;
      if(bitset__get(lab->in, current->argument))
        current = current->inArc;
      else
        current = current->outArc;
    }while(current != labset->noNode && current!= labset->yesNode);
    if(current == labset->yesNode){
      //should not happen: labeling already in the set
      return;
    }
    if(parent->inArc == labset->noNode){
      parent->inArc = malloc(sizeof(struct LabelingSetNode));
      current = parent->inArc;
    }else{
      parent->outArc = malloc(sizeof(struct LabelingSetNode));
      current = parent->outArc;
    }
    for(int arg = parent->argument+1; arg < lab->in->length; arg++){
      current->argument = arg;
      current->type = ARG_NODE;
      parent = current;
      if(bitset__get(lab->in, current->argument)){
        current->outArc = labset->noNode;
        current->inArc = malloc(sizeof(struct LabelingSetNode));
        current = current->inArc;
      }else{
        current->inArc = labset->noNode;
        current->outArc = malloc(sizeof(struct LabelingSetNode));
        current = current->outArc;
      }
    }
    // take back last step and assign yesNode
    if(parent->inArc == labset->noNode){
      free(parent->outArc);
      parent->outArc = labset->yesNode;
    }else{
      free(parent->inArc);
      parent->inArc = labset->yesNode;
    }
  }
}

void __print_labeling_set_rec(struct LabelingSet* labset, struct LabelingSetNode* current, int indent, char in_out_root){
  for(int i =0; i < indent; i++)
    printf("|  ");
  if(in_out_root == 0)
    printf("-IN -> ");
  else if(in_out_root == 1)
    printf("-OUT -> ");
  if(current == labset->yesNode)
    printf("YES\n");
  else if(current == labset->noNode)
    printf("NO\n");
  else{
    printf("%i\n", current->argument);
    __print_labeling_set_rec(labset,current->inArc,indent+1,0);
    __print_labeling_set_rec(labset,current->outArc,indent+1,1);
  }
}

void __print_labeling_set(struct LabelingSet* labset){
  __print_labeling_set_rec(labset, labset->root, 0, -1);
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
