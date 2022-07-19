# taas-fudge v3.1 (2022-07-11)
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
http://argumentationcompetition.org/2021/SolverRequirements.pdf.

taas-fudge implements the problems [SE-GR,EE-GR,DC-GR,DS-GR,SE-CO,DS-CO,DS-PR,
SE-ID,EA-PR,DC-CO,SE-ST,DS-ID,SE-PR,DC-PR,CE-ST,CE-CO,CE-PR] and supports the
TGF format for abstract argumentation frameworks. In particular, to decide
skeptical acceptance of an argument wrt. preferred semantics of an AAF in
TGF format use
```
  taas-fudge -p DS-PR -fo TGF -f <file in TGF format> -a <argument>
```
