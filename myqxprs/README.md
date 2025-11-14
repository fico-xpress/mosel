# myqxprs

## Description

The example module *myqxprs* uses the matrix handling functionality provided by the Mosel module *mmnl* for implementing a basic interface
to [Xpress Optimizer](https://www.fico.com/en/products/fico-xpress-solver) for solving nonlinear (quadratic and quadratically constrained) optimization problems. Features demonstrated by this module include:

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
> Besides a recent (6.0.0 or later) version of the Mosel C libraries this module also requires the Xpress Optimizer C libraries to be installed. 

Run the following command from the current directory:

`model make.mos`

This will compile the module to the directory `build/dso`. To select a different destination directory add the option `BUILDDIR=mypath` where `mypath` is the base directory to use.

The command `mosel make TODO=clean` will remove all generated files.

## Testing

After building the module, run the following command from the current directory:

`model make.mos TODO=test`

## Legal

See source code files for copyright notices.

## License

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](../../LICENSE) for the full license text.

