/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_dc-co.cpp
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : solve function for DC-CO
 ============================================================================
 */

void solve_dcco(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){
  ExternalSolver solver_admTest;
  sat__init(solver_admTest, (2*aaf->number_of_arguments)+1,taas__task_get_value(task,"-sat"));
  // initialise variables
  int* in_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int* out_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int idx = 1;
  for(int i = 0; i < aaf->number_of_arguments;i++){
    in_vars[i] = idx++;
    out_vars[i] = idx++;
  }
  // add admissibility clauses
  add_admTestClauses(solver_admTest,in_vars,out_vars,aaf,grounded);
  // check if there is an admissible labelling setting task->arg IN
  // (then it is also in a complete extension)
  sat__assume(solver_admTest,in_vars[task->arg]);
  int sat = sat__solve(solver_admTest);
  if(sat == 20){
      printf("NO\n");
      return;
  }
  printf("YES\n");
  return;
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
