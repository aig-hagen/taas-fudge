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
  std::vector<std::vector<int>> clauses;
  std::vector<int> assumptions;
  int num_vars;
  int num_clauses;
  bool last_clause_closed;
  std::vector<bool> model;
};

// inits a new solver with a new set of variables
void sat__init(IpasirSolver & solver, int num_vars){
  solver.num_vars = num_vars;
  solver.num_clauses = 0;
  solver.last_clause_closed = true;
  solver.clauses = std::vector<std::vector<int>>();
  solver.model = std::vector<bool>(solver.num_vars+1);
}

// adds a literal (clause must be terminated by 0)
void sat__add(IpasirSolver & solver, int var){
  if(solver.last_clause_closed){
    solver.clauses.push_back(std::vector<int>());
    solver.last_clause_closed = false;
    solver.num_clauses++;
  }
  solver.clauses[solver.num_clauses-1].push_back(var);
  if(var == 0)
    solver.last_clause_closed = true;
}

// assume literal
void sat__assume(IpasirSolver & solver, int lit){
  solver.assumptions.push_back(lit);
}

// adds clauses
void sat__addClause1(IpasirSolver & solver, int var){
  if(!solver.last_clause_closed){
    // this should not happen
    printf("Previous clause not closed.");
    exit(1);
  }
  solver.clauses.push_back(std::vector<int>());
  solver.num_clauses++;
  solver.clauses[solver.num_clauses-1].push_back(var);
  solver.clauses[solver.num_clauses-1].push_back(0);
}

// adds clauses
void sat__addClause2(IpasirSolver & solver, int var1, int var2){
  if(!solver.last_clause_closed){
    // this should not happen
    printf("Previous clause not closed.");
    exit(1);
  }
  solver.clauses.push_back(std::vector<int>());
  solver.num_clauses++;
  solver.clauses[solver.num_clauses-1].push_back(var1);
  solver.clauses[solver.num_clauses-1].push_back(var2);
  solver.clauses[solver.num_clauses-1].push_back(0);
}

// adds clauses
void sat__addClause3(IpasirSolver & solver, int var1, int var2, int var3){
  if(!solver.last_clause_closed){
    // this should not happen
    printf("Previous clause not closed.");
    exit(1);
  }
  solver.clauses.push_back(std::vector<int>());
  solver.num_clauses++;
  solver.clauses[solver.num_clauses-1].push_back(var1);
  solver.clauses[solver.num_clauses-1].push_back(var2);
  solver.clauses[solver.num_clauses-1].push_back(var3);
  solver.clauses[solver.num_clauses-1].push_back(0);
}

// adds clauses
void sat__addClause(IpasirSolver & solver, int* clause, int num){
  if(!solver.last_clause_closed){
    // this should not happen
    printf("Previous clause not closed.");
    exit(1);
  }
  solver.clauses.push_back(std::vector<int>());
  solver.num_clauses++;
  for(int i = 0; i < num; i++)
      solver.clauses[solver.num_clauses-1].push_back(clause[i]);
  solver.clauses[solver.num_clauses-1].push_back(0);
}

void sat__addClauseZTVec(IpasirSolver & solver, std::vector<int> clause){
  if(!solver.last_clause_closed){
    // this should not happen
    printf("Previous clause not closed.");
    exit(1);
  }
  solver.clauses.push_back(clause);
  solver.num_clauses++;
}

// adds clauses
void sat__addClauseZT(IpasirSolver & solver, int* clause){
  solver.clauses.push_back(std::vector<int>());
  solver.num_clauses++;
  int i = 0;
  do{
    solver.clauses[solver.num_clauses-1].push_back(clause[i]);
  }while(clause[i++] != 0);
}

// solve the problem
int sat__solve(IpasirSolver & solver){
  void* theSolver = ipasir_init();
  for(const std::vector<int> clause: solver.clauses) {
      for(const int lit: clause){
          ipasir_add(theSolver, lit);
      }
  }
  for(const int assumption: solver.assumptions){
    ipasir_assume(theSolver, assumption);
  }
  solver.assumptions.clear();
  solver.model.clear();

  int result = ipasir_solve(theSolver);

  // parse model in case of satisfiability
  if(result == 10){
    for(int var = 1; var <= solver.num_vars; var++){
         if(ipasir_val(theSolver, var) > 0)
            solver.model[var] = true;
         else
            solver.model[var] = false;
    }
  }
  return result;
}

// get value of var (positive=true, negative=false)
int sat__get(IpasirSolver & solver, int var){
  if(solver.model[var])
    return 1;
  return -1;
}

// free memory
void sat__free(IpasirSolver & solver){
  solver.clauses.clear();
  solver.assumptions.clear();
  solver.model.clear();
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
    if(!solver.model[in_vars[i]])
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
