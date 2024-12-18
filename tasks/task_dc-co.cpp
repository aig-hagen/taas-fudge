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

bool solve_dcco(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded, bool do_print = true){
  IpasirSolver solver_comTest;
  sat__init(solver_comTest, (2*aaf->number_of_arguments)+1);
  // initialise variables
  int* in_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int* out_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int idx = 1;
  for(int i = 0; i < aaf->number_of_arguments;i++){
    in_vars[i] = idx++;
    out_vars[i] = idx++;
  }
  // add completeness clauses
  add_comTestClauses(solver_comTest,in_vars,out_vars,aaf,grounded);
  // check if there is a complete labelling setting task->arg IN
  sat__assume(solver_comTest,in_vars[task->arg]);
  int sat = sat__solve(solver_comTest);
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
      if(sat__get(solver_comTest,in_vars[i]) > 0){
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
