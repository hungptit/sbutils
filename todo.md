# Multi-threaded search algorithm #

* Have functions which can
    * Create a tree of folder hierarchy
    * Serialize a folder tree (_fstructure)
    * De-serialize a folder tree
    * Serialize/de-serialize a list of folder paths (_fpaths).
* Create baseline: Users will need to provide desire exploration level
    * Use DFS algorithm.
    * Only serialize file names at top level if this is not a leaf folder (at lowest level).
    * Serialize all file names belong to the leaf folder.
* How to load a baseline
    * Linear search in a list of folder names (key name is info).
        * If a given folder is in the list then load all data from the current vertex + it children.
        * If a given folder is not in the list then we find the closest ancestor and load the data from that closest ancestor. If we could not find any then throw an error.
        * Use task based parallelism approach + move semantic to speed up.
* Search algorithm -> Use task based parallelism strategy.
    * For each search folder get a sub-graph of folder users want to search.
    * For each leaf folder
        * Load the baseline.
        * Collect all files information.
        * Find the differences between baseline and current results.
    * For each non-leaf folder
        * Load the baseline
        * Collect all files at top level
        * Find the differences between baseline and current results.
    * Combine obtained results at the main threads. We need 3 lists of added, modified, and deleted files.
* Design
    * We should be able to modify the search algorithm such that it can
        * Copy change lists from source sandbox to destination sandbox.
        * Find files in database given patterns. Shall we try vertical search engines (Lucene++)?
    * Have a function which return the differences between two vectors. This should be a template function so we can test it separately.
    * Get the improved version of viewer where users can give a folder that they want to locate.
        * File information are stored into keys then we can use task based parallelism approach.
        * Users can specified following constraint: search folders, file extensions, file stems, and may be file patterns.

* Todo
    * Change commands to mlocate, mdiff, mupdatedb, mcopy with better help.
    * Support JSON output format.
    * Use cppformat instead of iostream.

* Database
    * info: Store the list of relative folder paths -> std::vector<std::string>. These are vertexes.
    * graph: Store the graph -> Describe the edges std::vector<std::tuple<int, int>>
    * #dddd: File information.
