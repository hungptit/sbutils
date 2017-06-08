**utils** is a very fast sandbox tool that can be used to speed up large scale software development process.

# Features #

* Written using C++11/C++14 and all commands are multi-threaded using TBB.

* **mfind**: Find files in given folders using search patterns, file stems, and file extensions. 

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

## mfind ##

**mfind** is very similar to find utility in Linux, however, users can apply the some constraints such as file extension, file steps, and search patterns.

    % mfind /local/projects/3p/emacs/ -e .md -p README
    Number of files: 28
    /local/projects/3p/emacs/git_modes/README.md
    /local/projects/3p/emacs/helm-themes/README.md
    /local/projects/3p/emacs/helm_ag/README.md
    /local/projects/3p/emacs/flycheck/README.md
    /local/projects/3p/emacs/with_editor/README.md
    /local/projects/3p/emacs/markdown_mode/README.md
    /local/projects/3p/emacs/autocomplete/README.md
    /local/projects/3p/emacs/cmake-ide/README.md
    /local/projects/3p/emacs/ag/README.md
    /local/projects/3p/emacs/async/README.md
    /local/projects/3p/emacs/zenburn/README.md
    /local/projects/3p/emacs/fuzzy/README.md
    /local/projects/3p/emacs/helm/README.md
    /local/projects/3p/emacs/helm_gtags/README.md
    /local/projects/3p/emacs/popup/README.md
    /local/projects/3p/emacs/dash/README.md
    /local/projects/3p/emacs/smartparens/README.md
    /local/projects/3p/emacs/json_mode/README.md
    /local/projects/3p/emacs/projectile/README.md
    /local/projects/3p/emacs/function_args/README.md
    /local/projects/3p/emacs/cask/README.md
    /local/projects/3p/emacs/seq/README.md
    /local/projects/3p/emacs/rainbow_delimiters/README.md
    /local/projects/3p/emacs/helm_projectile/README.md
    /local/projects/3p/emacs/magit/README.md
    /local/projects/3p/emacs/web-beautify/README.md
    /local/projects/3p/emacs/json_snatcher/README.md
    /local/projects/3p/emacs/json_reformat/README.md

## mupdatedb ##

**mupdatedb** will build the file information database for given folders. This database will be used as a baseline for other commands including **mdiff**, **mlocate**, and **copydiff**. Below is a simple example

    mupdatedb /local/projects/ -d .database

## mdiff ##

**mdiff** lists all files that have been modified, added, and removed in given folders or a sandbox using the baseline. This command can handle a very large sandbox in a reasonable amount of time. Below is a sample command which will find the differences between the current state of **matlab/toolbox/** and **matlab/test/** folders and the baseline.

    % mdiff -d .database/ /local/projects/
    ---- Modified files: 3 ----
    /local/projects/3p/boost/include/boost/thread/future.hpp
    /local/projects/utils/README.md
    /local/projects/utils/FolderDiff.hpp
    ---- New files: 1 ----
    /local/projects/3p/src/boost/project-config.jam.3
    ---- Deleted files: 1 ----
    /local/projects/3p/boost/include/boost/thread/future.hpp~

    % mdiff -d .database/ /local/projects/3p/emacs/flycheck/
    ---- Modified files: 19 ----
    /local/projects/3p/emacs/flycheck/Makefile
    /local/projects/3p/emacs/flycheck/CHANGES.rst
    /local/projects/3p/emacs/flycheck/flycheck.el
    /local/projects/3p/emacs/flycheck/.travis.yml
    /local/projects/3p/emacs/flycheck/Cask
    /local/projects/3p/emacs/flycheck/doc/conf.py
    /local/projects/3p/emacs/flycheck/doc/elisp.py
    /local/projects/3p/emacs/flycheck/doc/info.py
    /local/projects/3p/emacs/flycheck/doc/languages.rst
    /local/projects/3p/emacs/flycheck/doc/community/people.rst
    /local/projects/3p/emacs/flycheck/doc/contributor/contributing.rst
    /local/projects/3p/emacs/flycheck/doc/user/error-reports.rst
    /local/projects/3p/emacs/flycheck/doc/user/flycheck-versus-flymake.rst
    /local/projects/3p/emacs/flycheck/maint/Makefile
    /local/projects/3p/emacs/flycheck/maint/requirements.txt
    /local/projects/3p/emacs/flycheck/maint/release.py
    /local/projects/3p/emacs/flycheck/test/flycheck-test.el
    /local/projects/3p/emacs/flycheck/test/init.el
    /local/projects/3p/emacs/flycheck/test/specs/test-documentation.el
    ---- New files: 5 ----
    /local/projects/3p/emacs/flycheck/maint/flycheck-format.el
    /local/projects/3p/emacs/flycheck/maint/flycheck-maint.el
    /local/projects/3p/emacs/flycheck/maint/flycheck-checkdoc.el
    /local/projects/3p/emacs/flycheck/.flake8
    /local/projects/3p/emacs/flycheck/test/specs/languages/test-elixir.el
    ---- Deleted files: 2 ----
    /local/projects/3p/emacs/flycheck/test/resources/language/lua/luacheckrc
    /local/projects/3p/emacs/flycheck/test/specs/test-code-style.el


## mlocate ##

We can use **mlocate** to locate files in given folders. The command will be significantly faster than **global** command if users can provide more specific constraints such as file extensions, search folders, and file stems. Below are sample commands that find a setup_flycheck pattern for all files in the current folder with different constraints.

    % mlocate -d .database/ AutoInterface
    Search results: 
    /local/projects/3p/src/hhvm/hphp/test/zend/bad/ext/standard/tests/class_object/AutoInterface.inc
    /local/projects/3p/src/hhvm/hphp/test/zend/good/ext/standard/tests/class_object/AutoInterface.inc

    % mlocate -d .database/ future -f /local/projects/3p/boost/
    Search results: 
    /local/projects/3p/boost/include/boost/asio/use_future.hpp
    /local/projects/3p/boost/include/boost/asio/impl/use_future.hpp
    /local/projects/3p/boost/include/boost/compute/async/future.hpp
    /local/projects/3p/boost/include/boost/fiber/future.hpp
    /local/projects/3p/boost/include/boost/fiber/future/packaged_task.hpp
    /local/projects/3p/boost/include/boost/fiber/future/future_status.hpp
    /local/projects/3p/boost/include/boost/fiber/future/future.hpp
    /local/projects/3p/boost/include/boost/fiber/future/async.hpp
    /local/projects/3p/boost/include/boost/fiber/future/promise.hpp
    /local/projects/3p/boost/include/boost/fiber/future/detail/task_base.hpp
    /local/projects/3p/boost/include/boost/fiber/future/detail/shared_state.hpp
    /local/projects/3p/boost/include/boost/fiber/future/detail/shared_state_object.hpp
    /local/projects/3p/boost/include/boost/fiber/future/detail/task_object.hpp
    /local/projects/3p/boost/include/boost/thread/future.hpp
    /local/projects/3p/boost/include/boost/thread/future.hpp~
    /local/projects/3p/boost/include/boost/thread/futures/future_error.hpp
    /local/projects/3p/boost/include/boost/thread/futures/future_error_code.hpp
    /local/projects/3p/boost/include/boost/thread/futures/wait_for_all.hpp
    /local/projects/3p/boost/include/boost/thread/futures/is_future_type.hpp
    /local/projects/3p/boost/include/boost/thread/futures/wait_for_any.hpp
    /local/projects/3p/boost/include/boost/thread/futures/future_status.hpp
    /local/projects/3p/boost/include/boost/thread/futures/launch.hpp

## mcopydiff ##

This command will copy changes that you have made in your local sandbox to the network sandbox if the source and destination file sizes are different. I do not use time stamp because it is unreliable. Below command will copy all changes that I have made in **matlab/** folder to **/sandbox/hungdang/tmp/test** folder.

    % mcopydiff -s matlab/ -d /sandbox/hungdang/tmp/test/ -v
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

## How to use PVS-Studio ##

Install PVS-Studio using this [guide](https://www.viva64.com/en/b/0457/#ID0E6PAE)

Add PVS-Studio comments to all source files using [how-to-use-pvs-studio-free](https://github.com/viva64/how-to-use-pvs-studio-free.git)
    
    how-to-use-pvs-studio-free commands/

Run analyzer

    pvs-studio-analyzer trace -- make -j5
    pvs-studio-analyzer analyze -j2 -l PVS-Studio.lic -o PVS-Studio.log
    
Convert the log into a readable format

    plog-converter -a GA:1,2 -t tasklist -o ./ PVS-Studio.log > results.log

