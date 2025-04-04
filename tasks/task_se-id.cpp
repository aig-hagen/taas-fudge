/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_se-id.cpp
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : solve function for SE-ID
 ============================================================================
 */

// the fudge approach for SE-ID
struct RaSet* compute_ideal(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){
    // initialise variables
    int* in_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
    int* out_vars = (int*) malloc(aaf->number_of_arguments * sizeof(int));
    int idx = 1;
    for(int i = 0; i < aaf->number_of_arguments;i++){
      in_vars[i] = idx++;
      out_vars[i] = idx++;
    }
    // solver_admTest is used for determining admissible set that attack
    //        not already ruled out to be in the preferred super-core
    IpasirSolver solver_admTest;
    sat__init(solver_admTest, idx-1);
    // add admissibility clauses
    add_admTestClauses(solver_admTest,in_vars,out_vars,aaf,grounded);
    // set "psc" will eventually hold the preferred super-core
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
                sat__add(solver_admTest, in_vars[*(int*)node->data]);
            }
        }
        if (emptyclause)
            break;
        sat__add(solver_admTest, 0);
        sat = sat__solve(solver_admTest);
        // computation of psc finished
        if(sat == 20)
            break;
        // look at each argument OUT in the above found admissible set
        // and remove it from psc
        for(int i = 0; i < aaf->number_of_arguments; i++){
            if(sat__get(solver_admTest,in_vars[i]) > 0){
                //removing the children from psc
                for(GSList* node = aaf->children[i]; node != NULL; node = node->next) {
                    if (raset__contains(psc, *(int*)node->data))
                        raset__remove(psc, *(int*)node->data);
                }
            }
        }
        onerunonly = false;
    }
    sat__free(solver_admTest);
    if (onerunonly){
          struct RaSet* ideal = raset__init_empty(aaf->number_of_arguments);
          for(int idx = bitset__next_set_bit(grounded->in,0); idx != -1 ; idx = bitset__next_set_bit(grounded->in, idx+1)){
            if(!grounded->twoValued && bitset__get(grounded->out,idx))
              continue;
            raset__add(ideal,idx);
          }
          return ideal;
    }
    // now compute the maximal admissible set in psc
    // if no argument is in the psc, the ideal extension is also empty
    if(psc->number_of_elements == 0){
      return psc;
    }
    //raset__print(psc,aaf->ids2arguments);
    // this will hold the ideal extension
    struct RaSet* ideal = raset__init_empty(aaf->number_of_arguments);
    // only add those arguments to ideal which are not attacked by another argument
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
        raset__add(ideal,psc->elements_arr[i]);
    }
    //raset__print(ideal,aaf->ids2arguments);
    // now iterate over ideal and remove arguments not defended by ideal;
    // repeat until no more arguments are removed
    bool changed;
    do{
      changed = false;
      for(int i = 0; i < ideal->number_of_elements; i++){
        int arg = ideal->elements_arr[i];
        bool keep_arg = true;
        for(GSList* node = aaf->parents[arg]; node != NULL; node = node->next){
          int attacker = * reinterpret_cast<int*>(node->data);
          bool found_defender = false;
          for(GSList* node2 = aaf->parents[attacker]; node2 != NULL; node2 = node2->next){
            if(raset__contains(ideal,*reinterpret_cast<int*>(node2->data))){
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
          raset__remove(ideal,arg);
          i--;
        }
      }
    }while(changed);
    raset__destroy(psc);
    return ideal;
}

void solve_seid(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded){
  struct RaSet* ideal = compute_ideal(task,aaf,grounded);
  raset__print_i23(ideal,aaf->ids2arguments);
  raset__destroy(ideal);
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
