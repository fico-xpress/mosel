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
> A recent version of the Mosel C libraries needs to be installed. 

This repository contains a copy of the minisat solver from [MiniSat Github repository](https://github.com/niklasso/minisat). Alternatively, on Linux or OSX you could use the [Glucose SAT solver](https://www.labri.fr/perso/lsimon/glucose), following the same build instructions as with MiniSat. 

Run the following command from the current directory:

`model make.mos`

This will compile the module to the directory `build/dso`. To select a different destination directory add the option `BUILDDIR=mypath` where `mypath` is the base directory to use.

The command `mosel make TODO=clean` will remove all generated files.

## Testing

Run the following command from the current directory:

`model make.mos TODO=test`

This will compile the module to the directory `build/dso`. To select a different destination directory add the option `BUILDDIR=mypath` where `mypath` is the base directory to use.

The command `mosel make TODO=clean` will remove all generated files.

## Legal

See source code files for copyright notices.

## License

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](../../LICENSE) for the full license text.
