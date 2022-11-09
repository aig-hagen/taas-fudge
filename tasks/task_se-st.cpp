/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_se-st.cpp
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : solve function for SE-ST
 ============================================================================
 */

void solve_sest(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){
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
  // check if there is a stable extension
  int sat = sat__solve(solver);
  if(sat == 20){
      printf("NO\n");
      return;
  }
  // print extension
  printf("w ");
  for(int i = 0; i < aaf->number_of_arguments; i++){
    if(sat__get(solver,in_vars[i]) > 0)
      printf("%s ",aaf->ids2arguments[i]);          
  }
  printf("\n");
  return;
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
