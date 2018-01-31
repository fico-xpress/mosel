# random

## Description

The module *random* provides implementations of various random number generators.  

## Documentation

The module documentation is included directly in the source of *[random.c](random.c)*.

## Building instructions

A compiled version of this module is provided with the standard Xpress Mosel distribution. You only need to recompile this module if you have made any changes to its source.

> Compiling a DSO requires a C compiler.
> A recent version of the Mosel C libraries needs to be installed. 


Depending on your platform, use the corresponding [makefiles](../README.md):

**[Windows](../makefile.win):**

`nmake -f ..\makefile.win random.dso`

**[Linux](../makefile.linux):**

`make -f ../makefile.linux random.dso`

**[OSX](../makefile.osx):**

`make -f ../makefile.osx random.dso`  

## Testing

This module is compatible with Mosel version 3.0 and more recent.

Make sure the file `random.dso` is on the DSO search path (either in the *dso* subdirectory of your Xpress Mosel installation or in a location pointed to by the enviroment variable `MOSEL_DSO`).
Run the provided test model *[random_test.mos](random_test.mos)*:
* from the command line: `mosel random_test`
* with Xpress Workbench: open the file `random_test.mos` in the Workbench workspace and select `Run random_test.mos` from the workspace menu
