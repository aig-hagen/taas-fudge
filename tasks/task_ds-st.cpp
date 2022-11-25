/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_ds-st.cpp
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : solve function for DS-ST
 ============================================================================
 */

void solve_dsst(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){
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
  // first, check whether there is any stable extension
  int sat = sat__solve(solver);
  if(sat == 20){
      // as there is no stable extension, every argument is skeptically accepted
      printf("YES\n");
      return;
  }
  // check whether the argument is in the grounded extension
  // in that case, we are finished
  if(bitset__get(grounded->in,task->arg)){
      printf("YES\n");
      return;
  }
  // check whether the argument attacked by the grounded extension
  // in that case, we are finished and the above extension
  // already does not contain the argument
  if(bitset__get(grounded->out,task->arg)){
      printf("NO\n");
      if(PRINT_WITNESS){
        printf("w ");
        for(int i = 0; i < aaf->number_of_arguments; i++){
          if(sat__get(solver,in_vars[i]) > 0){
            printf("%d ",(i+1));
          }
        }
        printf("\n");
      }
      return;
  }
  // now assume that the argument is out
  sat__assume(solver,-in_vars[task->arg]);
  sat = sat__solve(solver);
  if(sat == 20){
      printf("YES\n");
      return;
  }
  printf("NO\n");
  if(PRINT_WITNESS){
    printf("w ");
    for(int i = 0; i < aaf->number_of_arguments; i++){
      if(sat__get(solver,in_vars[i]) > 0){
        printf("%d ",(i+1));
      }
    }
    printf("\n");
  }
  return;
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
