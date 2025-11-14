# *moseltest* Testing Program

The *moseltest* Mosel program is a general framework for testing
Mosel using source Mosel files, C programs or Java programs.
It can also be used for testing pre-compiled models, packages or
applications (in the form of zip archives) and optionally collect
coverage statistics during the execution of the tests.

## Requirements
For its execution *moseltest* requires a valid Xpress installation and,
if the tests are written in C or Java, also the corresponding development
tools:
* for Java: *java* and *javac*
* for C: *cc* and *make* (Unix); *cl* and *nmake* (Windows)

## Installation
The *moseltest* executable is included with the binaries of FICO Xpress distributions. Alternatively, you can work with the source file [moseltest.mos](moseltest.mos) from this repository. 
1. If you do not have any recent installation of FICO Xpress, download the free Xpress Community Edition from [Xpress Community Edition download](https://content.fico.com/xpress-optimization-community-license), located on FICO's website. Please note that this download is solely governed under FICO's Xpress Community License, [Xpress Shrinkwrap License Agreement](https://www.fico.com/en/shrinkwrap-license-agreement-fico-xpress-optimization-suite-on-premises).  
2. (Optional) Download the source file [moseltest.mos](moseltest.mos) from this repository.
From the Mosel program in source form `moseltest.mos`, *moseltest* can be adapted to fit 
specific user requirements. To re-generate the *moseltest* executable use this compilation command (requires a C compiler):
```
 mosel make
```
This will generate the executable in the directory `build/bin`. To select a different destination directory add the option `BUILDDIR=mypath` where `mypath` is the base directory to use.
The command `mosel make TODO=clean` will remove all generated files.

The source file can also be used directly with a command such as
```
 mosel moseltest
```

## Running *moseltest*
As input, the *moseltest* program expects either a directory containing the files
to process or a single file or an archive name. This input information is passed
via the SRCDIR model parameter (default value: 'alltests').
For instance to run the Mosel program [myexampletest.mos](myexampletest.mos):
```
 moseltest SRCDIR=myexampletest.mos
```
Running a set of tests in the default location 'alltests', keeping a copy of all output logs in 'report.txt' (by default, only failures are logged):
```
 moseltest SAVELOGS=true
```

### Running *moseltest* in coverage mode

The coverage mode is enabled by specifying a list of Mosel packages or models
in the COVLST parameter. 
The optional COVREP setting selects additional report formats. 
Any packages and models to be profiled need to be provided in compiled form 
(using the -G option), their location is specified via the LIBPATH parameter, 
additional locations for source files can be stated via the COVSRC parameter (for LIBPATH and COVSRC: employ the path separator ';' under Windows, otherwise ':'):
```
 mosel moseltest SRCDIR=covtests COVLST="mypkg mypkg2 mypkg3 mymodel myapp.zip" COVREP=3 LIBPATH="pkgs/P1;pkgs/P2"
```

## Examples
The subdirectory 'alltests' contains a set of demo examples showing different use cases of the testing system (not exhaustive, please refer to [PDF-format documentation](moseltesting.pdf) for the full set of options and supported file formats).

File or directory | Description
------------------|-----------------------------------
[simpletest.mos](alltests/simpletest.mos) | single Mosel file with moseltest tags
[testwithdata.dir](alltests/testwithdata.dir/testdataio.mos) | single Mosel file with data file packaged into a separate directory
[runmyexampletest.mcf](alltests/runmyexampletest.mcf) | test configuration file to run myexampletest.bim (not provided, needs to be compiled from [myexampletest.mos](myexampletest.mos))
[incltest.dir](alltests/incltest.dir/main.mos) | Mosel files with include files 
[javatest.dir](alltests/javatest.dir/main.java) | Java program (compiled by the test) executing a Mosel model
[math.dir](alltests/math.dir/main.mos) | Mosel file using a DSO that is built by the test (Copy of [math_test.mos](../modules/math/math_test.mos) )
[pkgtest.dir](alltests/pkgtest.dir/main.mos) | Mosel file using a package that is built by the test

## Documentation
See the [moseltest documentation](moseltestdoc.md) for the full documentation of this program.

## Legal

See source code files for copyright notices.

## License

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](http://www.apache.org/licenses/LICENSE-2.0) for the full license text.


