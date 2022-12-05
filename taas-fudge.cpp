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
// printing witnesses supported for:
// - DC-CO, DC-PR (returns complete set containing query)
// - DC-ST (returns stable extension containing query)
// - DS-ST (returns stable extension not containing query)
// - DS-PR (returns some preferred extension that does not contain the query)
// - DS-ID, DC-ID (returns the ideal extension as witness in any case)
#define PRINT_WITNESS 1
/* ============================================================================================================== */
/* ============================================================================================================== */
#include <stdio.h>
#include <ctype.h>
#include <cmath>
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
#include "tasks/task_se-pr.cpp"
#include "tasks/task_ds-pr.cpp"
#include "tasks/task_se-id.cpp"
#include "tasks/task_ea-pr.cpp"
#include "tasks/task_dc-co.cpp"
#include "tasks/task_dc-sst.cpp"
#include "tasks/task_ds-sst.cpp"
#include "tasks/task_dc-stg.cpp"
#include "tasks/task_ds-stg.cpp"
#include "tasks/task_se-st.cpp"
#include "tasks/task_dc-st.cpp"
#include "tasks/task_ds-st.cpp"
#include "tasks/task_ds-id.cpp"
#include "tasks/task_ce-st.cpp"
#include "tasks/task_ce-co.cpp"
#include "tasks/task_ce-pr.cpp"
#include "tasks/task_se-sst.cpp"
#include "tasks/task_se-stg.cpp"

/* ============================================================================================================== */
/* ============================================================================================================== */
void solve_switch(struct TaskSpecification *task, struct AAF* aaf, struct Labeling* grounded) {
  if(taas__task_get_value(task,"-sat") == NULL){
    printf("SAT solver must be specified via -sat <path>.\n");
    exit(0);
  }
  // DS-PR
  if(strcmp(task->track,"DS-PR") == 0){
    solve_dspr(task, aaf, grounded);
    return;
  }
  // SE-ID
  if(strcmp(task->track,"SE-ID") == 0)
    return solve_seid(task, aaf, grounded);
  // EA-PR
  if(strcmp(task->track,"EA-PR") == 0)
    return solve_eapr(task, aaf, grounded);
  // DC-CO and DC-PR
  if(strcmp(task->track,"DC-CO") == 0 || strcmp(task->track,"DC-PR") == 0){
    solve_dcco(task, aaf, grounded);
    return;
  }
  // SE-ST
  if(strcmp(task->track,"SE-ST") == 0)
    return solve_sest(task, aaf, grounded);
  // DC-ST
  if(strcmp(task->track,"DC-ST") == 0){
    solve_dcst(task, aaf, grounded);
    return;
  }
  // DS-ST
  if(strcmp(task->track,"DS-ST") == 0){
    solve_dsst(task, aaf, grounded);
    return;
  }
  // DS-ID and DC-ID
  if(strcmp(task->track,"DS-ID") == 0 || strcmp(task->track,"DC-ID") == 0)
    if(PRINT_WITNESS){
      struct RaSet* ideal = compute_ideal(task,aaf,grounded);
      if(raset__contains(ideal,task->arg))
        printf("YES\n");
      else printf("NO\n");
      raset__print_i23(ideal,aaf->ids2arguments);
      raset__destroy(ideal);
    }else{
      solve_dsid(task, aaf, grounded);
      return;
    }
  // SE-PR
  if(strcmp(task->track,"SE-PR") == 0)
    return solve_sepr(task, aaf, grounded);
  // SE-SST
  if(strcmp(task->track,"SE-SST") == 0)
    return solve_sesst(task, aaf, grounded);
  // SE-STG
  if(strcmp(task->track,"SE-STG") == 0)
    return solve_sestg(task, aaf, grounded);
  // DC-SST
  if(strcmp(task->track,"DC-SST") == 0){
    solve_dcsst(task, aaf, grounded);
    return;
  }
  // DS-SST
  if(strcmp(task->track,"DS-SST") == 0){
    solve_dssst(task, aaf, grounded);
    return;
  }
  // DC-STG
  if(strcmp(task->track,"DC-STG") == 0){
    solve_dcstg(task, aaf, grounded);
    return;
  }
  // DS-STG
  if(strcmp(task->track,"DS-STG") == 0){
    solve_dsstg(task, aaf, grounded);
    return;
  }
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
			"taas-fudge v3.2.7 (2022-11-29)\nMatthias Thimm (matthias.thimm@fernuni-hagen.de), Federico Cerutti (federico.cerutti@unibs.it), Mauro Vallati (m.vallati@hud.ac.uk)",
			"[i23,tgf]",
			"[SE-GR,DC-GR,DS-GR,SE-CO,DC-CO,DS-CO,SE-PR,DC-PR,DS-PR,SE-ST,DC-ST,DS-ST,SE-ID,DC-ID,DS-ID,DC-SST,DS-SST,SE-SST,DC-STG,DS-STG,SE-STG]"
		);
  return taas__solve(argc,argv,info,solve_switch);
}

/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
