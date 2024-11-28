# taas-fudge v3.3.1 (2024-11-28)
Matthias Thimm (matthias.thimm@fernuni-hagen.de)
Federico Cerutti (federico.cerutti@unibs.it)
Mauro Vallati (m.vallati@hud.ac.uk)

taas-fudge is written in C++ and in order to compile it you
need glib 2.0 header files (https://developer.gnome.org/glib/).

Then compile taas-fudge via
```
    make
```
taas-fudge uses a probo-compliant command line interface, see also
http://argumentationcompetition.org/2021/SolverRequirements.pdf. In order to
run taas-fudge, a SAT solver using the IPASIR interface is required, which
has to be configured in the make file. This folder also contains the source code to
CaDiCaL v2.1.0, which is also preconfigured in the make file (but must be
compiled on the user system first).

taas-fudge implements the problems [SE-GR,DC-GR,DS-GR,SE-CO,DC-CO,DS-CO,SE-PR,DC-PR,
DS-PR,SE-ST,DC-ST,DS-ST,SE-ID,DC-ID,DS-ID,DC-SST,DS-SST,SE-SST,DC-STG,DS-STG,SE-STG]
and supports the ICCMA23 format for abstract argumentation frameworks. In particular,
to decide skeptical acceptance of an argument wrt. preferred semantics of an AAF use
```
  taas-fudge -p DS-PR -f <file in ICCMA23 format> -a <argument>
```
