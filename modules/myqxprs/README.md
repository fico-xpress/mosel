# myqxprs

## Description

The example module *myqxprs* uses the matrix handling functionality provided by the Mosel module *mmnl* for implementing a basic interface
to [Xpress Optimizer](https://community.fico.com/docs/DOC-3718) for solving nonlinear (quadratic and quadratically constrained) optimization problems. Features demonstrated by this module include:

* Modeling functionality:
  * definition of subroutines to start an optimization run and retrieve solution values
  * access to solver parameters
  * support for handling multiple problems
  * definition of a solver callback (integer solution callback)
  * generation of names for matrix entities and implementation of a matrix output routine
* NI functionality:
  * implementation of a reset and an unload service
  * implementation of module dependencies services
  * initialization of the module and definition of the required interface structures
  * handling of user interrupt (Ctrl-c) to stop the solver run
  * redirection of solver output to Mosel streams

The example implementation of a solver interface provided by *[myqxprs.c](myqxprs.c)* has purposely been restricted to a selection of basic features in order to focus on the use of the *mmnl* matrix handling interface; it can easily be extended to expose other functionality of the Xpress Optimizer C library within the Mosel language. 

## Documentation

See section 'Implementing a specific nonlinear solver interface via *mmnl*' of the whitepaper [Mosel Solver Interfaces](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/mosel_solvers/dhtml/index.html) for a description of this example module and a Mosel model example showing its use. The matrix handling functionality of *mmnl* is equally documented in this whitepaper.
The matrix handling functionality of the Mosel NI is documented in the chapter 'Matrix related functions' of the [Mosel NI Reference](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/mosel_NI/dhtml/index.html). 

**Further reading:** The [Mosel NI User Guide](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/mosel_niug/dhtml/index.html) provides further examples of module implementations.


## Building instructions

> Compiling a DSO requires a C compiler.
> Besides a recent (4.8.3 or later) version of the Mosel C libraries this module also requires the Xpress Optimizer C libraries to be installed. 

Building this module requires additional dependencies, please use one of these commands depending on your operating system:

**Windows:**

`cl /LD /MD /Femyqxprs.dso /I%XPRESSDIR%\include myqxprs.c %XPRESSDIR%\lib\xprs.lib %XPRESSDIR%\lib\xprnls.lib`

**Linux:**

`gcc -shared -o myqxprs.dso -I${XPRESSDIR}/include myqxprs.c -L${XPRESSDIR}/lib -lxprs -lxprnls`

**OSX:**

`gcc -dynamiclib -install_name myqxprs.dso -o myqxprs.dso -I${XPRESSDIR}/include myqxprs.c -L${XPRESSDIR}/lib -lxprs -lxprnls`

Or configure the provided [makefiles](../README.md) as follows:

**[Windows](../makefile.win):**

`nmake -f ..\makefile.win myqxprs.dso EXTRALIB="%XPRESSDIR%\lib\xprs.lib %XPRESSDIR%\lib\xprnls.lib"`

**[Linux](../makefile.linux):**

`make -f ../makefile.linux myqxprs.dso EXTRALIB="-lxprs -lxprnls"`

**[OSX](../makefile.osx):**

`make -f ../makefile.osx myqxprs.dso EXTRALIB="-lxprs -lxprnls"`

## Testing

A recent (4.8 or later) version of Xpress Mosel needs to be installed.
Make sure the file `myqxprs.dso` is on the DSO search path (either in the *dso* subdirectory of your Xpress Mosel installation or in a location pointed to by the enviroment variable `MOSEL_DSO`).
Run the provided test master model *[run_myqxprs_tests.mos](run_myqxprs_tests.mos)*:
* from the command line: `mosel run_myqxprs_tests`
* with Xpress Workbench: open the file `run_myqxprs_tests.mos` in the Workbench workspace and select `Run run_myqxprs_tests.mos` from the workspace *Run* menu
