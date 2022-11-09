/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : taas_inout.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : Utlity functions for taas solvers.
 ============================================================================
 */

/**
 * General solver information
 */
struct SolverInformation{
    char* description;
    char* formats;
    char* problems;
};

/**
 * Task specification
 */
struct TaskSpecification{
  /** The track (problem+semantics) to be solved. */
  char* track;
  /** The problem to be solved. */
  char* problem;
  /** The file path */
  char* file;
  /** The file format */
  char* format;
  /** For DC and DS queries this attribute contains the queried argument*/
  int arg;
  char* argAsString;
  /** additional arguments */
  int number_of_additional_arguments;
  char** additional_keys;
  char** additional_values;
};

/**
 * initialises general solver information
 */
struct SolverInformation* taas__solverinformation(char* description, char* formats, char* problems){
    struct SolverInformation *info = (struct SolverInformation *) malloc(sizeof(struct SolverInformation));
    info->description = description;
    info->formats = formats;
    info->problems = problems;
    return info;
}

void taas__solverinformation_destroy(struct SolverInformation *info){
    free(info);
}

/**
 * Handles the command. If basic solver information is asked for, NULL is returned;
 * otherwise the task specification is returned
 */
struct TaskSpecification* taas__cmd_handle(int argc, char *argv[], struct SolverInformation* info){
  // TODO: add some checks to validate input
  // if no arguments are given just print out the version info
  // parse for a problem
  struct TaskSpecification* task = (struct TaskSpecification*) malloc(sizeof(struct TaskSpecification));
  task->number_of_additional_arguments = 0;
  task->additional_keys = (char**) malloc(sizeof(char*));
  task->additional_values = (char**) malloc(sizeof(char*));
  task->format = NULL;
  int param = 0;
  for(int i = 1; i < argc; i++){
    if(strcmp(argv[i],"-p") == 0){
      task->track = argv[++i];
      param++;
      continue;
    }
    if(strcmp(argv[i],"-f") == 0){
      task->file = argv[++i];
      param++;
      continue;
    }
    if(strcmp(argv[i],"-a") == 0){
      task->argAsString = argv[++i];
      continue;
    }
    if(strcmp(argv[i],"-fo") == 0){
      task->format = argv[++i];
      param++;
      continue;
    }
    // for the parameter "--formats" print out the formats and exit
    if(strcmp(argv[i],"--formats") == 0){
      printf("%s\n", info->formats);
      return NULL;
    }
    // for the parameter "--problems" print out the problems and exit
    if(strcmp(argv[i],"--problems") == 0){
      printf("%s\n", info->problems);
      return NULL;
    }
    // parse an additional argument
    task->number_of_additional_arguments++;
    task->additional_keys = (char **) realloc(task->additional_keys, task->number_of_additional_arguments * sizeof(char*));
    task->additional_values = (char **) realloc(task->additional_values, task->number_of_additional_arguments * sizeof(char*));
    task->additional_keys[task->number_of_additional_arguments-1] = argv[i];
    task->additional_values[task->number_of_additional_arguments-1] = argv[++i];
  }
  //if no problem and file are given, just print out information
  if(param < 2){
    printf("%s\n", info->description);
    return NULL;
  }
  task->problem = (char*) malloc(3*sizeof(char));
  memcpy(task->problem, task->track, 2);
  task->problem[2] = '\0';
  task->arg = -1;
  return task;
}

void taas__cmd_destroy(struct TaskSpecification *task){
  free(task);
}

/**
 * Returns the value of an additional argument; if there is no
 * value with the given key, NULL is returned;
 */
char* taas__task_get_value(struct TaskSpecification *task, char* key){
  for(int i = 0; i < task->number_of_additional_arguments;i++){
    if(strcmp(task->additional_keys[i],key) == 0)
      return task->additional_values[i];
  }
  return NULL;
}

/** Read a file in tgf into the data structures */
void taas__readFile_tgf(char* path, struct AAF* aaf){
  // first get the number of arguments
  FILE* fp = fopen(path,"r");
  char* row = NULL;
  size_t len = 0;
  ssize_t read;
  int idx = 0;
  while((read = getline(&row, &len, fp)) != -1) {
    if(strcmp(trimwhitespace(row),"") == 0)
      continue;
    if(strcmp(trimwhitespace(row),"#") == 0)
      break;
    idx++;
  }
  aaf->number_of_arguments = idx;
  // now do the actual parsing
  aaf->ids2arguments = (char**) malloc(aaf->number_of_arguments * sizeof(char*));
	aaf->children = (GSList**) malloc(aaf->number_of_arguments * sizeof(GSList*));
	aaf->parents = (GSList**) malloc(aaf->number_of_arguments * sizeof(GSList*));
  aaf->arguments2ids = (GHashTable*) g_hash_table_new(g_str_hash, g_str_equal);
  aaf->number_of_attackers = (int*) malloc(aaf->number_of_arguments * sizeof(int));
	fp = fopen(path,"r");
	char* arg1;
	int argumentSection = 1;
	idx = 0;
  while ((read = getline(&row, &len, fp)) != -1) {
    if(strcmp(trimwhitespace(row),"") == 0)
      continue;
		if(strcmp(trimwhitespace(row),"#") == 0){
      // switch section of file
			argumentSection = 0;
			aaf->number_of_arguments = idx;
      aaf->number_of_attacks = 0;
			aaf->initial = (struct BitSet*) malloc(sizeof(struct BitSet));
			bitset__init(aaf->initial, aaf->number_of_arguments);
      // all bits initially one
      bitset__setAll(aaf->initial);
      aaf->loops = (struct BitSet*) malloc(sizeof(struct BitSet));
      bitset__init(aaf->loops, aaf->number_of_arguments);
      // all bits initially zero
      bitset__unsetAll(aaf->loops);
			continue;
		}
		if(argumentSection != 0){
      // parse an argument
      arg1 = (char*) malloc(sizeof(trimwhitespace(row))+1);
      strcpy(arg1,trimwhitespace(row));
			aaf->ids2arguments[idx] = arg1;
      int* sidx = (int*) malloc(sizeof(int));
      *sidx = idx;
      g_hash_table_insert(aaf->arguments2ids,arg1,sidx);
			aaf->children[idx] = NULL;
			aaf->parents[idx] = NULL;
      aaf->number_of_attackers[idx] = 0;
			idx++;
		}else{
      // parse an attack
      aaf->number_of_attacks++;
			idx = 0;
			while(row[idx] != ' ')idx++;
			row[idx] = 0;
      int* idx1 = (int*) g_hash_table_lookup(aaf->arguments2ids, row);
			int* idx2 = (int*) g_hash_table_lookup(aaf->arguments2ids, &row[idx+1]);
      aaf->children[*idx1] = g_slist_prepend(aaf->children[*idx1], idx2);
      aaf->parents[*idx2] = g_slist_prepend(aaf->parents[*idx2], idx1);
      aaf->number_of_attackers[*idx2]++;
      // if an argument is attacked, it is not initial
			bitset__unset(aaf->initial,*idx2);
      // check for self-attacking arguments
      if(*idx1 == *idx2){
        bitset__set(aaf->loops,*idx1);
      }
		}
	}
  fclose(fp);
}

/** Read a file in ICCMA23 format into the data structures */
void taas__readFile_i23(char* path, struct AAF* aaf){
  // first get the number of arguments
  FILE* fp = fopen(path,"r");
  char* row = NULL;
  size_t len = 0;
  ssize_t read;
  while((read = getline(&row, &len, fp)) != -1) {
    if(strncmp(trimwhitespace(row),"#",1) == 0)
      continue;
    if(strncmp(trimwhitespace(row),"p af",4) == 0){
      aaf->number_of_arguments = atoi(&(trimwhitespace(row)[4]));
      break;
    }
  }
  // now do the actual parsing
  aaf->ids2arguments = (char**) malloc(aaf->number_of_arguments * sizeof(char*));
	aaf->children = (GSList**) malloc(aaf->number_of_arguments * sizeof(GSList*));
	aaf->parents = (GSList**) malloc(aaf->number_of_arguments * sizeof(GSList*));
  aaf->arguments2ids = (GHashTable*) g_hash_table_new(g_str_hash, g_str_equal);
  aaf->number_of_attackers = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  aaf->number_of_attacks = 0;
  aaf->initial = (struct BitSet*) malloc(sizeof(struct BitSet));
  bitset__init(aaf->initial, aaf->number_of_arguments);
  // all bits initially one
  bitset__setAll(aaf->initial);
  aaf->loops = (struct BitSet*) malloc(sizeof(struct BitSet));
  bitset__init(aaf->loops, aaf->number_of_arguments);
  // all bits initially zero
  bitset__unsetAll(aaf->loops);
  // initialise arguments
  for(int idx = 0; idx < aaf->number_of_arguments; idx++){
    char *arg1 = (char*) malloc(sizeof(char)*(int)std::log10(idx+1)+2);
    sprintf(arg1, "%d", idx+1);
    aaf->ids2arguments[idx] = arg1;
    int* sidx = (int*) malloc(sizeof(int));
    *sidx = idx;
    g_hash_table_insert(aaf->arguments2ids,arg1,sidx);
    aaf->children[idx] = NULL;
    aaf->parents[idx] = NULL;
    aaf->number_of_attackers[idx] = 0;
  }
	while ((read = getline(&row, &len, fp)) != -1) {
    if(strcmp(trimwhitespace(row),"") == 0)
      continue;
		if(strncmp(trimwhitespace(row),"#",1) == 0)
      continue;
    // parse an attack
    aaf->number_of_attacks++;
		int idx = 0;
		while(row[idx] != ' ')idx++;
		row[idx] = 0;
    int* idx1 = (int*) malloc(sizeof(int));
    int* idx2 = (int*) malloc(sizeof(int));
    *idx1 = atoi(row)-1;
		*idx2 = atoi(&row[idx+1])-1;
    aaf->children[*idx1] = g_slist_prepend(aaf->children[*idx1], idx2);
    aaf->parents[*idx2] = g_slist_prepend(aaf->parents[*idx2], idx1);
    aaf->number_of_attackers[*idx2]++;
    // if an argument is attacked, it is not initial
		bitset__unset(aaf->initial,*idx2);
    // check for self-attacking arguments
    if(*idx1 == *idx2){
      bitset__set(aaf->loops,*idx1);
    }
	}
  fclose(fp);
}
// if DS or DC problem, parse argument under consideration
void taas__update_arg_param(struct TaskSpecification* task, struct AAF* aaf){
  if(strcmp(task->problem,"DS") == 0 || strcmp(task->problem,"DC") == 0)
    task->arg = * (int*) g_hash_table_lookup(aaf->arguments2ids, trimwhitespace(task->argAsString));
}
/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
