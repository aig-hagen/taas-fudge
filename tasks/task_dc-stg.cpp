/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_dc-stg.cpp
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : solve function for DC-STG
 ============================================================================
 */

bool solve_dcstg(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded, bool do_print = true){
  IpasirSolver solver;
  sat__init(solver, 2*aaf->number_of_arguments);
  // initialise variables
  int* in_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int* out_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
  int idx = 1;
  for(int i = 0; i < aaf->number_of_arguments;i++){
    in_vars[i] = idx++;
    out_vars[i] = idx++;
  }
  // add conflict-free clauses
  bool all_grounded = add_cfTestClauses(solver,in_vars,out_vars,aaf,grounded);
  // if the grounded extension already attacks all remaining arguments, it
  // is also the only stage extension
  if(all_grounded){
    sat__free(solver);
    if(bitset__get(grounded->in,task->arg)){
      if(do_print){
        printf("YES\n");
        printf("%s\n",taas__lab_print_i23(grounded,aaf));
      }
      return true;
    }else{
      if(do_print){
        printf("NO\n");
        printf("%s\n",taas__lab_print_i23(grounded,aaf));
      }
      return false;
    }
  }
  // main loop
  while(true){
      // assume arg is in
      sat__assume(solver,in_vars[task->arg]);
      int sat = sat__solve(solver);
      if(sat == 20){
          // no conflict-free set containing arg could be found, answer is NO
          if(do_print)
            printf("NO\n");
          return false;
      }
      // now maximise UNDEC of the previously found model
      // the current set of arguments that are not undecided
      struct RaSet* notUndec = raset__init_empty(aaf->number_of_arguments);
      // the previous set of arguments in the extension
      struct RaSet* in_arg = raset__init_empty(aaf->number_of_arguments);
      IpasirSolver inner_solver;
      sat__init(inner_solver, 2*aaf->number_of_arguments);
      add_cfTestClauses(inner_solver,in_vars,out_vars,aaf,grounded);
      // add clauses imposing that arguments IN/OUT in the previously
      // found model are again IN/OUT
      // at least one of the others must become IN/OUT
      int* clause = (int*) malloc(2*aaf->number_of_arguments * sizeof(int));
      int idx = 0;
      for(int i = 0; i < aaf->number_of_arguments; i++){
        if(sat__get(solver,in_vars[i]) > 0 || sat__get(solver,out_vars[i]) > 0){
          raset__add(notUndec,i);
          if(sat__get(solver,in_vars[i]) > 0)
            raset__add(in_arg,i);
          sat__addClause2(inner_solver,in_vars[i],out_vars[i]);
        }else{
          clause[idx++] = in_vars[i];
          clause[idx++] = out_vars[i];
        }
      }
      sat__addClause(inner_solver,clause,idx);
      while(true){
        sat__assume(inner_solver,in_vars[task->arg]);
        sat = sat__solve(inner_solver);
        if(sat == 20){
            // no set with smaller UNDEC set could be found
            // that also contains the argument
            // now check whether there is a set with smaller
            // undec set (that would then not contain the argument)
            sat = sat__solve(inner_solver);
            if(sat == 20){
              // so we found a stage extension with the argument
              if(do_print){
                printf("YES\n");
                raset__print_i23(in_arg,aaf->ids2arguments);
              }
              raset__destroy(notUndec);
              raset__destroy(in_arg);
              sat__free(solver);
              sat__free(inner_solver);
              return true;
            }
            // otherwise add a clause to solver imposing one of the UNDEC
            // arguments to be IN/OUT
            for(int i = 0; i < aaf->number_of_arguments; i++){
              if(!raset__contains(notUndec,i)){
                sat__add(solver,in_vars[i]);
                sat__add(solver,out_vars[i]);
              }
            }
            sat__add(solver,0);
            break;
        }
        // add another clause imposing even less UNDEC
        idx = 0;
        raset__reset(in_arg);
        for(int i = 0; i < aaf->number_of_arguments; i++){
          if(sat__get(inner_solver,in_vars[i]) > 0 || sat__get(inner_solver,out_vars[i]) > 0){
            if(!raset__contains(notUndec,i) && task->arg != i){
              sat__addClause2(inner_solver,in_vars[i],out_vars[i]);
              raset__add(notUndec,i);
            }
            if(sat__get(inner_solver,in_vars[i]) > 0)
              raset__add(in_arg,i);
          }else{
            clause[idx++] = in_vars[i];
            clause[idx++] = out_vars[i];
          }
        }
        sat__addClause(inner_solver,clause,idx);
      }
      raset__destroy(notUndec);
      raset__destroy(in_arg);
      sat__free(inner_solver);
  }
  sat__free(solver);
  // this should not happen
  return false;
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
