/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_dc-co.cpp
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : solve function for DC-ST
 ============================================================================
 */

bool solve_dcst(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded, bool do_print = true){
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
  // check if there is a stable extension containing the argument
  sat__assume(solver,in_vars[task->arg]);
  int sat = sat__solve(solver);
  if(sat == 20){
      if(do_print)
        printf("NO\n");
      return false;
  }
  if(do_print)
    printf("YES\n");
  if(PRINT_WITNESS && do_print){
    printf("w ");
    for(int i = 0; i < aaf->number_of_arguments; i++){
      if(sat__get(solver,in_vars[i]) > 0){
        printf("%d ",(i+1));
      }
    }
    printf("\n");
  }
  return true;
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
