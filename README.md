# FICO Xpress Mosel Open Source Repository

[FICO Xpress Mosel](https://community.fico.com/docs/DOC-3787) is an analytic orchestration, algebraic modeling, and programming language. 
Mosel defines a public C interface (Mosel Native Interface) that allows developers to extend the core functionality of the Mosel language according to their needs, such as the definition of data connectors, solver interfaces, or access to system functionality on the Mosel language level.
A Mosel program (or model) is a text file that takes the extension `.mos`, it is compiled into a platform-independent `.bim` (**BI**nary **M**odel) file that is then loaded by Mosel to be run with some data instance(s). 

Mosel source files can be edited with any text editor of your choice, or you can choose to work with the development environment [FICO Xpress Workbench](https://community.fico.com/docs/DOC-3771) that forms part of the Xpress distribution.  

This repository contains various extension libraries for the Mosel language, either in the form of *Mosel packages* (libraries implemented in the Mosel language distributed as platform independent *BIM* files) or *Mosel modules* (libraries implemented in C following the conventions of the Mosel Native Interface, also called *DSO-dynamic shared objects*). 

Each subdirectory contains the source of one library, along with building instructions and some test(s).

## Contents

Component | Type | Description | Notes
----------|------|-------------|----------------------
[myxprs](modules/myxprs/README.md) | DSO | Example of using the Mosel NI matrix handling interface for connecting an LP/MIP solver (Xpress Optimizer) | Requires Xpress Optimizer libraries
[math](modules/math/README.md) | DSO | Additional Maths functions for the Mosel language | Compiled version is included in the Mosel distribution
[random](modules/random/README.md) | DSO | Alternative random number generators | Compiled version is included in the Mosel distribution

## Documentation
* Mosel documentation: 
  * [Mosel Language Reference](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/mosel_lang/dhtml/index.html)
  * [Mosel User Guide](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/UG/dhtml/index.html)
  * [Mosel NI Reference](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/mosel_NI/dhtml/index.html)
  * [Mosel NI User Guide](http://www.fico.com/fico-xpress-optimization/docs/latest/mosel/mosel_niug/dhtml/index.html)
* Mosel model and program examples: [Examples database](http://examples.xpress.fico.com/example.pl#mosel)

## Installation
If you do not have any recent installation of FICO Xpress, download the free Xpress Community Edition from [Xpress Community Edition download](http://subscribe.fico.com/xpress-optimization-community-license)  

#### Requirements
> Mosel modules are C programs, you need a C compiler to generate new DSO. 

## Contributing
 
#### Editing a component
1. Clone the Mosel Open Source repository.
2. Edit and test locally.
3. Submit a pull request.
#### Adding a new component
1. Clone the Mosel Open Source repository.
2. Create a new subdirectory locally complete with makefiles, build instructions and tests.
3. Submit a pull request.

## Legal

See source code files for copyright notices.

## License

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](LICENSE) for the full license text.


