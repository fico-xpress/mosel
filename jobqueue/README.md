# jobqueue

## Description

The package *jobqueue* implemented by *[jobqueue.mos](jobqueue.mos)* provides a set of Mosel subroutines for managing the remote execution of submodels via *mmjobs* and *mmhttp* functionality. Individual model files, optionally with data files, are run on one worker from a pool (queue) of remote machines that are configured via this package. *jobqueue* also implements the handling of output and errors from the submodels and the generation and retrieval of individual result output files. 

## Documentation

The package contains its documentation as *moseldoc* markup. To produce the documentation run the following command: 

`mosel make TODO=doc`

The *[PDF version of moseldoc documentation](Doc/jobqueueref.pdf)* and a *[PDF](Doc/jobqueuepres.pdf)* of the slides presenting this package is also available. 

## Building instructions

A simple package build and with a test model run can be performed via the Mosel file *make.mos* that is located at the root of this package repository:

`mosel make`

This will compile the module to the directory `build/dso`. To select a different destination directory add the option `BUILDDIR=mypath` where `mypath` is the base directory to use.

The command `mosel make TODO=clean` will remove all generated files.

## Testing

After building the module, run the following command from the current directory:

`model make.mos TODO=test`

## Legal

See source code files for copyright notices.

## License

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](../LICENSE) for the full license text.

