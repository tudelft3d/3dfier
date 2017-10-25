
# 3dfier2ansys
                                            
 ___   _ ___ _         ___                     
|_  |_| |  _|_|___ ___|_  |___ ___ ___ _ _ ___ 
|_  | . |  _| | -_|  _|  _| .'|   |_ -| | |_ -|
|___|___|_| |_|___|_| |___|__,|_|_|___|_  |___|
                                      |___|    


Small utility program to convert the output of 3dfier to a mesh that can be used directly (in most cases! we can't guarantee it) as input in [ANSYS](http://www.ansys.com/) software to perform simulation.

## Compilation

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ ./3dfier2ansys 


## Usage

To convert the test-dataset of 3dfier (in `/example_data/`)