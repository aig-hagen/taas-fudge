/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : sat_external.cpp
 Author      : Matthias Thimm
 Version     : 1.0
 Copyright   : GPL3
 Description : Bridge functions to calling a SAT solver externally
 ============================================================================
 */

struct ExternalSolver{
  std::vector<std::vector<int>> clauses;
  std::vector<int> assumptions;
  int num_vars;
  int num_clauses;
  bool last_clause_closed;
  char* solver_path;
  std::vector<bool> model;
};

// inits a new solver with a new set of variables
void sat__init(ExternalSolver & solver, int num_vars, char* solver_path){
  solver.num_vars = num_vars;
  solver.num_clauses = 0;
  solver.last_clause_closed = true;
  solver.clauses = std::vector<std::vector<int>>();
  solver.solver_path = solver_path;
  solver.model = std::vector<bool>(solver.num_vars+1);
}

// adds a literal (clause must be terminated by 0)
void sat__add(ExternalSolver & solver, int var){
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
void sat__assume(ExternalSolver & solver, int lit){
  solver.assumptions.push_back(lit);
}

// adds clauses
void sat__addClause1(ExternalSolver & solver, int var){
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
void sat__addClause2(ExternalSolver & solver, int var1, int var2){
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
void sat__addClause3(ExternalSolver & solver, int var1, int var2, int var3){
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
void sat__addClause(ExternalSolver & solver, int* clause, int num){
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

void sat__addClauseZTVec(ExternalSolver & solver, std::vector<int> clause){
  if(!solver.last_clause_closed){
    // this should not happen
    printf("Previous clause not closed.");
    exit(1);
  }
  solver.clauses.push_back(clause);
  solver.num_clauses++;
}

// adds clauses
void sat__addClauseZT(ExternalSolver & solver, int* clause){
  solver.clauses.push_back(std::vector<int>());
  solver.num_clauses++;
  int i = 0;
  do{
    solver.clauses[solver.num_clauses-1].push_back(clause[i]);
  }while(clause[i++] != 0);
}

// solve the problem
int sat__solve(ExternalSolver & solver){
  redi::pstream proc(solver.solver_path, redi::pstreams::pstdout | redi::pstreams::pstdin | redi::pstreams::pstderr);
  proc << "p cnf " << solver.num_vars << " " << (solver.num_clauses+solver.assumptions.size()) << "\n";
  //std::cout << "p cnf " << solver.num_vars << " " << (solver.num_clauses+solver.assumptions.size()) << "\n";
  for(const std::vector<int> clause: solver.clauses) {
      for(const int lit: clause){
        proc << lit << " ";
        //std::cout << lit << " ";
      }
      proc << "\n";
      //std::cout << "\n";
  }
  for(const int assumption: solver.assumptions){
    proc << assumption << " 0\n";
    //std::cout << assumption << " 0\n";
  }
  //std::cout << "-----------------------------------------------" << std::endl;
  solver.assumptions.clear();
  proc << redi::peof;
  std::string line;
  solver.model.clear();
  while (std::getline(proc,line)) {
    //std::cout << line << std::endl;
    if(line.rfind("c ", 0) == 0)
      continue;
    if(line.rfind("s ", 0) == 0){
      if(line.rfind("UNSATISFIABLE") != std::string::npos){
        return 20;
      }
    }
    if(line.rfind("v ", 0) == 0){
        line.erase(0, 2);
        size_t pos = 0;
        while(line.length() > 0) {
          pos = line.find(" ");
          if(pos == std::string::npos)
            pos = line.length();
          int var = stoi(line.substr(0, pos));
          if(var > 0)
            solver.model[var] = true;
          else if(var < 0)
            solver.model[-var] = false;
          else break;
          line.erase(0, pos + 1);
        }
    }
  }
  return 10;
}

// get value of var (positive=true, negative=false)
int sat__get(ExternalSolver & solver, int var){
  if(solver.model[var])
    return 1;
  return -1;
}

// free memory
void sat__free(ExternalSolver & solver){
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
void printModel(ExternalSolver solver, int* in_vars, struct AAF* aaf){
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
