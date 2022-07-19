/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : taas_labeling.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : Labeling struct and functions for taas solvers.
 ============================================================================
 */

#define LAB_IN 1
#define LAB_OUT 2
#define LAB_UNDEC 4
#define LAB_UNLABELED -1

/**
 * Struct for (partial) labelings
 * Arguments not mentioned in either in or out
 * are either undecided or not included.
 */
struct Labeling{
  // Arguments that are in
  struct BitSet *in;
  // Arguments that are out
  struct BitSet *out;
  // whether this is a two-valued labeling
  // (just values IN and OUT); in this case
  // the bitset "out" is ignored
  char twoValued;
};

/*
 * Initialises a labeling.
 */
void taas__lab_init(struct Labeling* lab, char twoValued){
  lab->in = (struct BitSet*) malloc(sizeof(struct BitSet));
  lab->out = (struct BitSet*) malloc(sizeof(struct BitSet));
  lab->twoValued = twoValued;
}

/**
 * Randomly sets the given labeling
 */
void taas__lab_randomize(struct Labeling* lab){
  bitset__randomize(lab->in);
  if(!lab->twoValued){
    bitset__randomize(lab->out);
    for(int i = 0; i < lab->in->num_elements; i++)
      lab->out->data[i] = lab->out->data[i] & ~lab->in->data[i];
  }
}

/**
 * Returns the label of the given argument.
 */
int taas__lab_get_label(struct Labeling* lab, int arg){
  if(lab->twoValued){
    if(bitset__get(lab->in,arg))
      return LAB_IN;
    else return LAB_OUT;
  }
  if(bitset__get(lab->in,arg) && !bitset__get(lab->out,arg))
    return LAB_IN;
  if(!bitset__get(lab->in,arg) && bitset__get(lab->out,arg))
    return LAB_OUT;
  if(!bitset__get(lab->in,arg) && !bitset__get(lab->out,arg))
    return LAB_UNDEC;
  return LAB_UNLABELED;
}

/**
 * Sets the label of the given argument.
 */
void taas__lab_set_label(struct Labeling* lab, int arg, int label){
  if(label == LAB_IN){
      bitset__set(lab->in,arg);
      bitset__unset(lab->out,arg);
      return;
  }
  if(label == LAB_OUT){
    bitset__unset(lab->in,arg);
    bitset__set(lab->out,arg);
    return;
  }
  if(label == LAB_UNDEC){
    bitset__unset(lab->in,arg);
    bitset__unset(lab->out,arg);
    return;
  }
  bitset__set(lab->in,arg);
  bitset__set(lab->out,arg);
  return;
}

/**
 * gives a string representation of the labeling in the form
 * "[a1,...,an]" where a1,...,an are the in-labeled arguments.
 */
char* taas__lab_print(struct Labeling* lab, struct AAF* aaf){
  int len = 100;
  char* str = (char*) malloc(len);
  int sidx = 0;
  str[sidx++] = '[';
  int isFirst = 1;
  for(int idx = bitset__next_set_bit(lab->in,0); idx != -1 ; idx = bitset__next_set_bit(lab->in, idx+1)){
    // if there is also a bit set in lab->out it means the argument is unlabeled, so skip it
    if(!lab->twoValued && bitset__get(lab->out,idx))
      continue;
    if(sidx + strlen(aaf->ids2arguments[idx]) + 4 > len){
      len += 100;
      str = (char*) realloc(str, len);
    }
    if(isFirst != 0){
      strcpy(&str[sidx],aaf->ids2arguments[idx]);
      sidx += strlen(aaf->ids2arguments[idx]);
      isFirst = 0;
    } else{
      str[sidx++] = ',';
      strcpy(&str[sidx],aaf->ids2arguments[idx]);
      sidx += strlen(aaf->ids2arguments[idx]);
    }
  }
  str[sidx++] = ']';
  str[sidx] = '\0';
  return str;
}

/**
 * gives a string representation of the labeling in the form
 * "[a1=l1,...,an=ln]" where a1,...,an are all arguments and li is t
 */
char* taas__lab_print_as_labeling(struct Labeling* lab, struct AAF* aaf){
  int len = 100;
  char* str = (char*) malloc(len);
  int sidx = 0;
  str[sidx++] = '[';
  int isFirst = 1;
  for(int idx = 0; idx < aaf->number_of_arguments; idx++){
    if(sidx + strlen(aaf->ids2arguments[idx]) + 6 > len){
      len += 100;
      str = (char*) realloc(str, len);
    }
    if(isFirst != 0){
      strcpy(&str[sidx],aaf->ids2arguments[idx]);
      sidx += strlen(aaf->ids2arguments[idx]);
      str[sidx++] = '=';
      if(taas__lab_get_label(lab,idx) == LAB_IN)
        str[sidx++] = 'I';
      else if(taas__lab_get_label(lab,idx) == LAB_OUT)
        str[sidx++] = 'O';
      else if(taas__lab_get_label(lab,idx) == LAB_UNDEC)
        str[sidx++] = 'U';
      else
        str[sidx++] = 'X';
      isFirst = 0;
    } else{
      str[sidx++] = ',';
      strcpy(&str[sidx],aaf->ids2arguments[idx]);
      sidx += strlen(aaf->ids2arguments[idx]);
      str[sidx++] = '=';
      if(taas__lab_get_label(lab,idx) == LAB_IN)
        str[sidx++] = 'I';
      else if(taas__lab_get_label(lab,idx) == LAB_OUT)
        str[sidx++] = 'O';
      else if(taas__lab_get_label(lab,idx) == LAB_UNDEC)
        str[sidx++] = 'U';
      else
        str[sidx++] = 'X';
    }
  }
  str[sidx++] = ']';
  str[sidx] = '\0';
  return str;
}

/*
 * Destroys a labeling
 */
void taas__lab_destroy(struct Labeling* lab){
  bitset__destroy(lab->in);
  if(!lab->twoValued)
    bitset__destroy(lab->out);
  else free(lab->out);
  free(lab);
}

 /* ============================================================================================================== */
 /* == END FILE ================================================================================================== */
 /* ============================================================================================================== */
