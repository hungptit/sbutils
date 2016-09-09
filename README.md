**utils** is a very fast sandbox tool that can be used to speed up large scale software development process.

# Features #

* Written using C++11/C++14 and all commands are multi-threaded using TBB.

* **mlocate**: Locate a file in a given sandbox. This command is faster than **global** for a full sandbox search. And it can be significantly faster than **global** if users specify parent folders, file extensions, and/or file stems.

* **mdiff**: Find files that have modified in a given sandbox.

* **mcopydiff**: Copy all changes that we have made in a given sandbox to a new sandbox.

# Installation #

**sbutils** require the following packages:

* [Boost](http://www.boost.org/): filesystem, system, and program_options.

* [rocksdb](http://rocksdb.org/)

* [snappy](http://google.github.io/snappy/)

* [zlib](http://zlib.net/)

* [bzip2](http://www.bzip.org/)

* [lz4](https://github.com/Cyan4973/lz4)

* [jemalloc](https://github.com/jemalloc)

* [cereal](http://uscilab.github.io/cereal/)

* [fmt](https://github.com/fmtlib/fmt.git)

* [gtest](https://github.com/google/googletest)

* [Poco](http://pocoproject.org/)

* [TBB](https://www.threadingbuildingblocks.org/)

* [graph](https://github.com/hungptit/graph.git)

If you want to use a local version of above packages then try [3p](https://github.com/hungptit/3p). [3p](https://github.com/hungptit/3p) is a collection of Bash scripts that can be used to automatically build all required packages for **sbutils**. You compile all commands using below command. 
        cd utils/testing
        cmake ./
        make -j5

*You might have to modify the CMakeLists.txt to make it suitable for your configuration.*

# Examples #

## mupdatedb ##

**mupdatedb** will build the file information database for given folders. This database will be used as a baseline for other commands including **mdiff**, **mlocate**, and **copydiff**. Below is a simple example

    mupdatedb matlab/ -d .database

## mdiff ##

**mdiff** lists all files that have been modified, added, and removed in given folders or a sandbox using the baseline. This command can handle a very large sandbox in a reasonable amount of time. Below is a sample command which will find the differences between the current state of **matlab/toolbox/** and **matlab/test/** folders and the baseline.

        % mdiff matlab/toolbox/ matlab/test/
        ---- Modified files: 5 ----
        matlab/toolbox/local/toolbox_cache-glnxa64.xml
        matlab/toolbox/simulink/simulink/+Simulink/+ModelReference/+Conversion/CopyGraphicalInfo.m
        matlab/toolbox/simulink/simulink/+Simulink/+ModelReference/+Conversion/CheckModelForConversion.m
        matlab/toolbox/simulink/simulink/+Simulink/+ModelReference/+Conversion/ConversionData.m
        matlab/toolbox/simulink/simulink/+Simulink/+ModelReference/+Conversion/SubsystemConversion.m
        ---- New files: 3 ----
        matlab/toolbox/simulink/simulink/+Simulink/+ModelReference/+Conversion/CopyGraphicalInfo.m~
        matlab/toolbox/simulink/simulink/+Simulink/+ModelReference/+Conversion/ConversionData.m~
        matlab/toolbox/simulink/simulink/+Simulink/+ModelReference/+Conversion/CheckModelForConversion.m~
        ---- Deleted files: 78 ----
        matlab/toolbox/simulink/simulink/+Simulink/+ModelReference/+Conversion/SubsystemConversion.m~
        Elapsed time: 3.97246  seconds

## mlocate ##

We can use **mlocate** to locate files in given folders. The command will be significantly faster than **global** command if users can provide more specific constraints such as file extensions, search folders, and file stems. Below are sample commands that find a setup_flycheck pattern for all files in the current folder with different constraints.

    % mlocate setup_flycheck
    Read baseline: 404.257  milliseconds
    Filtering files param pack: 55.1193  milliseconds
    Search results:
    ./emacs/setup_flycheck.el
    Search results:
    Total time: 530.944  milliseconds

    % mlocate setup_flycheck -f ./emacs
    Read baseline: 171.286  milliseconds
    Filtering files param pack: 0.782505  milliseconds
    Search results:
    ./emacs/setup_flycheck.el
    Search results:
    Total time: 173.89  milliseconds

## mcopydiff ##

This command will copy changes that you have made in your local sandbox to the network sandbox if the source and destination file sizes are different. I do not use time stamp because it is unreliable. Below command will copy all changes that I have made in **matlab/** folder to **/sandbox/hungdang/tmp/test** folder.

    % mcopydiff -s matlab/ -d /sandbox/hungdang/tmp/test/
    Read baseline: 2192.57  milliseconds
    Search files: 15525.2  milliseconds
    Diff time: 1116.79  milliseconds
    ==== Summary for /sandbox/hungdang/tmp/test ====
    	Copied 240 modified files (2109819070 bytes)
    	Copied 2968 new files (9241188535 bytes)
    	Delete 0 files
    Copy files: 102.543  seconds

# Others #

There are utility classes in **sbutils** that can be used independently in other C++ projects including

* Timer: Calculate the elapsed time of a code segment. This class is very similar to **tic** and **toc** command in **Matlab**.

* TemporaryDirectory: Create a temporary folder and cleanup this folder when an object is destroyed. This class is very useful for writing unit tests.

# FAQs #
