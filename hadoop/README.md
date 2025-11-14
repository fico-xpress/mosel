# hadoop

## Description
   The module defines the IO driver `hdfs` that expects a file
   name of the form:
```
     [//[user@]host[:port]/]path
```
   where:

     *user* : user name to use (by default this is the Unix user ID)
     *host*: host to connect to (by default: 'localhost')
     *port*: port number (by default: 9000)
     *path*: the path to the file. By default paths are relative to the home directory of the user
   The driver supports file deletion and directories can be removed with
   the same function as the one for ordinary files (i.e. fdelete)

   Examples:

` fopen("hadoop.hdfs://myserver/toto/bla",F_OUTPUT) `

 Open file 'bla' for writing on 'myserver' in directory ${HOME}/toto

` fopen("hadoop.hdfs://myserver//tmp/tmpfile",F_INPUT) `

 Open file '/tmp/tmpfile' for reading from 'myserver'

` fdelete("hadoop.hdfs://myserver:9000/toto/bla") `

 Delete ${HOME}/toto/bla on 'myserver'


   As a convenient way of running Hadoop commands, the procedure
   `hadoop(cmd:string)` is also defined. It is equivalent to:

` system(getenv("HADOOP_HOME")+"/bin/hadoop "+cmd) `

   Example:

` hadoop("fs -ls") `

  Display the list of files in ${HOME}

## Prerequisite
   - Linux 64bit only (intel or arm)
   - Java JDK
   - Hadoop 3 package installed and configured
       http://hadoop.apache.org/
   - environment variable *HADOOP_HOME* pointing to the
     installation directory of Hadoop
   - environment variable *LD_LIBRARY_PATH* to include:
     *${JAVA_HOME}/jre/lib/server:${HADOOP_HOME}/lib/native*
   - the module will update the classpath as necessary, however when
     running Mosel from Java, CLASSPATH must include:
     *${HADOOP_HOME}/share/hadoop/common/\*:
     ${HADOOP_HOME}/share/hadoop/common/lib/\*:
     ${HADOOP_HOME}/share/hadoop/hdfs/\*:
     ${HADOOP_HOME}/share/hadoop/hdfs/lib/\**

## Building instructions

> Compiling a DSO requires a C compiler.
> A recent version of the Mosel C libraries needs to be installed. 

Run the following command from the current directory:

`model make.mos`

This will compile the module to the directory `build/dso`. To select a different destination directory add the option `BUILDDIR=mypath` where `mypath` is the base directory to use.

The command `mosel make TODO=clean` will remove all generated files.


