# jobqueue

## Description

The package *jobqueue* implemented by *[jobqueue.mos](source/jobqueue.mos)* provides a set of Mosel subroutines for managing the remote execution of submodels via *mmjobs* and *mmhttp* functionality. Individual model files, optionally with data files, are run on one worker from a pool (queue) of remote machines that are configured via this package. *jobqueue* also implements the handling of output and errors from the submodels and the generation and retrieval of individual result output files. 

## Documentation

The package contains its documentation as *moseldoc* markup. To produce the documentation please compile the package source files in the **source** subdirectory with the documentation option: 

`mosel make DOC=true`

A *[PDF](jobqueue.pdf)* of the slides presenting this package is also available. 

## Building instructions

A simple package build and with a test model run can be performed via the Mosel file *maketestjq.mos* that is located at the root of this package repository:

`mosel maketestjq`

Alternatively, you can choose to perform the following individual build steps:

1. Execute *make.mos* in the subdirectory **source**:
`mosel make`
2. Copy the resulting file *jobqueue.bim* into the **test** subdirectory or configure its location in the enironment variable `MOSEL_BIM`
3. Execute the test model *masterjq.mos* in the subdirectory **test** :
`mosel masterjq`

> A recent version of Mosel (Mosel 5) is required for executing the example.
> The remote instances may use an older version (Mosel 4), as required by the models that are to be run.


## Testing

This package is compatible with Mosel version 5.0 and more recent.

Running the provided test model *[masterjq.mos](test/masterjq.mos)*:
* compile the package source file and make sure its location is configured in the enironment variable `MOSEL_BIM`
* from the command line: `mosel masterjq`
* with Xpress Workbench: open the file `masterjq.mos` in the Workbench workspace and select `Run masterjq.mos` from the workspace menu

## Legal

See source code files for copyright notices.

## License

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](../../LICENSE) for the full license text.

