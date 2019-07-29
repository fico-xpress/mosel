# *moseltest* Testing Program

The *moseltest* Mosel program is a general framework for testing
Mosel using source Mosel files, C programs or Java programs.

## Requirements
For its execution *moseltest* requires a valid Xpress installation and,
if the tests are written in C or Java, also the corresponding development
tools:
* for Java: *java* and *javac*
* for C: *cc* and *make* (Unix); *cl* and *nmake* (Windows)

## Installation
1. If you do not have any recent installation of FICO Xpress, download the free Xpress Community Edition from [Xpress Community Edition download](http://subscribe.fico.com/xpress-optimization-community-license)  
2. Download the source file [moseltest.mos](moseltest.mos) from this repository.

## Running *moseltest*
As input, the *moseltest* program expects either a directory containing the files
to process or a single file name. This input information is passed
via the SRCDIR model parameter (default value: 'alltests').
For instance to run the Mosel program [myexampletest.mos](myexampletest.mos):
```
 mosel moseltest SRCDIR=myexampletest.mos
```
Running a set of tests in the default location 'alltests':
```
 mosel moseltest
```
When using a directory as the source for test files, any
directory structure is accepted (i.e. all subdirectories are traversed
and files are processed in alphabetical order).

## Documentation
See the file [readme.txt](readme.txt) or the [PDF documentation](moseltesting.pdf) for the full documentation of this program.

## Legal

See source code files for copyright notices.

