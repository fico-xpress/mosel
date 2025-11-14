# random

## Description

The module *random* provides implementations of various random number generators.  

## Documentation

The module documentation is included directly in the source of *[random.c](random.c)*.

## Building instructions

A compiled version of this module is provided with the standard Xpress Mosel distribution. You only need to recompile this module if you have made any changes to its source.

> Compiling a DSO requires a C compiler.
> A recent version of the Mosel C libraries needs to be installed. 


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

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](../LICENSE) for the full license text.

