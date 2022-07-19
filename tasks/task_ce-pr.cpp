/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_ce-pr.cpp
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : solve function for CE-PR
 ============================================================================
 */

void solve_cepr(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){
  ExternalSolver solver;
  sat__init(solver, (2*aaf->number_of_arguments)+1,taas__task_get_value(task,"-sat"));
  // initialise variables
  int* in_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int* out_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int idx = 1;
  for(int i = 0; i < aaf->number_of_arguments;i++){
    in_vars[i] = idx++;
    out_vars[i] = idx++;
  }
  // add admissibility clauses
  add_admTestClauses(solver,in_vars,out_vars,aaf,grounded);
  // add a clause imposing that at least one argument is in the set
  for(int i = 0; i < aaf->number_of_arguments; i++){
    sat__add(solver,in_vars[i]);
  }
  sat__add(solver,0);
  int cnt_pr = 0;
  // the current set
  struct RaSet* admSet = raset__init_empty(aaf->number_of_arguments);
  // a temp set
  struct RaSet* temp = raset__init_empty(aaf->number_of_arguments);
  int sat;
  int* clause = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  char noFurther;
  while(true){
    raset__reset(admSet);
    noFurther = TRUE;
    while(true){
        raset__reset(temp);
        sat = sat__solve(solver);
        if(sat == 20){
          break;
        }
        noFurther = FALSE;
        idx = 0;
        for(int i = 0; i < aaf->number_of_arguments; i++){
            if(sat__get(solver,in_vars[i]) > 0){
                raset__add(temp,i);
            }else if (sat__get(solver,in_vars[i]) < 0){
              clause[idx++] = in_vars[i];
            }
        }
        sat__addClause(solver,clause,idx);
        for(int i = 0; i < temp->number_of_elements; i++ ){
          raset__add(admSet,temp->elements_arr[i]);
          sat__assume(solver,in_vars[temp->elements_arr[i]]);
        }
    }
    if(noFurther)
      break;
    cnt_pr++;
    idx = 0;
    for(int i = 0; i < aaf->number_of_arguments; i++){
        if(!raset__contains(admSet,i)){
          clause[idx++] = in_vars[i];
        }
    }
    sat__addClause(solver,clause,idx);
  }
  free(clause);
  // if no non-empty preferred extension has been found, the empty
  // set is the only preferred extension
  if(cnt_pr == 0)
    cnt_pr = 1;
  printf("%i\n",cnt_pr);
  raset__destroy(admSet);
  raset__destroy(temp);
  sat__free(solver);
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
