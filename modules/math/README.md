# math

## Description

The module *math* implemented by *[math.c](math.c)* provides additional Maths functionality for the Mosel language.  

## Documentation

The module implements a direct wrapper for functions of the standard C library (`math.h`), please refer to the documentation of these underlying C functions for further detail. 

## Building instructions

A compiled version of this module is provided with the standard Xpress Mosel distribution. You only need to recompile this module if you have made any changes to its source.

> Compiling a DSO requires a C compiler.
> A recent version of the Mosel C libraries needs to be installed. 

Depending on your platform, use the corresponding [makefiles](../README.md):

**[Windows](../makefile.win):**

`nmake -f ..\makefile.win math.dso`

**[Linux](../makefile.linux):**

`make -f ../makefile.linux math.dso`

**[OSX](../makefile.osx):**

`make -f ../makefile.osx math.dso` 

## Testing

This module is compatible with Mosel version 3.0 and more recent.

Make sure the file `math.dso` is on the DSO search path (either in the *dso* subdirectory of your Xpress Mosel installation or in a location pointed to by the enviroment variable `MOSEL_DSO`).
Run the provided test model *[math_test.mos](math_test.mos)*:
* from the command line: `mosel math_test`
* with Xpress Workbench: open the file `math_test.mos` in the Workbench workspace and select `Run math_test.mos` from the workspace menu

## Legal

See source code files for copyright notices.

## License

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](../../LICENSE) for the full license text.
