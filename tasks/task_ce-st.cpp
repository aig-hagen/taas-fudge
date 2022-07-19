/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_ce-st.cpp
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : solve function for CE-ST
 ============================================================================
 */

void solve_cest(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){
  ExternalSolver solver;
  sat__init(solver, aaf->number_of_arguments+1,taas__task_get_value(task,"-sat"));
  // initialise variables
  int* in_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int idx = 1;
  for(int i = 0; i < aaf->number_of_arguments;i++){
    in_vars[i] = idx++;
  }
  // add stability clauses
  add_stbTestClauses(solver,in_vars,aaf,grounded);
  int stb_cnt = 0;
  int sat;
  int clause_idx;
  int* clause = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  while(true){
    // check if there is a stable extension
    int sat = sat__solve(solver);
    if(sat == 10){
      stb_cnt++;
      // add constraint for next iteration
      clause_idx = 0;
      for(int i = 0; i < aaf->number_of_arguments; i++){
        if(sat__get(solver,in_vars[i]) < 0){
          clause[clause_idx++] = in_vars[i];
          //printf("X: %s\n", aaf->ids2arguments[i]);
        }
      }
      if(clause_idx == 0)
        break;
      else sat__addClause(solver,clause,clause_idx);
    }else break;
  }
  // print number of found extensions
  printf("%i\n", stb_cnt);
  free(clause);
  return;
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
