# *moseldoc* Program

The *moseldoc* program serves for generating documentation from Mosel source files. The program takes as input a `bim` file produced from
 a Mosel model compiled with the __-D__ compiler option.
 Generation of the documentation for a model 'mymodel.mos' is therefore obtained by
 executing the following commands:
```
 mosel comp -D mymodel.mos
 moseldoc mymodel
```
The result of this process is an XML file (`mymodel_doc.xml`)
 and a directory containing an HTML version of the documentation
 (`mymodel_html`). The program will produce only the XML file
 (from a `bim` file) if the option __-xml__ is used and only the HTML data
 (from an XML file) if __-html__
 is selected. The option __-f__ will be required to force the 
 replacement of existing files.

The full set of available options for *moseldoc* is displayed when invoking just the *moseldoc* command without providing any input filename:
```
 moseldoc
```
It is possible to use wildcards such as `*.bim` meaning all bim files in the current directory.

## Installation
The *moseldoc* executable is included with the binaries of FICO Xpress distributions. Alternatively, you can work with the source file [moseldoc.mos](moseldoc.mos) from this repository. 
1. If you do not have any recent installation of FICO Xpress, download the free Xpress Community Edition from [Xpress Community Edition download](https://content.fico.com/xpress-optimization-community-license), located on FICO's website. Please note that this download is solely governed under FICO's Xpress Community License, [Xpress Shrinkwrap License Agreement](https://www.fico.com/en/shrinkwrap-license-agreement-fico-xpress-optimization-suite-on-premises).  
2. (Optional) Check out the contents of this repository.
From the Mosel program in source form [moseldoc.mos](moseldoc.mos), *moseldoc* can be adapted to fit 
specific user requirements. To re-generate the moseldoc executable use this compilation command (requires a C compiler):
```
 mosel make
```
This will generate the executable in the directory `build/bin`. To select a different destination directory add the option `BUILDDIR=mypath` where `mypath` is the base directory to use.
The command `mosel make TODO=clean` will remove all generated files.

When compiling *moseldoc*, the `datablock` functionality is used to include some styles file for the webpage layout.  
The provided set of files also contains the dictionary template (_.pot_) generated with the *xprnls* tool and sets of translations for German and French.

The source file can also be used directly to process a `bim` file with a command such as
```
 mosel moseldoc -- mymodel.bim
```

## Documentation
See the example [Using moseldoc](https://examples.xpress.fico.com/example.pl?id=moseldoc) for a detailed description of the functionality available through *moseldoc* and examples of using the 'doc' annotations markup.

## Legal

See source code files for copyright notices.

## License

The components in this repository are licensed under the Apache License, Version 2.0. See [LICENSE](http://www.apache.org/licenses/LICENSE-2.0) for the full license text.
