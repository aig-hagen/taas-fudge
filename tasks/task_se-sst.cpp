/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_ds-sst.cpp
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : solve functions for SE-SST
 ============================================================================
 */

// the fudge approach for SE-SST
void solve_sesst(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){    
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
  // is also semi-stable
  if(all_grounded){
    sat__free(solver);
    printf("%s\n",taas__lab_print_i23(grounded,aaf));
    return;
  }
  // add a clause imposing that at least one argument is not undecided
  for(int i = 0; i < aaf->number_of_arguments; i++){
    sat__add(solver,in_vars[i]);
    sat__add(solver,out_vars[i]);
  }
  sat__add(solver,0);
  // the current set of arguments that are not undecided
  struct RaSet* notUndec = raset__init_empty(aaf->number_of_arguments);
  // the previous set of arguments in the extension
  struct RaSet* in_arg = raset__init_empty(aaf->number_of_arguments);
  // a temp set
  struct RaSet* temp = raset__init_empty(aaf->number_of_arguments);
  int sat;
  int* clause = (int*) malloc(2*aaf->number_of_arguments * sizeof(int));
  while(true){
      sat = sat__solve(solver);
      if(sat == 20)
        break;
      int idx = 0;
      raset__reset(in_arg);
      for(int i = 0; i < aaf->number_of_arguments; i++){
          if(!raset__contains(notUndec,i) && (sat__get(solver,in_vars[i]) > 0 || sat__get(solver,out_vars[i]) > 0)){
              raset__add(temp,i);
          }else if (sat__get(solver,in_vars[i]) < 0 && sat__get(solver,out_vars[i]) < 0){
            clause[idx++] = in_vars[i];
            clause[idx++] = out_vars[i];
          }
          if(sat__get(solver,in_vars[i]) > 0)
            raset__add(in_arg,i);
      }
      sat__addClause(solver,clause,idx);
      for(int i = 0; i < temp->number_of_elements; i++ ){
        raset__add(notUndec,temp->elements_arr[i]);
        sat__addClause2(solver,in_vars[temp->elements_arr[i]],out_vars[temp->elements_arr[i]]);
      }
      raset__reset(temp);
  }
  free(clause);
  raset__print_i23(in_arg,aaf->ids2arguments);
  raset__destroy(notUndec);
  raset__destroy(in_arg);
  raset__destroy(temp);
  sat__free(solver);
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
