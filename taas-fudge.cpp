/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : taas-fudge.cpp
 Author      : Matthias Thimm
 Version     : 1.0
 Copyright   : GPL3
 Description : The taas-fudge solver for abstract argumentation.

============================================================================
*/
#define COMPUTATION_FINISHED 0
#define COMPUTATION_ABORTED__ANSWER_YES 1
#define COMPUTATION_ABORTED__ANSWER_NO  2
#define COMPUTATION_ABORTED__ANSWER_EMPTYSET  3
#define COMPUTATION_ABORTED__ANSWER_EMPTYEMPTYSET  4
#define COMPUTATION_FINISHED__EMPTY_GROUNDED  5
#define COMPUTATION_FINISHED__NONEMPTY_GROUNDED  6
#define COMPUTATION_FINISHED__ANSWER_NO  7

#define TRUE 1
#define FALSE 0
/* ============================================================================================================== */
/* ============================================================================================================== */
#include <stdio.h>
#include <ctype.h>
#include <glib.h>
#include <assert.h>
#include <stdbool.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "lib/pstreams-1.0.3/pstream.h"

#include "util/bitset.c"
#include "util/raset.c"
#include "util/miscutil.c"

#include "taas/taas_aaf.c"
#include "taas/taas_inout.c"
#include "taas/taas_labeling.c"
#include "taas/taas_basics.c"

#include "sat/sat_external.cpp"

#include "tasks/task_general.cpp"
#include "tasks/task_ds-pr.cpp"
#include "tasks/task_se-id.cpp"
#include "tasks/task_ea-pr.cpp"
#include "tasks/task_dc-co.cpp"
#include "tasks/task_se-st.cpp"
#include "tasks/task_dc-st.cpp"
#include "tasks/task_ds-st.cpp"
#include "tasks/task_ds-id.cpp"
#include "tasks/task_se-pr.cpp"
#include "tasks/task_ce-st.cpp"
#include "tasks/task_ce-co.cpp"
#include "tasks/task_ce-pr.cpp"

/* ============================================================================================================== */
/* ============================================================================================================== */
void solve_switch(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded) {
  if(taas__task_get_value(task,"-sat") == NULL){
    printf("SAT solver must be specified via -sat <path>.\n");
    exit(0);
  }
  /*ExternalSolver solver;
  sat__init(solver, 3,taas__task_get_value(task,"-sat"));
  sat__addClause3(solver, 1, 2, 3);
  sat__addClause3(solver, -1, 2, -3);
  sat__addClause3(solver, -1, -2, 3);
  sat__assume(solver,1);
  sat__assume(solver,2);
  //sat__addClause2(solver, 1, -2);
  sat__solve(solver);
  //TODO
  std::cout << solver.model[1] << std::endl;
  std::cout << solver.model[2] << std::endl;
  std::cout << solver.model[3] << std::endl;
  exit(0);*/
  // DS-PR
  if(strcmp(task->track,"DS-PR") == 0)
    return solve_dspr(task, aaf, grounded);
  // SE-ID
  if(strcmp(task->track,"SE-ID") == 0)
    return solve_seid(task, aaf, grounded);
  // EA-PR
  if(strcmp(task->track,"EA-PR") == 0)
    return solve_eapr(task, aaf, grounded);
  // DC-CO and DC-PR
  if(strcmp(task->track,"DC-CO") == 0 || strcmp(task->track,"DC-PR") == 0)
    return solve_dcco(task, aaf, grounded);
  // SE-ST
  if(strcmp(task->track,"SE-ST") == 0)
    return solve_sest(task, aaf, grounded);
  // DC-ST
  if(strcmp(task->track,"DC-ST") == 0)
    return solve_dcst(task, aaf, grounded);
  // DS-ST
  if(strcmp(task->track,"DS-ST") == 0)
    return solve_dsst(task, aaf, grounded);
  // DS-ID
  if(strcmp(task->track,"DS-ID") == 0)
    return solve_dsid(task, aaf, grounded);
  // SE-PR
  if(strcmp(task->track,"SE-PR") == 0)
    return solve_sepr(task, aaf, grounded);
  // CE-ST
  if(strcmp(task->track,"CE-ST") == 0)
    return solve_cest(task, aaf, grounded);
  // CE-CO
  if(strcmp(task->track,"CE-CO") == 0)
    return solve_ceco(task, aaf, grounded);
  // CE-PR
  if(strcmp(task->track,"CE-PR") == 0)
    return solve_cepr(task, aaf, grounded);
}

/* ============================================================================================================== */
int main(int argc, char *argv[]){
  // General solver information
	struct SolverInformation *info = taas__solverinformation(
			"taas-fudge v3.1.1 (2022-07-19)\nMatthias Thimm (matthias.thimm@fernuni-hagen.de), Federico Cerutti (federico.cerutti@unibs.it), Mauro Vallati (m.vallati@hud.ac.uk)",
			"[tgf]",
			"[SE-GR,EE-GR,DC-GR,DS-GR,SE-CO,DS-CO,DS-PR,SE-ID,EA-PR,DC-CO,SE-ST,DS-ST,DS-ID,SE-PR,DC-PR,DC-ST,CE-ST,CE-CO,CE-PR]"
		);
  return taas__solve(argc,argv,info,solve_switch);
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
