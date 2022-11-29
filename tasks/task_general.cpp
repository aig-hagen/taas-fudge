/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : task_general.cpp
 Author      : Matthias Thimm
 Version     : 1.0
 Copyright   : GPL3
 Description : general encodings for various problems
 ============================================================================
 */

// adds clauses such that a model represents an admissible set
// returns "true" iff all arguments are either in the grounded extension
// or attacked by it
bool add_admTestClauses(ExternalSolver & solver, int* in_vars, int* out_vars, struct AAF* aaf, struct Labeling* grounded){
   bool all_grounded = true;
   for(int i = 0; i < aaf->number_of_arguments; i++){
       // argument i cannot both be in and out
       sat__addClause2(solver,-in_vars[i],-out_vars[i]);
       // add knowledge from grounded extension
       if(bitset__get(grounded->in,i)){
          sat__addClause1(solver,in_vars[i]);
          continue;
       }else if(bitset__get(grounded->out,i)){
          sat__addClause1(solver,out_vars[i]);
          continue;
       }
       all_grounded = false;
       // if argument is IN then all attackers must be OUT
       // if argument is OUT then some attacker must be IN
       int* out_clause = (int*) malloc((2+aaf->number_of_arguments) * sizeof(int));
       int idx = 1;
       out_clause[0] = -out_vars[i];
       for(GSList* node = aaf->parents[i]; node != NULL; node = node->next){
           sat__addClause2(solver,-in_vars[i],out_vars[(*(int*)node->data)]);
           out_clause[idx++] = in_vars[*(int*)node->data];
       }
       out_clause[idx] = 0;
       sat__addClauseZT(solver,out_clause);
       free(out_clause);
   }
   return all_grounded;
}

// adds clauses such that a model represents a conflict-free set
// returns "true" iff all arguments are either in the grounded extension
// or attacked by it
bool add_cfTestClauses(ExternalSolver & solver, int* in_vars, int* out_vars, struct AAF* aaf, struct Labeling* grounded){
   bool all_grounded = true;
   for(int i = 0; i < aaf->number_of_arguments; i++){
       // argument i cannot both be in and out
       sat__addClause2(solver,-in_vars[i],-out_vars[i]);
       // check grounded extension
       if(!bitset__get(grounded->in,i) && !bitset__get(grounded->out,i))
            all_grounded = false;
       // add conflict-free clauses
       int* out_clause = (int*) malloc((2+aaf->number_of_arguments) * sizeof(int));
       int idx = 1;
       out_clause[0] = -out_vars[i];
       for(GSList* node = aaf->parents[i]; node != NULL; node = node->next){
           sat__addClause2(solver,-in_vars[(*(int*)node->data)],out_vars[i]);
           out_clause[idx++] = in_vars[*(int*)node->data];
       }
       out_clause[idx] = 0;
       sat__addClauseZT(solver,out_clause);
       free(out_clause);
   }
   return all_grounded;
}

// adds clauses such that a model represents two sets where one attacks the other
void add_attackClauses(ExternalSolver & solver, int* in_vars, int* in_attacked_vars, int attack_idx_offset, struct AAF* aaf, struct Labeling* grounded){
  int* clause_oneattack = (int*) malloc((aaf->number_of_attacks) * sizeof(int));//one attack must be present
    int idx_attack = 0;
    int idx = attack_idx_offset;
    for(int i = 0; i < aaf->number_of_arguments; i++){
      for(GSList* node = aaf->parents[i]; node != NULL; node = node->next){
        clause_oneattack[idx_attack++] = idx;
        // the following three formulas model "attack_var <=> (in_attacker && in_attacked)"
        sat__addClause2(solver,-idx,in_vars[*(int*)node->data]);
        sat__addClause2(solver,-idx,in_attacked_vars[i]);
        sat__addClause3(solver,idx,-in_attacked_vars[i],-in_vars[*(int*)node->data]);
        idx++;
      }
    }
    sat__addClause(solver,clause_oneattack,aaf->number_of_attacks);
}

// adds clauses such that a model represents a stable extension
void add_stbTestClauses(ExternalSolver & solver, int* in_vars, struct AAF* aaf, struct Labeling* grounded){
   // constraints for stability
   int* clause = (int*) malloc((1+aaf->number_of_arguments) * sizeof(int));
   for(int i = 0; i < aaf->number_of_arguments; i++){
      // add knowledge from grounded extension
      if(bitset__get(grounded->in,i))
       sat__addClause1(solver,in_vars[i]);
      else if(bitset__get(grounded->out,i))
       sat__addClause1(solver,-in_vars[i]);

      int idx = 1;
      clause[0] = in_vars[i];
      char emptyclause = TRUE;
      for(GSList* node = aaf->parents[i]; node != NULL; node = node->next){
        sat__addClause2(solver,-in_vars[i],-in_vars[(*(int*)node->data)]);
        clause[idx++] = in_vars[*(int*)node->data];
        emptyclause = FALSE;
     }
     if (!emptyclause){
        sat__addClause(solver,clause,idx);
      }
   }
   free(clause);
}

// adds clauses such that a model represents a complete extension
void add_comTestClauses(ExternalSolver & solver, int* in_vars, int* out_vars, struct AAF* aaf, struct Labeling* grounded){
  // constraints for conflict-freeness, admissibility and completeness
  int* clause = (int*) malloc((1+aaf->number_of_arguments) * sizeof(int));
  int* clause2 = (int*) malloc((1+aaf->number_of_arguments) * sizeof(int));
  int idx, idx2;
  for(int i = 0; i < aaf->number_of_arguments; i++){
      //argments cannot be in and out
      sat__addClause2(solver, -in_vars[i], -out_vars[i]);
      // add knowledge from grounded extension
      if(bitset__get(grounded->in,i)){
         sat__addClause1(solver,in_vars[i]);
         continue;
      }else if(bitset__get(grounded->out,i)){
         sat__addClause1(solver,out_vars[i]);
         continue;
      }
      idx = 1;
      idx2 = 1;
      clause[0] = -out_vars[i];
      clause2[0] = in_vars[i];
      char emptyclause = TRUE;
      // missing: if all parents out then argument in
      for(GSList* node = aaf->parents[i]; node != NULL; node = node->next){
          sat__addClause2(solver,-in_vars[i],out_vars[(*(int*)node->data)]);
          sat__addClause2(solver,-in_vars[(*(int*)node->data)],out_vars[i]);
          clause[idx++] = in_vars[(*(int*)node->data)];
          clause2[idx2++] = -out_vars[(*(int*)node->data)];
      }
      sat__addClause(solver,clause,idx);
      sat__addClause(solver,clause2,idx2);
  }
  free(clause);
  free(clause2);
}

 /** prints the model of the given Glucose solver */
/*
 void printModel(Glucose::Solver* solver, int* in_vars, struct AAF* aaf){
 	  printf("{");
    bool isFirst = true;
    for(int i = 0; i < aaf->number_of_arguments; i++){
      if(solver->model[in_vars[i]] != l_True)
        continue;
      if(isFirst)
        isFirst = false;
      else printf(",");
      printf("%s", aaf->ids2arguments[i]);
    }
    printf("}\n");
 }
*/
 /* ============================================================================================================== */
 /* == END FILE ================================================================================================== */
 /* ============================================================================================================== */
