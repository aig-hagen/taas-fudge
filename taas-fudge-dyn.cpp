/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : taas-fudge-dyn.cpp
 Author      : Matthias Thimm
 Version     : 1.0
 Copyright   : GPL3
 Description : The taas-fudge solver (dynamic version) for abstract argumentation.

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
/* ============================================================================================================== */
/* ============================================================================================================== */
#define PRINT_WITNESS 1
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

#include <set>
#include <map>
//#include <array>
#include <string>
//#include <cstdio>

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
using namespace std;

class FudgeDynSolver {

public:
    FudgeDynSolver() : state(0) {}

    void set_semantics(uint8_t sem) {
        switch (sem) {
            case 1:
                semantics = "CO";
                break;
            case 2:
                semantics = "PR";
                break;
            case 3:
                semantics = "ST";
                break;
            case 4:
                semantics = "SST";
                break;
            case 5:
                semantics = "STG";
                break;
            case 6:
                semantics = "ID";
                break;
            default:
                state = -1;
                return;
        }
        state = 0;
    }

    void add_argument(int32_t argument) {
        if (argument <= 0 || arguments.count(argument)) {
            state = -1;
            return;
        }
        arguments.insert(argument);
        state = 0;
    }

    void del_argument(int32_t argument) {
        if (!arguments.count(argument)) {
            state = -1;
            return;
        }
        arguments.erase(argument);
        state = 0;
    }

    void add_attack(int32_t source, int32_t target) {
        if (!arguments.count(source) || !arguments.count(target)
            || attacks.count(make_pair(source,target))) {
            state = -1;
            return;
        }
        attacks.insert(make_pair(source, target));
        state = 0;
    }

    void del_attack(int32_t source, int32_t target) {
        if (!attacks.count(make_pair(source, target))) {
            state = -1;
            return;
        }
        attacks.erase(make_pair(source, target));
        state = 0;
    }

    void assume(int32_t argument) {
        if (!arguments.count(argument) || assumptions.size()) {
            state = -1;
            return;
        }
        assumptions.insert(argument);
    }

    int32_t solve(bool credulous_mode) {
        if (state < 0) return -1;
        if (semantics.empty()) return -1;
        if (assumptions.empty()) return -1;

//        // create temporary apx file with current af
//        string name = std::getenv("TMPDIR") + string("af.apx");
//        ofstream file;
//        file.open(name);
//        for (int32_t argument : arguments) {
//          file << "arg(" << argument << ").\n";
//        }
//        for (pair<int32_t,int32_t> attack : attacks) {
//            file << "att(" << attack.first << "," << attack.second << ").\n";
//        }
//        file.close();
//
//        // call external solver on the corresponding task
//        string task = credulous_mode ? "DC-" + semantics : "DS-" + semantics;
//        string query = to_string(*assumptions.begin());
//        assumptions.clear();
//        string command = EXTERNAL_AF_SOLVER + string(" -p ") + task
//                         + string(" -f ") + name + string(" -fo apx -a ") + query;
//        FILE * pipe = popen(command.c_str(), "r");
//        if (!pipe) return -1;
//        array<char, 128> buffer;
//        string result;
//        while (fgets(buffer.data(), 128, pipe) != NULL) {
//            result += buffer.data();
//        }
//        pclose(pipe);
//        remove(name.c_str());
//
//        // parse the output
//        result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
//        if (result == "YES") {
//            return 10;
//        } else if (result == "NO") {
//            return 20;
//        }
        return -1;
    }

    int32_t val(int32_t argument) {
        return 0; // redundant: no certificate needed for dynamic track
    }

private:
    int32_t state;
    string semantics;
    set<int32_t> arguments;
    set<pair<int32_t,int32_t>> attacks;
    set<int32_t> assumptions;
};

/* ============================================================================================================== */
/* ============================================================================================================== */
extern "C" {

#include "dyn/ipafair.h"

static FudgeDynSolver* import(void* s) {
  return (FudgeDynSolver*) s;
}

const char* ipafair_signature() {
  return "taas-fudge-dyn v3.2.7 (2022-11-29)\nMatthias Thimm (matthias.thimm@fernuni-hagen.de), Federico Cerutti (federico.cerutti@unibs.it), Mauro Vallati (m.vallati@hud.ac.uk)";
}

void* ipafair_init() {
  return new FudgeDynSolver();
}

void ipafair_release(void* s) {
  delete import(s);
}

void ipafair_set_semantics(void* s, semantics sem){
  import(s)->set_semantics(sem);
}

void ipafair_add_argument(void* s, int32_t a){
  import(s)->add_argument(a);
}

void ipafair_del_argument(void* s, int32_t a){
  import(s)->del_argument(a);
}

void ipafair_add_attack(void* s, int32_t a, int32_t b){
  import(s)->add_attack(a,b);
}

void ipafair_del_attack(void* s, int32_t a, int32_t b){
  import(s)->del_attack(a,b);
}

void ipafair_assume(void* s, int32_t a){
  import(s)->assume(a);
}

int32_t ipafair_solve_cred(void* s){
  return import(s)->solve(true);
}

int32_t ipafair_solve_skept(void* s){
  return import(s)->solve(false);
}

int32_t ipafair_val(void* s, int32_t a) {
  return import(s)->val(a);
}

};
/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
