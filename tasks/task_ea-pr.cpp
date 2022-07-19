/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_ea-pr.cpp
 Author      : Matthias Thimm
 Version     : 1.0
 Copyright   : GPL3
 Description : solve function for EA-PR (enumerate all arguments that
               are skeptically accepted wrt. preferred semantics)
 ============================================================================
 */

// the fudge approach for EA-PR
void solve_eapr(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){
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
    for(int i = 0; i < aaf->number_of_arguments;i++){
      in_attacked_vars[i] = idx++;
      out_attacked_vars[i] = idx++;
    }
    // add admissibility clauses
    // solver_admTest_outer is used for determining admissible set that attack
    //        not already ruled out to be in the preferred super-core
    ExternalSolver solver_admTest_outer;
    sat__init(solver_admTest_outer, 2*aaf->number_of_arguments,taas__task_get_value(task,"-sat"));
    add_admTestClauses(solver_admTest_outer,in_vars,out_vars,aaf,grounded);
    // set "psc" will eventually contain the preferred super-core
    struct RaSet* psc = raset__init_empty(aaf->number_of_arguments);
    for(int i = 0; i < aaf->number_of_arguments; i++){
      // if argument is out in the grounded extension it is not in psc
      // all other arguments go in
      if(!bitset__get(grounded->out,i)){
        raset__add(psc,i);
      }
    }
    bool onerunonly = true;
    int sat;
    // main loop
    while(true){
        // find an admissible set that attacks at least one argument
        // from the current psc (if there is none we are finished)
        bool emptyclause = true;
        for(int i = 0; i < psc->number_of_elements; i++){
            for(GSList* node = aaf->parents[psc->elements_arr[i]]; node != NULL; node = node->next){
                emptyclause = false;
                sat__add(solver_admTest_outer, in_vars[*(int*)node->data]);
            }
        }
        if (emptyclause)
            break;
        sat__add(solver_admTest_outer, 0);
        sat = sat__solve(solver_admTest_outer);
        // computation of psc finished
        if(sat == 20)
            break;
        // look at each argument OUT in the above found admissible set
        // and remove it from psc
        for(int i = 0; i < aaf->number_of_arguments; i++){
            if(sat__get(solver_admTest_outer,in_vars[i]) > 0){
                //removing the children from psc
                for(GSList* node = aaf->children[i]; node != NULL; node = node->next) {
                    if (raset__contains(psc, *(int*)node->data))
                        raset__remove(psc, *(int*)node->data);
                }
            }
        }
        onerunonly = false;
    }
    sat__free(solver_admTest_outer);
    if (onerunonly){
        printf("%s\n", taas__lab_print(grounded, aaf));
        return;
    }
    for(int i = 0; i < aaf->number_of_arguments; i++){
        // if argument is out in the grounded extension it is not in psc
        // all other arguments go in
        if(bitset__get(grounded->in,i)){
            raset__add(psc,i);
        }
    }
    // now compute the maximal admissible set in psc
    // if no argument is in the psc, there are no accepted arguments
    if(psc->number_of_elements == 0){
      raset__print(psc,aaf->ids2arguments);
      raset__destroy(psc);
      return;
    }
    //raset__print(psc,aaf->ids2arguments);
    // this will hold the the set of acceptable arguments
    struct RaSet* acc = raset__init_empty(aaf->number_of_arguments);
    // only add those arguments to acc which are not attacked by another argument
    // in psc
    for(int i = 0; i < psc->number_of_elements; i++){
      bool isAttacked = false;
      for(GSList* node = aaf->parents[psc->elements_arr[i]]; node != NULL; node = node->next){
        if(raset__contains(psc, *reinterpret_cast<int*>(node->data))){
          isAttacked = true;
          break;
        }
      }
      if(!isAttacked)
        raset__add(acc,psc->elements_arr[i]);
    }
    //raset__print(acc,aaf->ids2arguments);
    // now iterate over acc and remove arguments not defended by acc;
    // repeat until no more arguments are removed
    bool changed;
    do{
      changed = false;
      for(int i = 0; i < acc->number_of_elements; i++){
        int arg = acc->elements_arr[i];
        //printf("A: %s\n", aaf->ids2arguments[arg]);
        bool keep_arg = true;
        for(GSList* node = aaf->parents[arg]; node != NULL; node = node->next){
          int attacker = * reinterpret_cast<int*>(node->data);
          //printf("B: %s\n", aaf->ids2arguments[attacker]);
          bool found_defender = false;
          for(GSList* node2 = aaf->parents[attacker]; node2 != NULL; node2 = node2->next){
            //printf("C: %s\n", aaf->ids2arguments[*reinterpret_cast<int*>(node2->data)]);
            if(raset__contains(acc,*reinterpret_cast<int*>(node2->data))){
              found_defender = true;
              break;
            }
          }
          if(!found_defender){
            keep_arg = false;
            break;
          }
        }
        if(!keep_arg){
          changed = true;
          raset__remove(acc,arg);
          //printf("D: ");raset__print(ideal,aaf->ids2arguments);
          i--;
        }
      }
    }while(changed);
    // acc now holds the ideal extension
    //raset__print(acc,aaf->ids2arguments);
    // now compute the strong preferred super core by removing all arguments
    // from psc\acc that are not contained in an admissible set
    // add admissibility clauses
    ExternalSolver solver_admTest2;
    sat__init(solver_admTest2, 2*aaf->number_of_arguments,taas__task_get_value(task,"-sat"));
    add_admTestClauses(solver_admTest2,in_vars,out_vars,aaf,grounded);
    struct RaSet* spsc = raset__init_empty(aaf->number_of_arguments);
    while(true){
      bool emptyclause = true;
      for(int i = 0; i < psc->number_of_elements; i++)
        if(!raset__contains(acc,psc->elements_arr[i]) && !raset__contains(spsc,psc->elements_arr[i])){
          sat__add(solver_admTest2,in_vars[psc->elements_arr[i]]);
          emptyclause = false;
        }
      if(emptyclause)
        break;
      sat__add(solver_admTest2,0);
      sat = sat__solve(solver_admTest2);
      if(sat == 20){
        //no further arguments are in an admissible set
        break;
      }
      for(int i = 0; i < aaf->number_of_arguments; i++){
          if (bitset__get(grounded->in, i) || bitset__get(grounded->out, i))
              continue;
          if(sat__get(solver_admTest2,in_vars[i]) > 0 && raset__contains(psc,i) && !raset__contains(acc,i)){
            raset__add(spsc,i);
          }
        }
    }
    sat__free(solver_admTest2);
    //now spsc contains the elements in the strong preferred super core that are not already in acc
    //raset__print(spsc,aaf->ids2arguments);
    // test all arguments in spsc
    for(int i = 0; i < spsc->number_of_elements; i++){
      int arg = spsc->elements_arr[i];
      // solver_admTest is used for checking whether a single set can be extended
      //       to an admissible set
      // solver_attAdmTest is used for checking whether there is an admissible set
      //       attacking another admissible set
      ExternalSolver solver_admTest;
      sat__init(solver_admTest, 2*aaf->number_of_arguments,taas__task_get_value(task,"-sat"));
      ExternalSolver solver_attAdmTest;
      sat__init(solver_attAdmTest, 4*aaf->number_of_arguments+aaf->number_of_attacks,taas__task_get_value(task,"-sat"));
      // add admissibility clauses
      add_admTestClauses(solver_admTest,in_vars,out_vars,aaf,grounded);
      add_admTestClauses(solver_attAdmTest,in_vars,out_vars,aaf,grounded);
      add_admTestClauses(solver_attAdmTest,in_attacked_vars,out_attacked_vars,aaf,grounded);
      // add constraints for modelling the attack to the other set
      add_attackClauses(solver_attAdmTest,in_vars,in_attacked_vars,4*aaf->number_of_arguments+1,aaf,grounded);
      // all arguments in acc must be always in any considered admissible set
      for(int j = 0; j < acc->number_of_elements; j++){
        sat__addClause1(solver_admTest,in_vars[acc->elements_arr[j]]);
        sat__addClause1(solver_attAdmTest,in_vars[acc->elements_arr[j]]);
        sat__addClause1(solver_attAdmTest,in_attacked_vars[acc->elements_arr[j]]);
      }
      while(true){
          // check whether there is an addmissible set attacking an
          // admissible set containing the argument under consideration
          sat__assume(solver_attAdmTest, in_attacked_vars[arg]);
          sat = sat__solve(solver_attAdmTest);
          // no more argument can be accepted, so we are finished
          if(sat == 20){
              // argument arg must be skeptically accepted
              raset__add(acc,arg);
              sat__free(solver_admTest);
              sat__free(solver_attAdmTest);
              break;
          }
          sat__assume(solver_admTest,in_vars[arg]);
          for(int j = 0; j < aaf->number_of_arguments; j++){
            if(sat__get(solver_attAdmTest,in_vars[j]) > 0){
              sat__assume(solver_admTest,in_vars[j]);
            }
          }
          sat = sat__solve(solver_admTest);
          if(sat == 20){
            // argument arg is not skeptically accepted
            sat__free(solver_admTest);
            sat__free(solver_attAdmTest);
            break;
          }
          for(int j = 0; j < aaf->number_of_arguments; j++){
            if(sat__get(solver_admTest,in_vars[j]) < 0){
              sat__add(solver_attAdmTest,in_vars[j]);
            }
          }
          sat__add(solver_attAdmTest,0);
      }
    }
    raset__print(acc,aaf->ids2arguments);
    raset__destroy(acc);
    raset__destroy(psc);
    return;
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
