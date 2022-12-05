/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_ds-pr.cpp
 Author      : Matthias Thimm, Federico Cerutti
 Version     : 2.0
 Copyright   : GPL3
 Description : solve functions for DS-PR (deciding skeptical acceptance
               wrt. preferred semantics)
 ============================================================================
 */

// the fudge approach for DS-PR
bool solve_dspr(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded, bool do_print = true){
  // solver_admTest is used for checking whether a single set can be extended
  //       to an admissible set
  // solver_attAdmTest is used for checking whether there is an admissible set
  //       attacking another admissible set
  ExternalSolver solver_admTest;
  sat__init(solver_admTest, 2*aaf->number_of_arguments,taas__task_get_value(task,"-sat"));
  ExternalSolver solver_attAdmTest;
  sat__init(solver_attAdmTest, 4*aaf->number_of_arguments+aaf->number_of_attacks,taas__task_get_value(task,"-sat"));
  // initialise variables
  int* in_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int* out_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int* in_attacked_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int* out_attacked_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int idx = 1;
  for(int i = 0; i < aaf->number_of_arguments;i++){
    in_vars[i] = idx++;
    out_vars[i] = idx++;
  }
  // add admissibility clauses
  add_admTestClauses(solver_admTest,in_vars,out_vars,aaf,grounded);
  for(int i = 0; i < aaf->number_of_arguments;i++){
      in_attacked_vars[i] = idx++;
      out_attacked_vars[i] = idx++;
  }
  // initialise solver_attAdmTest
  add_admTestClauses(solver_attAdmTest,in_vars,out_vars,aaf,grounded);
  add_admTestClauses(solver_attAdmTest,in_attacked_vars,out_attacked_vars,aaf,grounded);
  // add constraints for modelling the attack to the other set
  add_attackClauses(solver_attAdmTest,in_vars,in_attacked_vars,4*aaf->number_of_arguments + 1,aaf,grounded);
  // check if there is an admissible labelling setting task->arg IN
  sat__assume(solver_admTest,in_vars[task->arg]);
  int sat = sat__solve(solver_admTest);
  if(sat == 20){
      if(do_print){
        printf("NO\n");
        if(PRINT_WITNESS){
          // just print any preferred extension
          solve_sepr(task,aaf,grounded);
        }
      }
      return false;
  }
  for(int i = 0; i < aaf->number_of_arguments; i++){
    if(sat__get(solver_admTest,in_vars[i]) < 0){
      sat__add(solver_attAdmTest,in_vars[i]);
    }
  }
  sat__add(solver_attAdmTest,0);
  // check if there is an admissible labelling attacking task->arg
  sat__assume(solver_admTest,out_vars[task->arg]);
  sat = sat__solve(solver_admTest);
  if(sat == 10){
      if(do_print){
        printf("NO\n");
        // find preferred extension
        struct RaSet* initial_admSet = raset__init_empty(aaf->number_of_arguments);
        for(int i = 0; i < aaf->number_of_arguments; i++)
          if(sat__get(solver_admTest,in_vars[i]) > 0)
              raset__add(initial_admSet,i);
        solve_sepr(task,aaf,grounded,initial_admSet);
      }
      return false;
  }
  // main loop
  sat__addClause1(solver_attAdmTest,in_attacked_vars[task->arg]);
  while(true){
    sat = sat__solve(solver_attAdmTest);
    if(sat == 20){
        if(do_print)
          printf("YES\n");
        return true;
    }
    sat__assume(solver_admTest,in_vars[task->arg]);
    for(int i = 0; i < aaf->number_of_arguments; i++){
      if(sat__get(solver_attAdmTest,in_vars[i]) > 0){
        sat__assume(solver_admTest,in_vars[i]);
      }
    }
    sat = sat__solve(solver_admTest);
    if(sat == 20){
      if(do_print){
        printf("NO\n");
        if(PRINT_WITNESS){
          // we have to find a preferred extension as a witness, starting
          // from the admissible set encoded in solver_attAdmTest
          struct RaSet* initial_admSet = raset__init_empty(aaf->number_of_arguments);
          for(int i = 0; i < aaf->number_of_arguments; i++)
            if(sat__get(solver_attAdmTest,in_vars[i]) > 0)
              raset__add(initial_admSet,i);
          solve_sepr(task,aaf,grounded,initial_admSet);
        }
      }
      return false;
    }
    for(int i = 0; i < aaf->number_of_arguments; i++){
      if(sat__get(solver_admTest,in_vars[i]) < 0){
        sat__add(solver_attAdmTest,in_vars[i]);
      }
    }
    sat__add(solver_attAdmTest,0);
  }
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
