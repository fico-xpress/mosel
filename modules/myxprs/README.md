# myxprs

## Description

The example module *myxprs* uses the matrix handling functionality provided by the Mosel NI for implementing a basic interface
to [Xpress Optimizer](https://www.fico.com/en/products/fico-xpress-solver) for solving linear and mixed integer (LP and MIP) optimization problems. Features demonstrated by this module include:

* Modeling functionality:
  * definition of subroutines to start an optimization run and retrieve solution values
  * access to solver parameters
  * support for handling multiple problems
  * definition of a solver callback (integer solution callback)
  * generation of names for matrix entities and implementation of a matrix output routine
* NI functionality:
  * implementation of a reset and an unload service
  * initialization of the module and definition of the required interface structures
  * handling of user interrupt (Ctrl-c) to stop the solver run
  * redirection of solver output to Mosel streams

The example implementation of a solver interface provided by *[myxprs.c](myxprs.c)* has purposely been restricted to a selection of basic features in order to focus on the use of the NI matrix handling interface; it can easily be extended to expose other functionality of the Xpress Optimizer C library within the Mosel language. 

## Documentation

See chapter 'Implementing an LP/MIP solver interface' of the [Mosel NI User Guide](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/mosel_niug/dhtml/index.html) for a description of this example module and a Mosel model example showing its use. The matrix handling functionality of the Mosel NI is documented in the chapter 'Matrix related functions' of the [Mosel NI Reference](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/mosel_NI/dhtml/index.html). 

**Further reading:** The whitepaper [Mosel Solver Interfaces](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/mosel_solvers/dhtml/index.html) discusses alternative options for interfacing Mosel with optimization solvers.


## Building instructions

> Compiling a DSO requires a C compiler.
> Besides a recent (6.0 or later) version of the Mosel C libraries this module also requires the Xpress Optimizer C libraries to be installed. 

Building this module requires additional dependencies, please use one of these commands depending on your operating system:

**Windows:**

`cl /LD /MD /Femyxprs.dso /I%XPRESSDIR%\include myxprs.c %XPRESSDIR%\lib\xprs.lib %XPRESSDIR%\lib\xprnls.lib`

**Linux:**

`gcc -shared -o myxprs.dso -I${XPRESSDIR}/include myxprs.c -L${XPRESSDIR}/lib -lxprs -lxprnls`

**OSX:**

`gcc -dynamiclib -install_name myxprs.dso -o myxprs.dso -I${XPRESSDIR}/include myxprs.c -L${XPRESSDIR}/lib -lxprs -lxprnls`

Or configure the provided [makefiles](../README.md) as follows:

**[Windows](../makefile.win):**

`nmake -f ..\makefile.win myxprs.dso EXTRALIB="%XPRESSDIR%\lib\xprs.lib %XPRESSDIR%\lib\xprnls.lib"`

**[Linux](../makefile.linux):**

`make -f ../makefile.linux myxprs.dso EXTRALIB="-lxprs -lxprnls"`

**[OSX](../makefile.osx):**

`make -f ../makefile.osx myxprs.dso EXTRALIB="-lxprs -lxprnls"`

## Testing

A recent (6.0 or later) version of Xpress Mosel needs to be installed.
Make sure the file `myxprs.dso` is on the DSO search path (either in the *dso* subdirectory of your Xpress Mosel installation or in a location pointed to by the enviroment variable `MOSEL_DSO`).
Run the provided test master model *[run_myxprs_tests.mos](run_myxprs_tests.mos)*:
* from the command line: `mosel run_myxprs_tests`
* with Xpress Workbench: open the file `run_myxprs_tests.mos` in the Workbench workspace and select `Run run_myxprs_tests.mos` from the workspace *Run* menu

## Legal

See source code files for copyright notices.

## License

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](../../LICENSE) for the full license text.

