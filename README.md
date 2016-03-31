# Introduction #
**utils** are C++ utilities that I use to speed up my local sandbox workflow. To compile **utils** we need the following packages:

* [Boost](http://www.boost.org/): filesystem, thread, system.

* [leveldb](http://leveldb.org/)

* [snappy](http://google.github.io/snappy/)

* [cereal](http://uscilab.github.io/cereal/)

* [cppformat](https://github.com/cppformat/cppformat)

* [gtest](https://github.com/google/googletest)

* [Poco](http://pocoproject.org/)

# Installation #
* Get all required packages for **utils** including Boost. Poco, leveldb, snappy, cereal, cppformat, and gtest. If you do not want to handle this manually then you can try [3p](https://github.com/hungptit/3p). This git repository has automated shell scripts that can be used to build all required packages for **util**s.
* Compile all commands using below command:

        cd utils/testing
        cmake ./
        make -j5

# Usage #

## mupdatedb ##

**mupdatedb** will generate a file information database for given folders. This database will be the baseline for other commands including **mdiff**, **mlocate**, and **copydiff**. Below is a simple example

        mupdatedb matlab/ -d .database

## mdiff ##

**mdiff** lists all files that have been modified, added, and removed in given folders using the baseline. Below is a sample command which will find the differences between the current state of **matlab/toolbox/** and **matlab/test/** folders and their baseline.

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

We can use **mlocate** to locate files in given folders. The command will be significantly faster than the **locate** if users can provide more specific constraints. This sample command below will find all files in matlab/test folder that have tAutoFix stem.

        % mlocate matlab/test/ -s tAutoFix
        Search results: 
        matlab/test/toolbox/simulink/modelref/ss2mdlref/tAutoFix.m
        matlab/test/toolbox/slci/Compatibility/tAutoFix.m
        Elapsed time: 365.184  milliseconds

## mcopydiff ##

This command will copy changes that you have made in your local sandbox to the network sandbox if the source and destination file sizes are different. I do not use time stamp because it is unreliable. Below command will copy all changes that I have made in **matlab/** folder to **/sandbox/hungdang/tmp/test** folder.

        % mcopydiff -s matlab/ -d /sandbox/hungdang/tmp/test
        Summary:
        	Copied 13 modified files (1839641797 bytes)
        	Copied 2837 new files (7404812100 bytes)
        	Delete 0 files
        Elapsed time: 112.374  seconds

# FAQs #
