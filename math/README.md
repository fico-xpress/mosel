# math

## Description

The module *math* implemented by *[math.c](math.c)* (with the include file *[bigint.c](bigint.c)*) provides additional Maths functionality for the Mosel language, including the definition of a 64-bit integer type with standard access routines and operators. 

The header file *[mmath.h](mmath.h)* defines access routines for the type *int64* via Mosel's Intermodule communication interface (IMCI) in order to make this type directly available to other Mosel modules.

## Documentation

The module implements a direct wrapper for functions of the standard C library (`math.h`), please refer to the documentation of these underlying C functions for further detail. 

## Building instructions

A compiled version of this module is provided with the standard Xpress Mosel distribution. You only need to recompile this module if you have made any changes to its source.

> Compiling a DSO requires a C compiler.
> A recent version of the Mosel C libraries needs to be installed. 

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

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](../LICENSE) for the full license text.
