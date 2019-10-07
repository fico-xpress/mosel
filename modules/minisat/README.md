# minisat

## Description

The prototype module *minisat* uses the Mosel NI for implementing a basic interface
to the [MiniSat solver](http://minisat.se) for solving constraint satisfaction (SAT) problems. Features demonstrated by this module include:

* Modeling functionality:
  * definition of subroutines to start a solver run and retrieve solution values
  * new type 'logical' to represent logic clauses that can be composed as set expressions
* NI functionality:
  * implementation of a reset service
  * implementation of type access routines (including 'tostring' and a negation operator) 
    for the type 'logical'
  * initialization of the module and definition of the required interface structures

The example implementation of a SAT solver interface provided by *[minisat.cc](minisat.cc)* has purposely been restricted to a selection of basic features; it can easily be extended to expose other functionality of the solver within the Mosel language or to provide additional modeling capabilities.

The implementation of the Mosel module has been developed and tested with version 2.2.0 of the MiniSat library. 
Note that on Linux or OSX it is also possible to compile the module with version 4.1 of the Glucose solver that is based on version 2.2 of MiniSat.    

## Documentation

See the [Mosel NI User Guide](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/mosel_niug/dhtml/index.html) and the [Mosel NI Reference](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/mosel_NI/dhtml/index.html) for examples and documentation of the Mosel NI functionality. 


## Building instructions

> Compiling the *minisat* DSO requires a C++ compiler.
> Besides a recent (4.8 or later) version of the Mosel C libraries this module also requires the MiniSat C++ sources to be present. For the compilation of the MiniSat solver the zlib library must be installed. 

Download the MiniSat solver sources from the [MiniSat Github repository](https://github.com/niklasso/minisat). Or alternatively, on Linux or OSX you could use the [Glucose SAT solver](https://www.labri.fr/perso/lsimon/glucose), following the same build instructions as with MiniSat. 

For compiling MiniSat under Windows you also need to download and build the [zlib library](https://www.zlib.net) if it is not yet present on your computer.

Building this module along with the *minisat* solver itself requires additional dependencies, please use one of these commands depending on your operating system (where `ZLIB` indicates the location of the zlib library (files `zlib.lib` and `zlib.h`) and `MINISAT` is the directory that contains the sources of the SAT solver):

**Windows:**

`cl -Ox -LD -MD -Feminisat.dso -I%XPRESSDIR%/include -I%ZLIB% -I%MINISAT% minisat.cc %MINISAT%/core/Solver.cc`

**Linux and OSX:**

`g++ -O3 -fPIC -fpermissive -shared -ominisat.dso -I${XPRESSDIR}/include -I${MINISAT} minisat.cc ${MINISAT}/core/Solver.cc`


## Testing

A recent (4.8 or later) version of Xpress Mosel needs to be installed.
Make sure the file `minisat.dso` is on the DSO search path (either in the *dso* subdirectory of your Xpress Mosel installation or in a location pointed to by the enviroment variable `MOSEL_DSO`).
Run the provided test model *[minisat_test.mos](minisat_test.mos)*:
* from the command line: `mosel minisat_test`
* with Xpress Workbench: open the file `minisat_test.mos` in the Workbench workspace and select `Run minisat_test.mos` from the workspace *Run* menu


## Legal

See source code files for copyright notices.

## License

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](../../LICENSE) for the full license text.
