/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : ipasir_solver.cpp
 Author      : Matthias Thimm
 Version     : 1.0
 Copyright   : GPL3
 Description : Bridge functions to calling an IPASIR SAT solver externally
 ============================================================================
 */

 #include <iostream>
 extern "C" {
     #include "../ipasir.h"  // Include the IPASIR header
 }


struct IpasirSolver{
    void* theSolver = ipasir_init();
};

// inits a new solver with a new set of variables
void sat__init(IpasirSolver & solver, int num_vars){
   solver.theSolver = ipasir_init();
}

// adds a literal (clause must be terminated by 0)
void sat__add(IpasirSolver & solver, int var){
    ipasir_add(solver.theSolver, var);
}

// assume literal
void sat__assume(IpasirSolver & solver, int lit){
    ipasir_assume(solver.theSolver, lit);
}

// adds clauses
void sat__addClause1(IpasirSolver & solver, int var){
    ipasir_add(solver.theSolver, var);
    ipasir_add(solver.theSolver, 0);
}

// adds clauses
void sat__addClause2(IpasirSolver & solver, int var1, int var2){
    ipasir_add(solver.theSolver, var1);
    ipasir_add(solver.theSolver, var2);
    ipasir_add(solver.theSolver, 0);
}

// adds clauses
void sat__addClause3(IpasirSolver & solver, int var1, int var2, int var3){
    ipasir_add(solver.theSolver, var1);
    ipasir_add(solver.theSolver, var2);
    ipasir_add(solver.theSolver, var3);
    ipasir_add(solver.theSolver, 0);
}

// adds clauses
void sat__addClause(IpasirSolver & solver, int* clause, int num){
    for(int i = 0; i < num; i++)
        ipasir_add(solver.theSolver, clause[i]);
    ipasir_add(solver.theSolver, 0);
}

void sat__addClauseZTVec(IpasirSolver & solver, std::vector<int> clause){
    for(int i = 0; i < clause.size(); i++)
        ipasir_add(solver.theSolver, clause[i]);
}

// adds clauses
void sat__addClauseZT(IpasirSolver & solver, int* clause){
  int i = 0;
  do{
      ipasir_add(solver.theSolver,clause[i]);
  }while(clause[i++] != 0);
}

// solve the problem
int sat__solve(IpasirSolver & solver){
  return ipasir_solve(solver.theSolver);
}

// get value of var (positive=true, negative=false)
int sat__get(IpasirSolver & solver, int var){
  return ipasir_val(solver.theSolver, var);
}

// free memory
void sat__free(IpasirSolver & solver){
  free(solver.theSolver);
}

/** prints the clause */
void printClause(int* clause, int num, int* in_vars, struct AAF* aaf){
  printf("<");
  char isFirst = TRUE;
  for(int i = 0; i < num; i++){
    if(isFirst)
      isFirst = FALSE;
    else printf(",");
    for(int j = 0; j < aaf->number_of_arguments; j++){
      if(in_vars[j] == clause[i])
        printf("%s", aaf->ids2arguments[j]);
      else if(in_vars[j] == -clause[i])
        printf("-%s", aaf->ids2arguments[j]);
    }
  }
  printf(">\n");
}

 /** prints the model of the given solver */
void printModel(IpasirSolver solver, int* in_vars, struct AAF* aaf){
	  printf("{");
  char isFirst = TRUE;
  for(int i = 0; i < aaf->number_of_arguments; i++){
    if(!sat__get(solver,in_vars[i]))
      continue;
    if(isFirst)
      isFirst = FALSE;
    else printf(",");
    printf("%s", aaf->ids2arguments[i]);
  }
  printf("}\n");
}

 /* ============================================================================================================== */
 /* == END FILE ================================================================================================== */
 /* ============================================================================================================== */
