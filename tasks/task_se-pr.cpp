/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_ds-pr.cpp
 Author      : Matthias Thimm, Federico Cerutti
 Version     : 2.0
 Copyright   : GPL3
 Description : solve functions for SE-PR
 ============================================================================
 */

// the fudge approach for SE-PR
void solve_sepr(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded, struct RaSet* initial_admSet = NULL){
  ExternalSolver solver;
  sat__init(solver, 2*aaf->number_of_arguments,taas__task_get_value(task,"-sat"));
  // initialise variables
  int* in_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int* out_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int idx = 1;
  for(int i = 0; i < aaf->number_of_arguments;i++){
    in_vars[i] = idx++;
    out_vars[i] = idx++;
  }
  // add admissibility clauses
  bool all_grounded = add_admTestClauses(solver,in_vars,out_vars,aaf,grounded);
  // if the grounded extension already attacks all remaining arguments, it
  // is also preferred
  if(all_grounded){
    sat__free(solver);
    printf("%s\n",taas__lab_print_i23(grounded,aaf));
    return;
  }
  // add a clause imposing that at least one argument is in the set
  for(int i = 0; i < aaf->number_of_arguments; i++){
    if(initial_admSet == NULL || !raset__contains(initial_admSet,i))
      sat__add(solver,in_vars[i]);
  }
  sat__add(solver,0);
  // the current set
  struct RaSet* admSet;
  if (initial_admSet == NULL){
    admSet = raset__init_empty(aaf->number_of_arguments);
  }else{
     admSet = initial_admSet;
     // add clauses imposing that at arguments in the initial admissible
     // set are in
     for(int i = 0; i < initial_admSet->number_of_elements; i++ ){
       sat__addClause1(solver,in_vars[initial_admSet->elements_arr[i]]);
     }
  }
  // a temp set
  struct RaSet* temp = raset__init_empty(aaf->number_of_arguments);
  int sat;
  int* clause = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  while(true){
      sat = sat__solve(solver);
      if(sat == 20)
        break;
      int idx = 0;
      for(int i = 0; i < aaf->number_of_arguments; i++){
          if(!raset__contains(admSet,i) && sat__get(solver,in_vars[i]) > 0){
              raset__add(temp,i);
          }else if (sat__get(solver,in_vars[i]) < 0){
            clause[idx++] = in_vars[i];
          }
      }
      sat__addClause(solver,clause,idx);
      for(int i = 0; i < temp->number_of_elements; i++ ){
        raset__add(admSet,temp->elements_arr[i]);
        sat__addClause1(solver,in_vars[temp->elements_arr[i]]);
      }
      raset__reset(temp);
  }
  free(clause);
  raset__print_i23(admSet,aaf->ids2arguments);
  raset__destroy(admSet);
  raset__destroy(temp);
  sat__free(solver);
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
