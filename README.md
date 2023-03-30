# taas-fudge v3.2.8 (2023-03-30)
Matthias Thimm (matthias.thimm@fernuni-hagen.de)
Federico Cerutti (federico.cerutti@unibs.it)
Mauro Vallati (m.vallati@hud.ac.uk)


taas-fudge is written in C++ and in order to compile it you
need glib 2.0 header files (https://developer.gnome.org/glib/).

Then compile taas-fudge via
```
    ./build-taas-fudge.sh
```
taas-fudge uses a probo-compliant command line interface, see also
http://argumentationcompetition.org/2021/SolverRequirements.pdf. In order to
run taas-fudge, a SAT solver is required. This folder contains the source code to
CaDiCaL v1.3.1, which is automatically build within the above skript.

taas-fudge implements the problems [SE-GR,EE-GR,DC-GR,DS-GR,SE-CO,DS-CO,DS-PR,
SE-ID,EA-PR,DC-CO,SE-ST,DS-ID,SE-PR,DC-PR,CE-ST,CE-CO,CE-PR] and supports the
the ICCMA23 format for abstract argumentation frameworks. In particular, to decide
skeptical acceptance of an argument wrt. preferred semantics of an AAF in
TGF format use
```
  taas-fudge -p DS-PR -fo i23 -f <file in ICCMA23 format> -a <argument>
```
