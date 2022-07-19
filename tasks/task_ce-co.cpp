/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_ce-co.cpp
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : solve function for CE-CO
 ============================================================================
 */

void solve_ceco(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){
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
  // add completeness clauses
  add_comTestClauses(solver,in_vars,out_vars,aaf,grounded);
  int co_cnt = 0;
  int sat;
  int clause_idx;
  int* clause = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  while(true){
    // check if there is a complete extension
    int sat = sat__solve(solver);
    if(sat == 10){
      co_cnt++;
      // add constraint for next iteration
      clause_idx = 0;
      for(int i = 0; i < aaf->number_of_arguments; i++){
        if(sat__get(solver,in_vars[i]) > 0){
          clause[clause_idx++] = -in_vars[i];
        }else{
          clause[clause_idx++] = in_vars[i];
        }
      }
      if(clause_idx == 0)
        break;
      else sat__addClause(solver,clause,clause_idx);
    }else break;
  }
  // print number of found extensions
  printf("%i\n", co_cnt);
  free(clause);
  return;
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
