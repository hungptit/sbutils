# Parallelize the search algorithm #
* Build database
    * Given a root folders
        * Explore the folders to k levels
        * Each node (folder) will store
            * Files at the root level.
            * All children of that node.
        * How to store the tree information?
            * Node's name -> files and folder at root level.
              Node -> files
                   -> folders
            * 
            * 
        * Detail implementation
            * 
* Search algorithm
    * Get a list of folders which are stored in the database
    * For each folder
        * Get the database
        * Get the list of files which has been changed using time stamp.
        * This will be executed assynchrously.
    * Search for each folder will be executed assynchronously and results will be merged at the end.
    * Split results into three lists:
        * Edited files
        * New files.
        * Deleted files.
    
    
